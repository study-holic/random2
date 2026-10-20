import { openBuilderEngine } from './mapbuilder.mjs';
import { makeArena } from './mkarena.mjs';
const E = await openBuilderEngine(3);
const { X, out, view, check } = E;
const buf = makeArena(E, { picksA: [4, 0, 9, 10, 8], picksB: [0, 9, 4, 8, 10], polA: 0, polB: 0 });
let p = X.Engine_Malloc(buf.length); new Uint8Array(X.memory.buffer, p, buf.length).set(buf);
check(X.MapBuilder_FromFileBytes(p, buf.length, out), 'from'); const h = view().getBigUint64(out, true);
for (const [x, y] of [[10, 6], [11, 6], [38, 6], [39, 6]]) console.log("add", x, y, X.MapBuilder_AddCamp(h, 3, x, y, out));
check(X.MapBuilder_Encode(h, out, out + 4), 'enc'); const q = view().getUint32(out, true), n = view().getUint32(out + 4, true);
const b2 = Buffer.from(new Uint8Array(X.memory.buffer, q, n));
p = X.Engine_Malloc(b2.length); new Uint8Array(X.memory.buffer, p, b2.length).set(b2);
check(X.Game_FromMapFileBytes(p, b2.length, out), 'game'); const g = view().getBigUint64(out, true);
check(X.Game_GetNumCamps(g, out), 'nc'); const nc = view().getUint32(out, true); console.log('game camps', nc);
check(X.Game_GetNumCampGroups(g, out), 'ng'); console.log('game groups', view().getUint32(out, true));
for (let i = 0; i < nc; i++) { check(X.Game_GetCampHandle(g, i, out), 'ch'); const ch = view().getBigUint64(out, true); check(X.Game_GetCamp(g, ch, out), 'gc'); console.log('camp', view().getInt32(out, true), view().getInt32(out + 4, true), view().getInt32(out + 8, true)); }
