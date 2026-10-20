import { openBuilderEngine } from './mapbuilder.mjs';
import fs from 'node:fs';
const E = await openBuilderEngine(3);
const { X, out, view, check } = E;
const load = (f) => { const bytes = fs.readFileSync(f); const p = X.Engine_Malloc(bytes.length); new Uint8Array(X.memory.buffer, p, bytes.length).set(bytes);
  check(X.MapBuilder_FromFileBytes(p, bytes.length, out), 'from'); return view().getBigUint64(out, true); };
const enc = (h) => { check(X.MapBuilder_Encode(h, out, out + 4), 'enc'); const q = view().getUint32(out, true), n = view().getUint32(out + 4, true); return Buffer.from(new Uint8Array(X.memory.buffer, q, n)); };
const tryGame = (buf) => { const p = X.Engine_Malloc(buf.length); new Uint8Array(X.memory.buffer, p, buf.length).set(buf); return X.Game_FromMapFileBytes(p, buf.length, out); };
const h = load('../../maps/packs.map26a');
check(X.MapBuilder_GetNumCamps(h, out), 'nc'); let nc = view().getUint32(out, true);
console.log('camps', nc);
let removed = 0;
for (let y = 0; y < 50; y++) for (let x = 0; x < 50; x++) { for (const fn of ['MapBuilder_RemoveWall','MapBuilder_RemoveBush','MapBuilder_RemoveHole','MapBuilder_RemoveAltar']) { const rc = X[fn](h, x, y); if (rc === 0) removed++; } }
console.log('removed rc0', removed);
// remove camps by handle (index 0 repeatedly)
while (true) { check(X.MapBuilder_GetNumCamps(h, out), 'nc'); nc = view().getUint32(out, true); if (!nc) break; check(X.MapBuilder_GetCampHandle(h, 0, out), 'ch'); const ch = view().getBigUint64(out, true); check(X.MapBuilder_RemoveCamp(h, ch), 'rm'); }
let b = enc(h); console.log('empty', b.length, 'game', tryGame(b));
check(X.MapBuilder_AddAltar(h, 25, 10), 'alt'); b = enc(h); console.log('altar', b.length, 'game', tryGame(b));
check(X.MapBuilder_AddCamp(h, 3, 25, 25, out), 'camp'); b = enc(h); console.log('gold', b.length, 'game', tryGame(b));
fs.writeFileSync('C:/cambc-solo/maps-lab/probe_fromfile.map26a', b);
