// usage: node tools/maps21/designs.mjs <engine.wasm> <map> <out.json>  -> champion (0..12) and camp (0..5) designs decoded per sdk/cpp/imports.h
import fs from 'node:fs'; import { WASI } from 'node:wasi';
const [wasm, mapf, outf] = process.argv.slice(2);
const wasi = new WASI({ version: 'preview1', args: [], env: {}, returnOnExit: true });
const inst = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(wasm)), { wasi_snapshot_preview1: wasi.wasiImport });
wasi.initialize(inst); const X = inst.exports; const out = X.Engine_Malloc(4096); const dv = () => new DataView(X.memory.buffer);
const b = fs.readFileSync(mapf); const p = X.Engine_Malloc(b.length); new Uint8Array(X.memory.buffer).set(b, p);
if (X.Game_FromMapFileBytes(p, b.length, out, out + 8)) throw new Error('game'); const g = dv().getBigUint64(out, true);
const KIND = ['', 'attack_reset', 'damage', 'aoe_damage', 'cc', 'heal', 'dash', 'shield'];
const o2 = out + 1024, W = (i) => dv().getUint32(o2 + 4 * i, true), F = (i) => +dv().getFloat32(o2 + 4 * i, true).toFixed(4), I = (i) => dv().getInt32(o2 + 4 * i, true);
function ab(s) { const k = W(s), r = { kind: KIND[k] || k, cd: I(s + 1), charges: W(s + 2), targets: W(s + 3), attrs: W(s + 4) };
  if (k === 2 || k === 5) Object.assign(r, { range: F(s + 5), mod: F(s + 6) });
  else if (k === 3 || k === 6) Object.assign(r, { range: F(s + 5), radius: F(s + 6), mod: F(s + 7), dur: I(s + 8) });
  else if (k === 4 || k === 7) Object.assign(r, { range: F(s + 5), mod: F(s + 6), dur: I(s + 7) });
  else r.payload = W(s + 5); return r; }
const res = { champions: {}, camps: {} };
for (let t = 0; t < 13; t++) { new Uint8Array(X.memory.buffer, o2, 2048).fill(0); if (X.Game_API_Champion_Design(g, t, o2)) continue;
  res.champions[t] = { vision_type: W(1), passive: W(2) & 0xff, a1: ab(3), a2: ab(12), attack_type: W(21) & 0xff,
    vis: I(22), move: I(23), range: I(24), hp: F(25), hp_g: F(26), str: F(27), str_g: F(28), foc: F(29), foc_g: F(30), campmult: F(31) }; }
for (let t = 0; t < 6; t++) { new Uint8Array(X.memory.buffer, o2, 2048).fill(0); if (X.Game_API_Camp_Design(g, t, o2)) continue;
  res.camps[t] = { passive: W(2) & 0xff, a1: ab(3), a2: ab(12), attack_type: W(21) & 0xff,
    vis: F(22), move: I(23), range: I(24), hp: F(25), hp_g: F(26), str: F(27), str_g: F(28), foc: F(29), foc_g: F(30) }; }
fs.writeFileSync(outf, JSON.stringify(res, null, 1));
