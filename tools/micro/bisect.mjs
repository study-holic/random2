import fs from 'node:fs';
import { openBuilderEngine } from './mapbuilder.mjs';
import { validate } from './mkarena.mjs';
const E = await openBuilderEngine(3);
const { X, out, view, check } = E;
function base() {
  const bytes = fs.readFileSync('../../maps/pilgrimage.map26a');
  const p = X.Engine_Malloc(bytes.length); new Uint8Array(X.memory.buffer, p, bytes.length).set(bytes);
  check(X.MapBuilder_FromFileBytes(p, bytes.length, out), 'from'); const h = view().getBigUint64(out, true);
  for (let y = 0; y < 65; y++) for (let x = 0; x < 50; x++) for (const fn of ['MapBuilder_RemoveWall', 'MapBuilder_RemoveBush', 'MapBuilder_RemoveHole', 'MapBuilder_RemoveAltar']) X[fn](h, x, y);
  for (;;) { check(X.MapBuilder_GetNumCamps(h, out), 'nc'); if (!view().getUint32(out, true)) break; check(X.MapBuilder_GetCampHandle(h, 0, out), 'ch'); check(X.MapBuilder_RemoveCamp(h, view().getBigUint64(out, true)), 'rm'); }
  return h;
}
const enc = (h) => { check(X.MapBuilder_Encode(h, out, out + 4), 'encode'); const q = view().getUint32(out, true), n = view().getUint32(out + 4, true); return Buffer.from(new Uint8Array(X.memory.buffer, q, n)); };
const spawn = (h, a, b, c, d) => { const v = view(); v.setInt32(out, a, true); v.setInt32(out + 4, b, true); v.setInt32(out + 8, c, true); v.setInt32(out + 12, d, true); return X.MapBuilder_SetTeamASpawnArea(h, out); };
let h = base(); console.log('cleared', validate(E, enc(h)));
h = base(); console.log('spawn rc', spawn(h, 0, 6, 1, 43), validate(E, enc(h)));
h = base(); console.log('spawn small rc', spawn(h, 0, 6, 1, 9), validate(E, enc(h)));
h = base(); console.log('spawn 4x4 rc', spawn(h, 0, 20, 3, 23), validate(E, enc(h)));
h = base(); X.MapBuilder_AddWall(h, 10, 10); console.log('wall', validate(E, enc(h)));
h = base(); X.MapBuilder_AddHole(h, 10, 50); console.log('hole', validate(E, enc(h)));
for (const r of [[0,20,1,23],[0,20,3,21],[0,6,3,43],[0,20,2,22],[0,20,4,24],[0,20,3,24],[1,20,4,23]]) { h = base(); console.log(JSON.stringify(r), spawn(h, ...r), validate(E, enc(h))); }
