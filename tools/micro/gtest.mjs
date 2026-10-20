import { openBuilderEngine } from './mapbuilder.mjs';
import fs from 'node:fs';
const E = await openBuilderEngine(3);
for (const f of process.argv.slice(2)) {
  const bytes = fs.readFileSync(f);
  const p = E.X.Engine_Malloc(bytes.length);
  new Uint8Array(E.X.memory.buffer, p, bytes.length).set(bytes);
  try { E.check(E.X.Game_FromMapFileBytes(p, bytes.length, E.out), 'game'); console.log(f, 'ok'); } catch (e) { console.log(f, String(e)); }
}
