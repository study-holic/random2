import fs from 'node:fs'; import { WASI } from 'node:wasi';
const wasm = process.argv[2], mapf = process.argv[3];
const wasi = new WASI({ version: 'preview1', args: [], env: {}, returnOnExit: true });
const inst = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(wasm)), { wasi_snapshot_preview1: wasi.wasiImport });
wasi.initialize(inst); const X = inst.exports;
const out = X.Engine_Malloc(256); const dv = () => new DataView(X.memory.buffer);
const b = fs.readFileSync(mapf); const p = X.Engine_Malloc(b.length); new Uint8Array(X.memory.buffer).set(b, p);
console.log('from', X.MapBuilder_FromFileBytes(p, b.length, out)); const h = dv().getBigUint64(out, true);
const dump = (n) => { const a = []; for (let i = 0; i < n; i++) a.push(dv().getInt32(out + 4 * i, true)); return a.join(','); };
for (const f of ['GetBitboardMetadata', 'GetSymmetry', 'GetNumChampsPerTeam', 'GetTeamASpawnArea', 'GetNumCamps', 'GetNumCampGroups']) { dv().setBigUint64(out, 0n, true); console.log(f, X['MapBuilder_' + f](h, out), dump(6)); }
console.log('camphandle', X.MapBuilder_GetCampHandle(h, 0, out), dump(4));
console.log('camp', X.MapBuilder_GetCamp(h, dv().getBigUint64(out, true), out), dump(6));
