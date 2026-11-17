// The backend runs as the core module accounts_ui_backend ("backend": "module"):
// it is loaded and restricted to this app, and the vault the ui-host backend
// kept is moved into its own data folder. Run with `nix run .#ui-tests`.
import { createPrivateKey, createPublicKey, randomBytes } from "node:crypto";
import { existsSync, lstatSync, mkdirSync, mkdtempSync, readdirSync, readFileSync, statSync, symlinkSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join, resolve } from "node:path";

// A vault the ui-host backend kept: with --user-dir, under the profile's
// module_data/accounts_ui. Written before the app starts. The store is the
// in-process one, so nothing reaches the network.
process.env.LOGOS_ACCOUNTS_STORE_URL = "memory";
const PROBE = "migration-probe.txt";
const legacyVault = join(process.env.BASECAMP_TEST_USER_DIR, "module_data", "accounts_ui", "vault");
mkdirSync(legacyVault, { recursive: true });
writeFileSync(join(legacyVault, PROBE), "kept by the ui-host backend");
// A link to a file outside the vault: neither moved nor followed.
const LINK = "linked-elsewhere.json";
const outside = join(mkdtempSync(join(tmpdir(), "accounts-outside-")), "not-a-key.json");
writeFileSync(outside, "outside the vault");
symlinkSync(outside, join(legacyVault, LINK));

// A real account with no password, as the vault writes one: the file is named
// after the address, the hex of the Ed25519 public key.
const seed = randomBytes(32);
const pkcs8 = Buffer.concat([Buffer.from("302e020100300506032b657004220420", "hex"), seed]);
const ADDRESS = createPublicKey(createPrivateKey({ key: pkcs8, format: "der", type: "pkcs8" }))
  .export({ format: "der", type: "spki" }).subarray(-32).toString("hex");
writeFileSync(join(legacyVault, `${ADDRESS}.open.json`), JSON.stringify({
  logos_account_vault: 1, address: ADDRESS, protection: "none", key: seed.toString("hex"),
  warning: "test key",
}), { mode: 0o600 });

// The relay chosen in the app, kept beside the vault.
const SETTINGS = "vault-settings.ini";
writeFileSync(join(legacyVault, "..", SETTINGS), "[relay]\naddress=\ntoken=\n");

// A file already in the module's vault, under a name the old vault also has.
// The data folder is module_data/<module>/<instance>, and an existing instance
// is reused.
const CLASH = "already-here.txt";
const newVault = join(process.env.BASECAMP_TEST_USER_DIR, "module_data", "accounts_ui_backend", "test", "vault");
mkdirSync(newVault, { recursive: true });
writeFileSync(join(newVault, CLASH), "the module's");
writeFileSync(join(legacyVault, CLASH), "the ui-host backend's");

const root = process.env.LOGOS_QT_MCP || new URL("../result-mcp", import.meta.url).pathname;
const { test, run } = await import(resolve(root, "test-framework/framework.mjs"));

const APP = "accounts_ui";
const BACKEND = `${APP}_backend`;

function sessionLog() {
  const logs = join(process.env.BASECAMP_TEST_USER_DIR, "logs");
  const file = readdirSync(logs).filter((f) => /^basecamp_.*\.log$/.test(f)).sort().pop();
  if (!file) throw new Error(`no session log in ${logs}`);
  return readFileSync(join(logs, file), "utf8");
}

// Evaluated in the context of the object, so the view's ids resolve.
async function evalOn(app, objectName, expression) {
  const match = ((await app.findByProperty("objectName", objectName)).matches ?? [])[0];
  if (!match) throw new Error(`no ${objectName}`);
  const res = await app.inspector.send("evaluate", { objectId: match.id, expression });
  if (res.error) throw new Error(res.error);
  return res.result;
}

test("backend module: the vault opens", async (app) => {
  let v;
  await app.waitFor(async () => {
    v = await evalOn(app, "vaultProblem", "store.backend.statusText");
    if (!(v && !["Starting", "Could not read the vault"].includes(v))) throw new Error(`store.backend.statusText: ${v}`);
  }, { timeout: 60000, interval: 500, description: "the vault to open" });
});

