// usage: node tools/maps21/dump.mjs <engine.wasm (mapeditor v1_0_0)> <outfile.json> <map files...>
// Reads .map26a files through the official MapBuilder_FromFileBytes export. Writes {name:{w,h,sym,champs,spawnA,camps:[[type,x,y]],groups,walls,holes,bushes,altars}}
import fs from 'node:fs'; import path from 'node:path'; import { WASI } from 'node:wasi';
const [wasm, outf, ...files] = process.argv.slice(2);
const wasi = new WASI({ version: 'preview1', args: [], env: {}, returnOnExit: true });
const inst = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(wasm)), { wasi_snapshot_preview1: wasi.wasiImport });
wasi.initialize(inst); const X = inst.exports; const out = X.Engine_Malloc(256);
const dv = () => new DataView(X.memory.buffer); const i32 = (k = 0) => dv().getInt32(out + 4 * k, true);
const res = {};
for (const f of files) {
  const b = fs.readFileSync(f); const p = X.Engine_Malloc(b.length); new Uint8Array(X.memory.buffer).set(b, p);
  if (X.MapBuilder_FromFileBytes(p, b.length, out)) throw new Error('load ' + f);
  const h = dv().getBigUint64(out, true); const g = (n) => { X['MapBuilder_' + n](h, out); return [i32(0), i32(1), i32(2), i32(3)]; };
  const [w, ht] = g('GetBitboardMetadata'); const sym = g('GetSymmetry')[0]; const champs = g('GetNumChampsPerTeam')[0];
  const spawnA = g('GetTeamASpawnArea'); const nc = g('GetNumCamps')[0]; const ng = g('GetNumCampGroups')[0];
  const camps = []; for (let i = 0; i < nc; i++) { X.MapBuilder_GetCampHandle(h, i, out); const ch = dv().getBigUint64(out, true); X.MapBuilder_GetCamp(h, ch, out); camps.push([i32(0), i32(1), i32(2)]); }
  const grp = []; for (let gi = 0; gi < ng; gi++) { X.MapBuilder_GetCampGroupHandle(h, gi, out); const gh = dv().getBigUint64(out, true); X.MapBuilder_GetCampGroup(h, gh, out); const info = [i32(0), i32(1), i32(2), i32(3)]; grp.push({ info, camps: [] }); for (let j = 0; j < info[3]; j++) { X.MapBuilder_GetCampGroupCamp(h, gh, j, out); const ch = dv().getBigUint64(out, true); X.MapBuilder_GetCamp(h, ch, out); grp[gi].camps.push([i32(0), i32(1), i32(2)]); } }
  const lay = { walls: [], holes: [], bushes: [], altars: [] };
  const key = { walls: 'GetWall', holes: 'GetHole', bushes: 'GetBush', altars: 'GetAltar' };
  for (const k in key) for (let y = 0; y < ht; y++) for (let x = 0; x < w; x++) { dv().setInt32(out, 0, true); X['MapBuilder_' + key[k]](h, x, y, out); if (i32(0) & 0xff) lay[k].push([x, y]); }
  res[path.basename(f, '.map26a')] = { w, h: ht, sym, champs, spawnA, camps, groups: ng, grp, ...lay };
}
fs.writeFileSync(outf, JSON.stringify(res));
