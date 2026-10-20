import { openBuilderEngine, Builder } from './mapbuilder.mjs';
import fs from 'node:fs';
const E = await openBuilderEngine(3);
const b = new Builder(E, 30, 30);
b.champs(3);
b.spawnA(1, 1, 2, 3);
try { const buf = b.encode(); console.log('encoded', buf.length); fs.writeFileSync('C:/cambc-solo/maps-lab/probe0.map26a', buf); } catch (e) { console.log(String(e)); }
b.altar(14, 3);
try { const buf = b.encode(); console.log('encoded+altar', buf.length); fs.writeFileSync('C:/cambc-solo/maps-lab/probe1.map26a', buf); } catch (e) { console.log(String(e)); }
