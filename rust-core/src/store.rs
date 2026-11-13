//! Where a log is published and read back.
//!
//! The account crates take their provider by value, and this app needs the
//! same store in three places at once (the resolver, each publish, and the
//! read-back that confirms it), so every provider here is a cheap handle over
//! shared state rather than the state itself.
//!
//! By default logs live in the account registry on a Logos Execution Zone
//! (logos-messaging/account-registry-lez), one per address. Reads go straight
//! to the sequencer and are verified here. A write is an `Append` transaction,
//! which a funded LEZ account has to pay for, so this library prepares it and
//! hands it to the [`Submit`] the app registers, which sends it through the
//! user's LEZ wallet; the store then waits for the log to land.
//!
//! chat-store's `/v1/account` (logos-messaging/chat-store#8) remains as an
//! alternative. A log crosses it as the artifact the account-log crate
//! transmits, `signature || payload`, in both directions.

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use account::{AccountProvider, AccountPublisher};
use account_log::{AccountAddr, SignedAccountLog, MAX_PAYLOAD_BYTES};
use account_registry_client::relay::{submit_http, RelayAuth, RelayReply, RelayRequest};
use account_registry_client::{
    AccountId, LezAccountRegError, PrivateKey, Registry, PUBLISH_TIMEOUT,
};
use serde::Deserialize;
use serde_json::json;
use thiserror::Error;

/// The public LEZ testnet sequencer, used unless the caller names another.
pub const DEFAULT_SEQUENCER_URL: &str = "https://testnet.lez.logos.co";

/// The registry program on the default sequencer. Unset until it is deployed,
/// so a caller must name one.
pub const DEFAULT_REGISTRY_PROGRAM: &str = "";

const FETCH_TIMEOUT: Duration = Duration::from_secs(15);

/// How long a sponsored publish waits for the relay's write to land. Testnet
/// makes about one block a minute.
const RELAY_WAIT: Duration = Duration::from_secs(180);

/// The largest artifact chat-store can answer with: the format's payload cap
/// plus the signature in front of it.
const MAX_ARTIFACT_BYTES: u64 = MAX_PAYLOAD_BYTES as u64 + 64;

#[derive(Debug, Error)]
pub enum StoreError {
    #[error("store request failed: {0}")]
    Transport(String),
    #[error("the store refused the log: it is not an extension of what it holds")]
    Diverged,
    #[error("this store has no account-log route: nothing answers /v1/account there")]
    NoRoute,
    #[error("store returned {code}{}", reason.as_deref().map(|r| format!(": {r}")).unwrap_or_default())]
    Status { code: u16, reason: Option<String> },
    #[error("store returned something that is not a signed log: {0}")]
    Malformed(String),
    #[error("that would take the log past the registry's limit of {} KiB", .0 / 1024)]
    TooLarge(usize),
    #[error("no account registry is configured: {0}")]
    NotConfigured(String),
    #[error("the LEZ wallet did not send the update: {0}")]
    Wallet(String),
    #[error("the relay did not publish the update: {0}")]
    Relay(String),
}

impl From<LezAccountRegError> for StoreError {
    fn from(error: LezAccountRegError) -> Self {
        match error {
            LezAccountRegError::NotAnExtension | LezAccountRegError::Conflict => Self::Diverged,
            LezAccountRegError::TooLarge(limit) => Self::TooLarge(limit),
            LezAccountRegError::Unverified(e) | LezAccountRegError::Log(e) => {
                Self::Malformed(e.to_string())
            }
            LezAccountRegError::Timeout => {
                Self::Transport("the update did not appear on chain in time".into())
            }
            other => Self::Transport(other.to_string()),
        }
    }
}

/// Sends one prepared `Append` and answers once a sequencer has taken it, or
/// why it was not sent. The request is JSON:
///
/// ```text
/// {"program": hex, "account": hex, "instruction": hex, "gasLimit": n}
/// ```
///
/// `account` is the one account the transaction names, not signing, in the
/// shard of `program`; `instruction` is its data. Whoever implements this
/// picks the fee payer.
pub type Submit = dyn Fn(&str) -> Result<(), String> + Send + Sync;

#[derive(Clone, Default)]
struct Submitter(Arc<Mutex<Option<Arc<Submit>>>>);

/// Sends one submission to an account relay and returns its reply, both as
/// JSON bytes. Whoever implements this carries them, e.g. over libp2p.
pub type RelaySend = dyn Fn(&[u8]) -> Result<Vec<u8>, String> + Send + Sync;

/// How submissions reach the relay.
#[derive(Clone)]
enum RelayRoute {
    Http(String),
    Sender(Arc<RelaySend>),
}

/// An account relay, which publishes a log and pays its fee.
#[derive(Clone)]
struct RelayTarget {
    route: RelayRoute,
    token: Option<String>,
}

