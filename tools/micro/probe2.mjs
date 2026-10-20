import { openBuilderEngine, Builder } from './mapbuilder.mjs';
import fs from 'node:fs';
const E = await openBuilderEngine(3);
const tryGame = (buf) => { const p = E.X.Engine_Malloc(buf.length); new Uint8Array(E.X.memory.buffer, p, buf.length).set(buf);
  const rc = E.X.Game_FromMapFileBytes(p, buf.length, E.out); return rc; };
const variants = {
  camp: (b) => { b.altar(14, 3); b.camp('GOLD_CLUSTER', 15, 15); },
  wolf: (b) => { b.altar(14, 3); b.camp('WOLF', 10, 20); },
  noaltar_camp: (b) => { b.camp('WOLF', 10, 20); },
  bigspawn: (b) => { b.altar(14, 3); b.camp('WOLF', 10, 20); b.spawnA(1, 1, 3, 3); },
};
for (const [k, f] of Object.entries(variants)) {
  const b = new Builder(E, 30, 30); b.champs(3); b.spawnA(1, 1, 2, 3); f(b);
  try { const buf = b.encode(); console.log(k, buf.length, 'game rc', tryGame(buf)); fs.writeFileSync(`C:/cambc-solo/maps-lab/probe_${k}.map26a`, buf); } catch (e) { console.log(k, String(e)); }
}
