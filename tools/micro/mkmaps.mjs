// Batch arena writer: node mkmaps.mjs specs.json  (specs: [{file, picksA, picksB, polA, polB, levelA, levelB}])
import fs from 'node:fs';
import { openBuilderEngine } from './mapbuilder.mjs';
import { makeArena, validate } from './mkarena.mjs';
const E = await openBuilderEngine(3);
const specs = JSON.parse(fs.readFileSync(process.argv[2], 'utf8'));
let bad = 0;
for (const s of specs) {
  const buf = makeArena(E, s);
  if (validate(E, buf) !== 0) { bad++; console.log('invalid', s.file); continue; }
  fs.writeFileSync(s.file, buf);
}
console.log(`wrote ${specs.length - bad} maps, ${bad} invalid`);
