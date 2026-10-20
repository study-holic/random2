import { openBuilderEngine } from './mapbuilder.mjs';
import fs from 'node:fs';
for (const patch of [1,2,3]) {
const E = await openBuilderEngine(patch);
const bytes = fs.readFileSync(process.argv[2]);
const p = E.X.Engine_Malloc(bytes.length);
new Uint8Array(E.X.memory.buffer, p, bytes.length).set(bytes);
try {
E.check(E.X.MapBuilder_FromFileBytes(p, bytes.length, E.out), 'from');
const h = E.view().getBigUint64(E.out, true);
E.check(E.X.MapBuilder_Encode(h, E.out, E.out + 4), 'enc');
const q = E.view().getUint32(E.out, true), n = E.view().getUint32(E.out + 4, true);
const buf = Buffer.from(new Uint8Array(E.X.memory.buffer, q, n));
console.log(patch, 'orig', bytes.length, 'reenc', n, 'equal', Buffer.compare(buf, bytes) === 0, buf.subarray(0,16).toString('hex'));
} catch (e) { console.log(patch, String(e)); }
}