/// The account registry on a LEZ sequencer.
#[derive(Clone)]
pub struct LezStore {
    sequencer: Arc<str>,
    registry: Result<Registry, Arc<str>>,
    submitter: Submitter,
    payer: Arc<Mutex<Option<PrivateKey>>>,
    relay: Arc<Mutex<Option<RelayTarget>>>,
    sponsored: Arc<AtomicBool>,
}

impl LezStore {
    /// The registry deployed at `program` (base58) on `sequencer`. An empty or
    /// unreadable program fails every call with [`StoreError::NotConfigured`].
    pub fn new(sequencer: &str, program: &str) -> Self {
        let registry = match program.trim() {
            "" => Err("set the registry program".into()),
            program => program
                .parse::<AccountId>()
                .map_err(|_| Arc::from(format!("{program} is not a LEZ account id")))
                .and_then(|program| {
                    Registry::connect(sequencer, program).map_err(|e| e.to_string().into())
                }),
        };
        Self {
            sequencer: sequencer.trim_end_matches('/').into(),
            registry,
            submitter: Submitter::default(),
            payer: Arc::default(),
            relay: Arc::default(),
            sponsored: Arc::new(AtomicBool::new(true)),
        }
    }

    pub fn describe(&self) -> String {
        match &self.registry {
            Ok(registry) => format!("registry {} on {}", registry.program(), self.sequencer),
            Err(_) => format!("no registry on {}", self.sequencer),
        }
    }

    /// Route every publish through `submit`. Until one is set, a publish fails.
    pub fn set_submitter(&self, submit: Arc<Submit>) {
        *self.submitter.0.lock().expect("submitter lock") = Some(submit);
    }

    /// Pay for every publish with `key` (hex), a funded LEE account's key,
    /// instead of going through the submitter. For running without a wallet.
    pub fn set_payer_key(&self, key: &str) -> Result<(), StoreError> {
        let key = key
            .trim()
            .parse::<PrivateKey>()
            .map_err(|_| StoreError::NotConfigured("the payer key is not a LEE key".into()))?;
        *self.payer.lock().expect("payer lock") = Some(key);
        Ok(())
    }

    /// Have the relay at `url` publish and pay, while sponsoring is on.
    /// `token`, if any, is sent as `token` credentials.
    pub fn set_relay(&self, url: &str, token: Option<&str>) {
        let url = url.trim();
        let route = (!url.is_empty()).then(|| RelayRoute::Http(url.to_owned()));
        self.route_relay(route, token);
    }

    /// Like [`Self::set_relay`], with `send` carrying each submission.
    pub fn set_relay_sender(&self, send: Arc<RelaySend>, token: Option<&str>) {
        self.route_relay(Some(RelayRoute::Sender(send)), token);
    }

    fn route_relay(&self, route: Option<RelayRoute>, token: Option<&str>) {
        let token = token.map(str::to_owned).filter(|token| !token.is_empty());
        *self.relay.lock().expect("relay lock") = route.map(|route| RelayTarget { route, token });
    }

    /// Whether a relay pays for publishes. Off, or with no relay set, the
    /// account's own funds do.
    pub fn set_sponsored(&self, on: bool) {
        self.sponsored.store(on, Ordering::SeqCst);
    }

    /// The relay that pays for the next publish, if any.
    fn sponsor(&self) -> Option<RelayTarget> {
        if !self.sponsored.load(Ordering::SeqCst) {
            return None;
        }
        self.relay.lock().expect("relay lock").clone()
    }

    fn registry(&self) -> Result<&Registry, StoreError> {
        self.registry
            .as_ref()
            .map_err(|reason| StoreError::NotConfigured(reason.to_string()))
    }
}

impl AccountProvider for LezStore {
    type Error = StoreError;

    /// The log as the chain holds it, not yet checked against `addr`: the
    /// resolver verifies it, as it does what the other stores serve.
    fn fetch(&self, addr: &AccountAddr) -> Result<Option<SignedAccountLog>, Self::Error> {
        let registry = self.registry()?;
        let account = registry.account(registry.account_id(addr))?;
        let data: &[u8] = account.data.shard(registry.program()).as_ref();
        if data.is_empty() {
            return Ok(None);
        }
        SignedAccountLog::from_bytes(data)
            .map(Some)
            .map_err(|e| StoreError::Malformed(e.to_string()))
    }
}

impl AccountPublisher for LezStore {
    type Error = StoreError;

