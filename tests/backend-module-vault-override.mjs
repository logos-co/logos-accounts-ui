// LOGOS_ACCOUNTS_VAULT_DIR names the vault, so the one the ui-host backend
// kept is left where it is. Run with `nix run .#ui-tests`.
import { existsSync, mkdirSync, mkdtempSync, readdirSync, statSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join, resolve } from "node:path";

process.env.LOGOS_ACCOUNTS_STORE_URL = "memory";
const vault = join(mkdtempSync(join(tmpdir(), "accounts-override-")), "vault");
process.env.LOGOS_ACCOUNTS_VAULT_DIR = vault;
const PROBE = "migration-probe.txt";
const legacyVault = join(process.env.BASECAMP_TEST_USER_DIR, "module_data", "accounts_ui", "vault");
mkdirSync(legacyVault, { recursive: true });
writeFileSync(join(legacyVault, PROBE), "kept by the ui-host backend");

const root = process.env.LOGOS_QT_MCP || new URL("../result-mcp", import.meta.url).pathname;
const { test, run } = await import(resolve(root, "test-framework/framework.mjs"));

async function evalOn(app, objectName, expression) {
  const match = ((await app.findByProperty("objectName", objectName)).matches ?? [])[0];
  if (!match) throw new Error(`no ${objectName}`);
  const res = await app.inspector.send("evaluate", { objectId: match.id, expression });
  if (res.error) throw new Error(res.error);
  return res.result;
}

test("vault override: the vault opens", async (app) => {
  let v;
  await app.waitFor(async () => {
    v = await evalOn(app, "vaultProblem", "store.backend.statusText");
    if (!(v && !["Starting", "Could not read the vault"].includes(v))) throw new Error(`store.backend.statusText: ${v}`);
  }, { timeout: 60000, interval: 500, description: "the vault to open" });
});

test("vault override: the old vault is left where it is", async () => {
  if (!existsSync(join(legacyVault, PROBE))) throw new Error("the old vault was moved");
  if (existsSync(join(vault, PROBE))) throw new Error("the old vault was moved into the override");
  const found = [];
  const walk = (dir) => {
    for (const e of readdirSync(dir)) {
      const p = join(dir, e);
      if (statSync(p).isDirectory()) walk(p);
      else if (e === PROBE && p.includes("accounts_ui_backend")) found.push(p);
    }
  };
  walk(join(process.env.BASECAMP_TEST_USER_DIR, "module_data"));
  if (found.length) throw new Error(`moved into the module's data folder: ${found.join(", ")}`);
});

run();
