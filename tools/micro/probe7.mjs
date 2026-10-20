import { openBuilderEngine } from './mapbuilder.mjs';
import fs from 'node:fs';
const E = await openBuilderEngine(3);
const { X, out, view, check } = E;
const b2 = fs.readFileSync(process.argv[2]);
const p = X.Engine_Malloc(b2.length); new Uint8Array(X.memory.buffer, p, b2.length).set(b2);
check(X.Game_FromMapFileBytes(p, b2.length, out), 'game'); const g = view().getBigUint64(out, true);
check(X.Game_GetNumCamps(g, out), 'nc'); const nc = view().getUint32(out, true); console.log('game camps', nc);
const xs = [];
for (let i = 0; i < nc; i++) { check(X.Game_GetCampHandle(g, i, out), 'ch'); const ch = view().getBigUint64(out, true); check(X.Game_GetCamp(g, ch, out), 'gc'); xs.push([view().getInt32(out, true), view().getInt32(out + 4, true), view().getInt32(out + 8, true)]); }
console.log(JSON.stringify(xs));