    /// Prepares the append against the log on chain, has the submitter send
    /// it, and waits until the chain holds `log`.
    fn publish(&mut self, addr: &AccountAddr, log: &SignedAccountLog) -> Result<(), Self::Error> {
        let registry = self.registry()?;
        if let Some(relay) = self.sponsor() {
            return publish_through(registry, &relay, addr, log);
        }
        if let Some(payer) = self.payer.lock().expect("payer lock").clone() {
            return Ok(registry.clone().with_payer(payer).publish(addr, log)?);
        }
        let submit = self
            .submitter
            .0
            .lock()
            .expect("submitter lock")
            .clone()
            .ok_or_else(|| StoreError::Wallet("no LEZ wallet is connected".into()))?;
        let prepared = registry.prepare_append(addr, log)?;
        let request = json!({
            "program": hex::encode(prepared.program.value()),
            "account": hex::encode(prepared.account.value()),
            "instruction": hex::encode(prepared.instruction_data()),
            "gasLimit": prepared.gas_limit,
        });
        submit(&request.to_string()).map_err(StoreError::Wallet)?;
        Ok(registry.await_append(&prepared, PUBLISH_TIMEOUT)?)
    }
}

/// The body chat-store answers every failure with.
#[derive(Deserialize)]
struct ErrorBody {
    error: String,
}

/// What the store said went wrong, if it said it the way chat-store does.
fn reason(response: &mut ureq::http::Response<ureq::Body>) -> Option<String> {
    let text = response
        .body_mut()
        .with_config()
        .limit(4096)
        .read_to_string()
        .ok()?;
    serde_json::from_str::<ErrorBody>(&text)
        .ok()
        .map(|body| body.error)
}

fn transport(e: ureq::Error) -> StoreError {
    StoreError::Transport(match e {
        ureq::Error::Timeout(_) => format!("no answer within {} s", FETCH_TIMEOUT.as_secs()),
        ureq::Error::Io(e) if e.kind() == std::io::ErrorKind::ConnectionRefused => {
            "connection refused".into()
        }
        other => other.to_string(),
    })
}

/// chat-store over HTTP.
#[derive(Clone)]
pub struct ChatStore {
    base_url: Arc<str>,
    agent: ureq::Agent,
}

impl ChatStore {
    pub fn new(base_url: &str) -> Self {
        let config = ureq::Agent::config_builder()
            .timeout_global(Some(FETCH_TIMEOUT))
            .build();
        Self {
            base_url: base_url.trim_end_matches('/').into(),
            agent: config.into(),
        }
    }

    pub fn base_url(&self) -> &str {
        &self.base_url
    }

    fn url(&self, addr: &AccountAddr) -> String {
        format!("{}/v1/account/{addr}", self.base_url)
    }
}

impl AccountProvider for ChatStore {
    type Error = StoreError;

    fn fetch(&self, addr: &AccountAddr) -> Result<Option<SignedAccountLog>, Self::Error> {
        let mut response = self
            .agent
            .get(&self.url(addr))
            .config()
            .http_status_as_error(false)
            .build()
            .call()
            .map_err(transport)?;
        match response.status().as_u16() {
            200..=299 => {}
            // An account that has never published is not a failure to report,
            // and the route says so in its own words. A 404 without them is a
            // server with no such route, which must not read as an account
            // that has published nothing.
            404 => {
                return match reason(&mut response) {
                    Some(_) => Ok(None),
                    None => Err(StoreError::NoRoute),
                }
            }
            code => {
                return Err(StoreError::Status {
                    code,
                    reason: reason(&mut response),
                })
            }
        }
        let artifact = response
            .body_mut()
            .with_config()
            .limit(MAX_ARTIFACT_BYTES)
            .read_to_vec()
            .map_err(|e| match e {
                ureq::Error::BodyExceedsLimit(_) => StoreError::Malformed(e.to_string()),
                // The body stopped coming: the connection's failure, not the log's.
                other => transport(other),
            })?;
        SignedAccountLog::from_bytes(&artifact)
            .map(Some)
            .map_err(|e| StoreError::Malformed(e.to_string()))
    }
}

impl AccountPublisher for ChatStore {
    type Error = StoreError;

    fn publish(&mut self, addr: &AccountAddr, log: &SignedAccountLog) -> Result<(), Self::Error> {
        let mut response = self
            .agent
            .post(&self.url(addr))
            .config()
            // A POST redirected with 301, 302 or 303 is re-sent as a GET, whose
            // answer would pass for a publish that never happened.
            .max_redirects(0)
            .http_status_as_error(false)
            .build()
            .header("Content-Type", "application/octet-stream")
            .send(log.to_bytes())
            .map_err(transport)?;
        match response.status().as_u16() {
            200..=299 => Ok(()),
            // The store holds a log this one does not extend: a longer copy of
            // it, or another history under the same address.
            409 => Err(StoreError::Diverged),
            404 => Err(StoreError::NoRoute),
            code => Err(StoreError::Status {
                code,
                reason: reason(&mut response),
            }),
        }
    }
}

/// A store in memory, shared by every clone. What the tests publish to, and
/// what the app falls back on only when it is told to run without a store.
#[derive(Clone, Default)]
pub struct MemoryStore(Arc<Mutex<Vec<(AccountAddr, SignedAccountLog)>>>);

