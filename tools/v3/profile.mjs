import { openEngine, openReplay, replayBytes } from './decode.mjs';
const f = process.argv[2];
const bytes = replayBytes(f);
const E = await openEngine(bytes);
let R = openReplay(E, bytes);
const N = R.size;
let t = process.hrtime.bigint(); const ms = () => { const n = process.hrtime.bigint(); const d = Number(n - t) / 1e6; t = n; return d.toFixed(0); };
for (let p = 0; p <= N; p++) R.seek(p);
console.log('seek only all steps', ms(), 'ms for', N);
R.close(); R = openReplay(E, bytes); ms();
for (let p = 0; p <= N; p += 25) { R.seek(p); }
console.log('seek every 25', ms());
R.close(); R = openReplay(E, bytes); ms();
for (let p = 0; p <= N; p += 25) { R.seek(p); R.state(); }
console.log('seek+state every 25', ms());
R.close(); R = openReplay(E, bytes); ms();
for (let p = 0; p <= N; p++) { R.seek(p); R.state(); }
console.log('seek+state all', ms());
R.close();
