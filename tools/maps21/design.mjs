// usage: node tools/maps21/design.mjs <engine.wasm> <map> <type> -> raw dump of Game_API_Champion_Design (ints/floats)
import fs from 'node:fs'; import { WASI } from 'node:wasi';
const [wasm, mapf, ty] = process.argv.slice(2);
const wasi = new WASI({ version: 'preview1', args: [], env: {}, returnOnExit: true });
const inst = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(wasm)), { wasi_snapshot_preview1: wasi.wasiImport });
wasi.initialize(inst); const X = inst.exports; const out = X.Engine_Malloc(4096); const dv = () => new DataView(X.memory.buffer);
const b = fs.readFileSync(mapf); const p = X.Engine_Malloc(b.length); new Uint8Array(X.memory.buffer).set(b, p);
for (const a of [[p, b.length, 0, out], [p, b.length, out, out + 8]]) { dv().setBigUint64(out, 0n, true); const rc = X.Game_FromMapFileBytes(...a); console.log('rc', rc, dv().getBigUint64(out, true)); if (!rc) break; }
const g = dv().getBigUint64(out, true);
const o2 = out + 1024; new Uint8Array(X.memory.buffer, o2, 2048).fill(0);
const rc = X.Game_API_Champion_Design(g, Number(ty), o2); console.log('design rc', rc);
const r = []; for (let i = 0; i < 128; i++) { const I = dv().getInt32(o2 + 4 * i, true), F = dv().getFloat32(o2 + 4 * i, true); r.push(i + ':' + (Math.abs(F) > 1e-3 && Math.abs(F) < 1e5 && !Number.isInteger(F) ? F.toFixed(3) : I)); }
console.log(r.join(' '));