impl MemoryStore {
    pub fn new() -> Self {
        Self::default()
    }

    fn held(&self, addr: &AccountAddr) -> Option<SignedAccountLog> {
        let held = self.0.lock().expect("store lock");
        held.iter()
            .find(|(held, _)| held == addr)
            .map(|(_, log)| log.clone())
    }
}

impl AccountProvider for MemoryStore {
    type Error = StoreError;

    fn fetch(&self, addr: &AccountAddr) -> Result<Option<SignedAccountLog>, Self::Error> {
        Ok(self.held(addr))
    }
}

impl AccountPublisher for MemoryStore {
    type Error = StoreError;

    /// Enforces what the real route will: a log is accepted only where it
    /// extends the one held, so a test cannot pass against a store more
    /// permissive than the service.
    fn publish(&mut self, addr: &AccountAddr, log: &SignedAccountLog) -> Result<(), Self::Error> {
        let mut held = self.0.lock().expect("store lock");
        match held.iter_mut().find(|(held, _)| held == addr) {
            Some((_, existing)) => {
                let candidate = log.payload.as_bytes();
                if !candidate.starts_with(existing.payload.as_bytes()) {
                    return Err(StoreError::Diverged);
                }
                *existing = log.clone();
            }
            None => held.push((addr.clone(), log.clone())),
        }
        Ok(())
    }
}

/// Hands `log` to `relay` and waits until the chain holds it.
fn publish_through(
    registry: &Registry,
    relay: &RelayTarget,
    addr: &AccountAddr,
    log: &SignedAccountLog,
) -> Result<(), StoreError> {
    let prepared = registry.prepare_append(addr, log)?;
    let auth = relay
        .token
        .clone()
        .map_or_else(RelayAuth::default, |token| RelayAuth {
            kind: "token".into(),
            payload: token,
        });
    let request = RelayRequest::new(addr, log, auth);
    let unreachable =
        |error: String| StoreError::Relay(format!("it could not be reached: {error}"));
    let reply = match &relay.route {
        RelayRoute::Http(url) => {
            submit_http(url, &request).map_err(|error| unreachable(error.to_string()))?
        }
        RelayRoute::Sender(send) => {
            let bytes = send(&request.to_bytes()).map_err(unreachable)?;
            serde_json::from_slice::<RelayReply>(&bytes).map_err(|error| {
                StoreError::Relay(format!("it answered something else: {error}"))
            })?
        }
    };
    if !reply.accepted {
        return Err(StoreError::Relay(
            reply.reason.unwrap_or_else(|| "it refused".into()),
        ));
    }
    match registry.await_append(&prepared, RELAY_WAIT) {
        Err(
            LezAccountRegError::Timeout
            | LezAccountRegError::Transport(_)
            | LezAccountRegError::Rpc { .. }
            | LezAccountRegError::Response(_),
        ) => Err(StoreError::Relay(
            "it accepted the update, which is not on chain yet but may still land; wait before \
             paying with your own funds"
                .into(),
        )),
        landed => Ok(landed?),
    }
}

/// The store this app was pointed at. One type rather than a type parameter,
/// because the choice is made once from the environment and every caller below
/// would otherwise carry it.
#[derive(Clone)]
pub enum Store {
    /// The account registry on a LEZ sequencer.
    Lez(LezStore),
    /// chat-store, over HTTP.
    Chat(ChatStore),
    /// This process's own memory. What the tests publish to, and what a harness
    /// with no network gets.
    Memory(MemoryStore),
}

impl Store {
    /// `memory` as the sequencer selects the in-process store; anything else
    /// is a sequencer URL, with the registry deployed at `program` on it.
    pub fn open(sequencer: &str, program: &str) -> Self {
        match sequencer {
            "memory" => Self::memory(),
            sequencer => Self::Lez(LezStore::new(sequencer, program)),
        }
    }

    /// chat-store at `url`, or the in-process store for `memory`.
    pub fn chat(url: &str) -> Self {
        match url {
            "memory" => Self::memory(),
            url => Self::Chat(ChatStore::new(url)),
        }
    }

    pub fn memory() -> Self {
        Self::Memory(MemoryStore::new())
    }

    pub fn describe(&self) -> String {
        match self {
            Self::Lez(store) => store.describe(),
            Self::Chat(store) => store.base_url().to_owned(),
            Self::Memory(_) => "memory".to_owned(),
        }
    }

    /// See [`LezStore::set_payer_key`]. The other stores take no payer.
    pub fn set_payer_key(&self, key: &str) -> Result<(), StoreError> {
        match self {
            Self::Lez(store) => store.set_payer_key(key),
            _ => Ok(()),
        }
    }

