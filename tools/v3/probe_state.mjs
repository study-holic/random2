// usage: node probe_state.mjs <replay> -> prints timing and the shape of one decoded state (for field discovery)
import { openEngine, openReplay, replayBytes } from './decode.mjs';
const f = process.argv[2];
const t0 = Date.now();
const bytes = replayBytes(f);
const E = await openEngine(bytes);
const R = openReplay(E, bytes);
console.log('engine patch', E.patch, 'timeline size', R.size, 'open ms', Date.now() - t0);
R.seek(Math.floor(R.size / 2));
const s = R.state();
const shape = (o, d = 0) => (o === null ? 'null' : Array.isArray(o) ? `[${o.length}]` + (o.length && d < 2 ? shape(o[0], d + 1) : '') : typeof o === 'object' ? '{' + Object.entries(o).map(([k, v]) => k + ':' + (d < 2 ? shape(v, d + 1) : typeof v)).join(',') + '}' : typeof o === 'bigint' ? 'bigint' : typeof o);
console.log(shape(s));
console.log('step', JSON.stringify(s.step, (k, v) => (typeof v === 'bigint' ? Number(v) : v)));
console.log('champ0', JSON.stringify(s.champions[0], (k, v) => (typeof v === 'bigint' ? Number(v) : v)));
R.close();