test("backend module: loaded as a core module, restricted to the app", async () => {
  const log = sessionLog();
  const restricted = log.indexOf(`Restricted ${BACKEND} to ${APP}`);
  const loaded = log.indexOf(`Module loaded: ${BACKEND}`);
  if (restricted < 0) throw new Error("no restriction line in the session log");
  if (loaded < 0) throw new Error("no load line for the backend in the session log");
  if (restricted > loaded) throw new Error("the backend was loaded before its restriction");
});

test("backend module: no ui-host deprecation for the app", async () => {
  const hit = sessionLog().split("\n").find((l) => l.includes("DEPRECATED") && l.includes(APP));
  if (hit) throw new Error(`deprecation logged: ${hit}`);
});

test("backend module: no stopped panel", async (app) => {
  const res = await app.findByProperty("objectName", "appStopped.root");
  const shown = (res.matches ?? []).filter((m) => m.visible !== false);
  if (shown.length) throw new Error("the app's stopped panel is shown");
});

test("backend module: the ui-host vault is moved into the module's data folder", async () => {
  const find = (dir) => {
    for (const e of readdirSync(dir)) {
      const p = join(dir, e);
      if (statSync(p).isDirectory()) { const hit = find(p); if (hit) return hit; }
      else if (e === PROBE && p.includes(`${BACKEND}`)) return p;
    }
    return null;
  };
  const moved = find(join(process.env.BASECAMP_TEST_USER_DIR, "module_data"));
  if (!moved) throw new Error(`${PROBE} is not in ${BACKEND}'s data folder`);
  if (existsSync(join(legacyVault, PROBE))) throw new Error("the old copy is still there: it was copied, not moved");
});

test("backend module: a symlink in the old vault is left where it is", async () => {
  const found = [];
  const walk = (dir) => {
    for (const e of readdirSync(dir)) {
      const p = join(dir, e);
      if (lstatSync(p).isDirectory()) walk(p);
      else if (e === LINK && p.includes(BACKEND)) found.push(p);
    }
  };
  walk(join(process.env.BASECAMP_TEST_USER_DIR, "module_data"));
  if (found.length) throw new Error(`followed or moved into the vault: ${found.join(", ")}`);
  if (!lstatSync(join(legacyVault, LINK)).isSymbolicLink()) throw new Error("the link was taken from the old vault");
});

test("backend module: a migrated account is in the list, with its key", async (app) => {
  let account;
  await app.waitFor(async () => {
    const accounts = JSON.parse(await evalOn(app, "vaultProblem", "store.backend.accountsJson"));
    account = accounts.find((a) => a.address === ADDRESS);
    if (!account) throw new Error(`${ADDRESS} is not in ${JSON.stringify(accounts)}`);
  }, { timeout: 30000, interval: 500, description: "the migrated account to be listed" });
  if (!account.managed) throw new Error(`listed without its key: ${JSON.stringify(account)}`);
  if (!existsSync(join(newVault, `${ADDRESS}.open.json`))) throw new Error("its key is not in the module's vault");
});

test("backend module: the relay settings move beside the new vault", async () => {
  if (!existsSync(join(newVault, "..", SETTINGS))) throw new Error(`${SETTINGS} is not beside the module's vault`);
  if (existsSync(join(legacyVault, "..", SETTINGS))) throw new Error(`${SETTINGS} is still beside the old vault`);
});

test("backend module: a file already in the new vault is not overwritten", async () => {
  const kept = readFileSync(join(newVault, CLASH), "utf8");
  if (kept !== "the module's") throw new Error(`overwritten with: ${kept}`);
  const old = join(legacyVault, CLASH);
  if (!existsSync(old) || readFileSync(old, "utf8") !== "the ui-host backend's")
    throw new Error("the old copy was not left where it was");
});

// Two clients for one dependency each mint a token, and the dependency keeps
// one per caller: the other's calls are refused.
test("backend module: no call refused for a stale token", async () => {
  const hit = sessionLog().split("\n").find((l) => /auth token not recognized|rejecting unauthorized call/.test(l));
  if (hit) throw new Error(`refused: ${hit}`);
});

run();