    /// See [`LezStore::set_submitter`]. The other stores need none.
    pub fn set_submitter(&self, submit: Arc<Submit>) {
        if let Self::Lez(store) = self {
            store.set_submitter(submit);
        }
    }

    /// See [`LezStore::set_relay`]. The other stores need no relay.
    pub fn set_relay(&self, url: &str, token: Option<&str>) {
        if let Self::Lez(store) = self {
            store.set_relay(url, token);
        }
    }

    /// See [`LezStore::set_relay_sender`].
    pub fn set_relay_sender(&self, send: Arc<RelaySend>, token: Option<&str>) {
        if let Self::Lez(store) = self {
            store.set_relay_sender(send, token);
        }
    }

    /// See [`LezStore::set_sponsored`].
    pub fn set_sponsored(&self, on: bool) {
        if let Self::Lez(store) = self {
            store.set_sponsored(on);
        }
    }
}

impl AccountProvider for Store {
    type Error = StoreError;

    fn fetch(&self, addr: &AccountAddr) -> Result<Option<SignedAccountLog>, Self::Error> {
        match self {
            Self::Lez(store) => store.fetch(addr),
            Self::Chat(store) => store.fetch(addr),
            Self::Memory(store) => store.fetch(addr),
        }
    }
}

impl AccountPublisher for Store {
    type Error = StoreError;

    fn publish(&mut self, addr: &AccountAddr, log: &SignedAccountLog) -> Result<(), Self::Error> {
        match self {
            Self::Lez(store) => store.publish(addr, log),
            Self::Chat(store) => store.publish(addr, log),
            Self::Memory(store) => store.publish(addr, log),
        }
    }
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;
    use account_log::{AccountLogDraft, Context, Ed25519SigningKey, EntryData, ACCOUNT_LOG_DOMAIN};
    use std::collections::HashMap;
    use std::io::{BufRead, BufReader, Read, Write};
    use std::sync::atomic::{AtomicBool, Ordering};

    fn signed(key: &Ed25519SigningKey, values: &[&str]) -> (AccountAddr, SignedAccountLog) {
        let context = Context::new("chat.signer").unwrap();
        let mut draft = AccountLogDraft::new();
        for value in values {
            draft
                .add(context, EntryData::Text((*value).into()))
                .unwrap();
        }
        let payload = draft.log().encode().unwrap();
        let signature = key.sign(payload.as_bytes());
        (
            AccountAddr::from(&key.verifying_key()),
            SignedAccountLog { payload, signature },
        )
    }

    #[test]
    fn an_account_that_never_published_fetches_to_nothing() {
        let store = MemoryStore::new();
        let addr = AccountAddr::from(&Ed25519SigningKey::generate().verifying_key());
        assert_eq!(store.fetch(&addr).unwrap(), None);
    }

    #[test]
    fn an_extension_is_stored_and_read_back() {
        let key = Ed25519SigningKey::generate();
        let mut store = MemoryStore::new();
        let (addr, first) = signed(&key, &["saro"]);
        let (_, second) = signed(&key, &["saro", "raya"]);

        store.publish(&addr, &first).unwrap();
        store.publish(&addr, &second).unwrap();

        assert_eq!(store.fetch(&addr).unwrap(), Some(second));
    }

    /// The fake refuses what the registry will refuse, so a test that passes
    /// here is not passing because the store was lenient.
    #[test]
    fn a_log_that_does_not_extend_is_refused() {
        let key = Ed25519SigningKey::generate();
        let mut store = MemoryStore::new();
        let (addr, held) = signed(&key, &["saro", "raya"]);
        let (_, forked) = signed(&key, &["pax", "raya", "saro"]);

        store.publish(&addr, &held).unwrap();

        assert!(matches!(
            store.publish(&addr, &forked),
            Err(StoreError::Diverged)
        ));
        assert_eq!(store.fetch(&addr).unwrap(), Some(held));
    }

    /// Every clone is the same store, which is what lets one live in the
    /// resolver and another in each publish.
    #[test]
    fn clones_share_one_store() {
        let key = Ed25519SigningKey::generate();
        let store = MemoryStore::new();
        let (addr, log) = signed(&key, &["saro"]);

        store.clone().publish(&addr, &log).unwrap();

        assert_eq!(store.fetch(&addr).unwrap(), Some(log));
    }

    /// The program account the fake registry is deployed at.
    pub(crate) const PROGRAM: &str = "9y1cAZgyEEZLPAH8q8Z37e6Jvaz2rUTiS8qGm6DpL5BU";

    /// A sequencer serving the registry's shards over `getAccount`, and a
    /// submitter applying each `Append` to them as the program does. `down`
    /// makes every request fail.
    #[derive(Clone, Default)]
    pub(crate) struct FakeRegistry {
        shards: Arc<Mutex<HashMap<String, Vec<u8>>>>,
        pub(crate) down: Arc<AtomicBool>,
        pub(crate) submitted: Arc<Mutex<Vec<serde_json::Value>>>,
    }

