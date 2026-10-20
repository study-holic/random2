// usage: node tools/v3/camps0.mjs <replay ...> -> one JSON line per replay: map camps (type, x, y) and team spawns from the
// first state where every camp has a position; also lists top-level state keys once (for altar/spawn discovery).
import { openEngine, openReplay, replayBytes } from './decode.mjs';
import path from 'node:path';
let shown = false;
for (const f of process.argv.slice(2)) {
  const bytes = replayBytes(f); const E = await openEngine(bytes); const R = openReplay(E, bytes);
  try {
    let s = null;
    for (let p = 0; p <= Math.min(R.size, 400); p += 1) { R.seek(p); s = R.state(); if (s.camps.length && s.camps.every((c) => c.position) && s.champions.length && s.champions.every((c) => c.position)) break; }
    if (!shown) { console.error('state keys:', Object.keys(s).join(','), '| camp keys:', Object.keys(s.camps[0] || {}).join(',')); shown = true; }
    const out = { file: path.basename(f), round: s.round, camps: s.camps.map((c) => [c.type, c.position.x, c.position.y]),
      champs: s.champions.map((c) => [c.team, c.type, c.position.x, c.position.y]) };
    for (const k of Object.keys(s)) if (/altar|spawn|wall|map|width|height/i.test(k)) out[k] = s[k];
    console.log(JSON.stringify(out, (k, v) => (typeof v === 'bigint' ? Number(v) : v)));
  } finally { R.close(); }
}
