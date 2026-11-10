//! Where a log is published and read back.
//!
//! The account crates take their provider by value, and this app needs the
//! same store in three places at once (the resolver, each publish, and the
//! read-back that confirms it), so every provider here is a cheap handle over
//! shared state rather than the state itself.
//!
//! Logs live in the account registry on a Logos Execution Zone
//! (logos-messaging/account-registry-lez), one per address. Reads go straight
//! to the sequencer and are verified here. A write is an `Append` transaction,
//! which a funded LEZ account has to pay for, so this library prepares it and
//! hands it to the [`Submit`] the app registers, which sends it through the
//! user's LEZ wallet; the store then waits for the log to land.

use std::sync::{Arc, Mutex};

use account::{AccountProvider, AccountPublisher};
use account_log::{AccountAddr, SignedAccountLog};
use account_registry_client::{AccountId, LezAccountRegError, Registry};
use serde_json::json;
use thiserror::Error;

/// The public LEZ testnet sequencer, used unless the caller names another.
pub const DEFAULT_SEQUENCER_URL: &str = "https://testnet.lez.logos.co";

/// The registry program on the default sequencer. Unset until it is deployed,
/// so a caller must name one.
pub const DEFAULT_REGISTRY_PROGRAM: &str = "";

#[derive(Debug, Error)]
pub enum StoreError {
    #[error("registry request failed: {0}")]
    Transport(String),
    #[error("the registry refused the log: it is not an extension of what it holds")]
    Diverged,
    #[error("the registry returned something that is not this account's log: {0}")]
    Malformed(String),
    #[error("that would take the log past the registry's limit of {} KiB", .0 / 1024)]
    TooLarge(usize),
    #[error("no account registry is configured: {0}")]
    NotConfigured(String),
    #[error("the LEZ wallet did not send the update: {0}")]
    Wallet(String),
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

/// The account registry on a LEZ sequencer.
#[derive(Clone)]
pub struct LezStore {
    sequencer: Arc<str>,
    registry: Result<Registry, Arc<str>>,
    submitter: Submitter,
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

    fn registry(&self) -> Result<&Registry, StoreError> {
        self.registry
            .as_ref()
            .map_err(|reason| StoreError::NotConfigured(reason.to_string()))
    }
}

impl AccountProvider for LezStore {
    type Error = StoreError;

    fn fetch(&self, addr: &AccountAddr) -> Result<Option<SignedAccountLog>, Self::Error> {
        Ok(self.registry()?.fetch(addr)?)
    }
}

impl AccountPublisher for LezStore {
    type Error = StoreError;

    /// Prepares the append against the log on chain, has the submitter send
    /// it, and waits until the chain holds `log`.
    fn publish(&mut self, addr: &AccountAddr, log: &SignedAccountLog) -> Result<(), Self::Error> {
        let registry = self.registry()?;
        let submit = self
            .submitter
            .0
            .lock()
            .expect("submitter lock")
            .clone()
            .ok_or_else(|| StoreError::Wallet("no LEZ wallet is connected".into()))?;
        let prepared = registry.prepare_append(addr, log)?;
        let request = json!({
            "program": hex::encode(registry.program().value()),
            "account": hex::encode(prepared.account.value()),
            "instruction": hex::encode(prepared.instruction_data()),
            "gasLimit": prepared.gas_limit,
        });
        submit(&request.to_string()).map_err(StoreError::Wallet)?;
        Ok(registry.await_append(addr, log, &prepared)?)
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

/// The store this app was pointed at. One type rather than a type parameter,
/// because the choice is made once from the environment and every caller below
/// would otherwise carry it.
#[derive(Clone)]
pub enum Store {
    /// The account registry on a LEZ sequencer.
    Lez(LezStore),
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

    pub fn memory() -> Self {
        Self::Memory(MemoryStore::new())
    }

    pub fn describe(&self) -> String {
        match self {
            Self::Lez(store) => store.describe(),
            Self::Memory(_) => "memory".to_owned(),
        }
    }

    /// See [`LezStore::set_submitter`]. The in-process store needs none.
    pub fn set_submitter(&self, submit: Arc<Submit>) {
        if let Self::Lez(store) = self {
            store.set_submitter(submit);
        }
    }
}

impl AccountProvider for Store {
    type Error = StoreError;

    fn fetch(&self, addr: &AccountAddr) -> Result<Option<SignedAccountLog>, Self::Error> {
        match self {
            Self::Lez(store) => store.fetch(addr),
            Self::Memory(store) => store.fetch(addr),
        }
    }
}

impl AccountPublisher for Store {
    type Error = StoreError;

    fn publish(&mut self, addr: &AccountAddr, log: &SignedAccountLog) -> Result<(), Self::Error> {
        match self {
            Self::Lez(store) => store.publish(addr, log),
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
                .add(context.clone(), EntryData::Text((*value).into()))
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
}