    impl FakeRegistry {
        /// Serves this registry, and returns a store connected to it.
        pub(crate) fn store(&self) -> Store {
            let listener = std::net::TcpListener::bind("127.0.0.1:0").unwrap();
            let url = format!("http://{}", listener.local_addr().unwrap());
            let registry = self.clone();
            std::thread::spawn(move || {
                for stream in listener.incoming() {
                    registry.answer(stream.unwrap());
                }
            });
            let store = Store::open(&url, PROGRAM);
            let registry = self.clone();
            store.set_submitter(Arc::new(move |request: &str| registry.submit(request)));
            store
        }

        fn answer(&self, stream: std::net::TcpStream) {
            let mut reader = BufReader::new(stream);
            let mut length = 0;
            loop {
                let mut line = String::new();
                reader.read_line(&mut line).unwrap();
                if let Some(value) = line.to_ascii_lowercase().strip_prefix("content-length:") {
                    length = value.trim().parse().unwrap();
                }
                if line == "\r\n" {
                    break;
                }
            }
            let mut body = vec![0; length];
            reader.read_exact(&mut body).unwrap();
            let response = if self.down.load(Ordering::SeqCst) {
                "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
                    .to_owned()
            } else {
                let request: serde_json::Value = serde_json::from_slice(&body).unwrap();
                assert_eq!(request["method"], "getAccount");
                let account: AccountId = request["params"][0].as_str().unwrap().parse().unwrap();
                let data = self
                    .shards
                    .lock()
                    .unwrap()
                    .get(&hex::encode(account.value()))
                    .cloned();
                let shards = match data {
                    Some(data) => json!({ PROGRAM: data }),
                    None => json!({}),
                };
                let reply = json!({
                    "jsonrpc": "2.0",
                    "id": request["id"],
                    "result": { "nonce": 0, "data": { "shards": shards } },
                })
                .to_string();
                format!(
                    "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{reply}",
                    reply.len()
                )
            };
            reader.into_inner().write_all(response.as_bytes()).unwrap();
        }

        /// Puts `log` on chain as `addr`'s, whoever signed it.
        pub(crate) fn hold(&self, addr: &AccountAddr, log: &SignedAccountLog) {
            let registry = Registry::connect("http://localhost", PROGRAM.parse().unwrap()).unwrap();
            let account = hex::encode(registry.account_id(addr).value());
            self.shards.lock().unwrap().insert(account, log.to_bytes());
        }

        /// Serves an account relay that publishes every submission on this
        /// registry, or refuses each with `refusal`, and returns its URL.
        pub(crate) fn relay(&self, refusal: Option<&'static str>) -> String {
            let listener = std::net::TcpListener::bind("127.0.0.1:0").unwrap();
            let url = format!("http://{}", listener.local_addr().unwrap());
            let registry = self.clone();
            std::thread::spawn(move || {
                for stream in listener.incoming() {
                    let mut reader = BufReader::new(stream.unwrap());
                    let mut length = 0;
                    loop {
                        let mut line = String::new();
                        reader.read_line(&mut line).unwrap();
                        let lower = line.to_ascii_lowercase();
                        if let Some(value) = lower.strip_prefix("content-length:") {
                            length = value.trim().parse().unwrap();
                        }
                        if line == "\r\n" {
                            break;
                        }
                    }
                    let mut body = vec![0; length];
                    reader.read_exact(&mut body).unwrap();
                    let request = RelayRequest::from_bytes(&body).unwrap();
                    let reply = match refusal {
                        Some(reason) => json!({ "accepted": false, "reason": reason }),
                        None => {
                            let (addr, log) = request.decode().unwrap();
                            registry.hold(&addr, &log);
                            json!({ "accepted": true })
                        }
                    }
                    .to_string();
                    let status = if refusal.is_some() {
                        "422 Unprocessable Entity"
                    } else {
                        "202 Accepted"
                    };
                    let response = format!(
                        "HTTP/1.1 {status}\r\nContent-Type: application/json\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{reply}",
                        reply.len()
                    );
                    reader.into_inner().write_all(response.as_bytes()).unwrap();
                }
            });
            url
        }

        /// The program's `apply`: signature, then stored payload, then entries.
        fn submit(&self, request: &str) -> Result<(), String> {
            let request: serde_json::Value = serde_json::from_str(request).unwrap();
            self.submitted.lock().unwrap().push(request.clone());
            let instruction = hex::decode(request["instruction"].as_str().unwrap()).unwrap();
            assert_eq!(instruction[0], 0, "Append is the first instruction");
            let signature = &instruction[33..97];
            let entries = &instruction[101..];
            let account = request["account"].as_str().unwrap().to_owned();
            let mut shards = self.shards.lock().unwrap();
            let payload = match shards.get(&account) {
                Some(stored) => stored[64..].to_vec(),
                None => ACCOUNT_LOG_DOMAIN.to_vec(),
            };
            shards.insert(account, [signature, &payload, entries].concat());
            Ok(())
        }
    }

