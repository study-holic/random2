import { openBuilderEngine } from './mapbuilder.mjs';
import fs from 'node:fs';
const E = await openBuilderEngine(3);
const { X, out, view, check } = E;
const bytes = fs.readFileSync(process.argv[2]); const p = X.Engine_Malloc(bytes.length); new Uint8Array(X.memory.buffer, p, bytes.length).set(bytes);
check(X.MapBuilder_FromFileBytes(p, bytes.length, out), 'from'); const h = view().getBigUint64(out, true);
check(X.MapBuilder_GetTeamASpawnArea(h, out), 'sa'); const v = view(); console.log('spawnA', [0,4,8,12].map(o => v.getInt32(out+o, true)));
check(X.MapBuilder_GetSymmetry(h, out), 'sym'); console.log('sym', view().getInt32(out, true));