    #[test]
    fn a_sponsored_publish_goes_through_the_relay_not_the_wallet() {
        let fake = FakeRegistry::default();
        let mut store = fake.store();
        store.set_relay(&fake.relay(None), None);
        let key = Ed25519SigningKey::generate();
        let (addr, log) = signed(&key, &["saro"]);

        store.publish(&addr, &log).unwrap();
        assert_eq!(store.fetch(&addr).unwrap(), Some(log));
        assert!(fake.submitted.lock().unwrap().is_empty());
    }

    #[test]
    fn a_relay_reached_through_a_sender_publishes_too() {
        let fake = FakeRegistry::default();
        let mut store = fake.store();
        let registry = fake.clone();
        store.set_relay_sender(
            Arc::new(move |bytes: &[u8]| {
                let (addr, log) = RelayRequest::from_bytes(bytes).unwrap().decode().unwrap();
                registry.hold(&addr, &log);
                Ok(br#"{"accepted":true}"#.to_vec())
            }),
            None,
        );
        let key = Ed25519SigningKey::generate();
        let (addr, log) = signed(&key, &["saro"]);

        store.publish(&addr, &log).unwrap();
        assert_eq!(store.fetch(&addr).unwrap(), Some(log));
        assert!(fake.submitted.lock().unwrap().is_empty());
    }

    #[test]
    fn a_relay_refusal_says_why() {
        let fake = FakeRegistry::default();
        let mut store = fake.store();
        store.set_relay(&fake.relay(Some("the relay is out of funds")), None);
        let key = Ed25519SigningKey::generate();
        let (addr, log) = signed(&key, &["saro"]);

        let refused = store.publish(&addr, &log).unwrap_err();
        assert!(matches!(&refused, StoreError::Relay(reason) if reason.contains("out of funds")));
        assert!(fake.submitted.lock().unwrap().is_empty());
    }

    #[test]
    fn with_sponsoring_off_the_wallet_pays() {
        let fake = FakeRegistry::default();
        let mut store = fake.store();
        store.set_relay(&fake.relay(Some("must not be asked")), None);
        store.set_sponsored(false);
        let key = Ed25519SigningKey::generate();
        let (addr, log) = signed(&key, &["saro"]);

        store.publish(&addr, &log).unwrap();
        assert_eq!(fake.submitted.lock().unwrap().len(), 1);
    }

    #[test]
    fn a_published_log_is_read_back_from_the_registry() {
        let fake = FakeRegistry::default();
        let mut store = fake.store();
        let key = Ed25519SigningKey::generate();
        let (addr, first) = signed(&key, &["saro"]);
        let (_, second) = signed(&key, &["saro", "raya"]);

        assert_eq!(store.fetch(&addr).unwrap(), None);
        store.publish(&addr, &first).unwrap();
        store.publish(&addr, &second).unwrap();

        assert_eq!(store.fetch(&addr).unwrap(), Some(second));
        let submitted = fake.submitted.lock().unwrap();
        assert_eq!(submitted.len(), 2);
        assert!(submitted[0]["gasLimit"].as_u64().unwrap() >= 4_000_000);
    }

    #[test]
    fn a_log_that_does_not_extend_is_refused_before_it_is_sent() {
        let fake = FakeRegistry::default();
        let mut store = fake.store();
        let key = Ed25519SigningKey::generate();
        let (addr, held) = signed(&key, &["saro", "raya"]);
        let (_, forked) = signed(&key, &["pax"]);

        store.publish(&addr, &held).unwrap();
        assert!(matches!(
            store.publish(&addr, &forked),
            Err(StoreError::Diverged)
        ));
        assert_eq!(fake.submitted.lock().unwrap().len(), 1);
    }

    #[test]
    fn a_publish_with_no_wallet_connected_is_refused() {
        let url = format!("http://{}", "127.0.0.1:9");
        let mut store = Store::open(&url, PROGRAM);
        let (addr, log) = signed(&Ed25519SigningKey::generate(), &["saro"]);
        assert!(matches!(
            store.publish(&addr, &log),
            Err(StoreError::Wallet(_))
        ));
    }

    #[test]
    fn a_registry_without_a_program_says_so() {
        let store = Store::open(DEFAULT_SEQUENCER_URL, "");
        let addr = AccountAddr::from(&Ed25519SigningKey::generate().verifying_key());
        assert!(matches!(
            store.fetch(&addr),
            Err(StoreError::NotConfigured(_))
        ));
        assert_eq!(
            store.describe(),
            "no registry on https://testnet.lez.logos.co"
        );
    }

    #[test]
    fn a_sequencer_nothing_listens_on_is_a_transport_failure() {
        let store = Store::open("http://127.0.0.1:9", PROGRAM);
        let addr = AccountAddr::from(&Ed25519SigningKey::generate().verifying_key());
        assert!(matches!(store.fetch(&addr), Err(StoreError::Transport(_))));
    }

    #[test]
    fn a_store_nothing_listens_on_is_named_plainly() {
        let addr = AccountAddr::from(&Ed25519SigningKey::generate().verifying_key());
        let error = ChatStore::new("http://127.0.0.1:9")
            .fetch(&addr)
            .unwrap_err();
        assert_eq!(
            error.to_string(),
            "store request failed: connection refused"
        );
    }

    pub(crate) fn serve(answers: Vec<String>) -> String {
        serve_bytes(answers.into_iter().map(String::into_bytes).collect())
    }

    /// Answers each connection with the next canned response, after reading
    /// the whole request so the client is never cut off mid-send.
    pub(crate) fn serve_bytes(answers: Vec<Vec<u8>>) -> String {
        use std::io::{BufRead, BufReader, Read, Write};
        let listener = std::net::TcpListener::bind("127.0.0.1:0").unwrap();
        let url = format!("http://{}", listener.local_addr().unwrap());
        std::thread::spawn(move || {
            for answer in answers {
                let (stream, _) = listener.accept().unwrap();
                let mut reader = BufReader::new(stream);
                let mut length = 0;
                loop {
                    let mut line = String::new();
                    reader.read_line(&mut line).unwrap();
                    if let Some(value) = line.to_ascii_lowercase().strip_prefix("content-length:") {
                        length = value.trim().parse().unwrap();
                    }
                    if line == "\r\n" {
                        break;
                    }
                }
                reader.read_exact(&mut vec![0; length]).unwrap();
                reader.into_inner().write_all(&answer).unwrap();
            }
        });
        url
    }

    /// One HTTP response, with `extra` headers and `body`.
    pub(crate) fn answer(status: &str, extra: &str, body: &str) -> String {
        format!(
            "HTTP/1.1 {status}\r\n{extra}Content-Length: {}\r\nConnection: close\r\n\r\n{body}",
            body.len()
        )
    }

    pub(crate) fn json_error(status: &str, error: &str) -> String {
        answer(
            status,
            "Content-Type: application/json\r\n",
            &format!("{{\"error\":\"{error}\"}}"),
        )
    }

    fn addr() -> AccountAddr {
        AccountAddr::from(&Ed25519SigningKey::generate().verifying_key())
    }

    #[test]
    fn a_body_cut_short_is_the_connection_failing_not_the_log() {
        let store = ChatStore::new(&serve(vec![
            "HTTP/1.1 200 OK\r\nContent-Length: 200\r\nConnection: close\r\n\r\n0123456789".into(),
        ]));
        assert!(matches!(
            store.fetch(&addr()),
            Err(StoreError::Transport(_))
        ));
    }

    #[test]
    fn the_routes_own_404_is_an_account_that_never_published() {
        let store = ChatStore::new(&serve(vec![json_error(
            "404 Not Found",
            "no account log for account_addr",
        )]));
        assert_eq!(store.fetch(&addr()).unwrap(), None);
    }

    /// The default store answers every unknown path this way, so reading it as
    /// "nothing published" would hide that the route is not there at all.
    #[test]
    fn a_bare_404_is_a_store_without_the_route() {
        let store = ChatStore::new(&serve(vec![answer("404 Not Found", "", "")]));
        assert!(matches!(store.fetch(&addr()), Err(StoreError::NoRoute)));
    }

    #[test]
    fn a_redirected_publish_is_not_a_publish() {
        let mut store = ChatStore::new(&serve(vec![answer(
            "301 Moved Permanently",
            "Location: /elsewhere\r\n",
            "",
        )]));
        let (addr, log) = signed(&Ed25519SigningKey::generate(), &["saro"]);
        assert!(matches!(
            store.publish(&addr, &log),
            Err(StoreError::Status { code: 301, .. })
        ));
    }

    #[test]
    fn a_refusal_carries_the_stores_reason() {
        let mut store = ChatStore::new(&serve(vec![json_error(
            "400 Bad Request",
            "payload: not a log",
        )]));
        let (addr, log) = signed(&Ed25519SigningKey::generate(), &["saro"]);
        let error = store.publish(&addr, &log).unwrap_err().to_string();
        assert_eq!(error, "store returned 400: payload: not a log");
    }

    #[test]
    fn a_base_url_keeps_one_spelling() {
        assert_eq!(
            ChatStore::new("https://example.test/").base_url(),
            "https://example.test"
        );
    }
}
