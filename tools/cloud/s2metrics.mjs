// S2 minimal metrics (field names per tools/cloud/v5-common.md): per replay and team, the draft, deaths, locks placed,
// gear points per champion at round 200 (shard equivalents, tools/v3/a2_fights.py stats_from_items), unspent gold per
// champion at rounds 100 and 200, and tributed gems. Snapshots every --stride timeline positions (default 10).
// usage: CAMBC_ENGINE_CACHE=data/top10/engine node tools/cloud/s2metrics.mjs <mapgate.json> [--stride N] > out.jsonl
import fs from 'node:fs';
import { openEngine, openReplay, replayBytes } from '../top10/decode.mjs';
const num = (v) => (typeof v === 'bigint' ? Number(v) : v);
function gearPts(items) {
  let p = 0;
  for (let idx = 0; idx < items.length; ++idx) {
    const n = num(items[idx]); if (!n) continue;
    const fam = Math.floor(idx / 6), emb = idx % 2;
    p += !emb ? n : fam === 1 ? 6 * n : fam === 6 ? 7 * n : 5 * n;
  }
  return p;
}
async function one(file, stride) {
  const b = replayBytes(file); const E = await openEngine(b); const R = openReplay(E, b);
  try {
    const T = [0, 1].map(() => ({ deaths: 0, locks_placed: 0, gear_200: null, gold_100: null, gold_200: null }));
    let prev = null;
    const at = (s, r, key, f) => { if (s.round >= r) for (const t of [0, 1]) if (T[t][key] === null) {
      const cs = s.champions.filter((c) => c.team === t); T[t][key] = cs.reduce((a, c) => a + f(c), 0) / cs.length; } };
    for (let p = 0; p <= R.size; p += stride) {
      R.seek(p); const s = R.state(); if (!s.champions.length) continue;
      if (prev && prev.champions.length === s.champions.length)
        s.champions.forEach((c, i) => {
          const o = prev.champions[i];
          if (o.alive && !c.alive) T[c.team].deaths++;
          if (o.alive && c.alive && num(o.consumables[2]) > 0 && num(c.consumables[2]) === 0) T[c.team].locks_placed++;
        });
      at(s, 100, 'gold_100', (c) => num(c.gold)); at(s, 200, 'gold_200', (c) => num(c.gold)); at(s, 200, 'gear_200', (c) => gearPts(c.items));
      prev = s;
    }
    R.seek(R.size); const last = R.state();
    return { teams: T, gems: last.submittedGems.map(num), drafts: last.drafts.map((d) => Array.from(d).map(num)), round: last.round };
  } finally { R.close(); }
}
const args = process.argv.slice(2); const si = args.indexOf('--stride'); const stride = si >= 0 ? Number(args[si + 1]) : 10;
const mg = JSON.parse(fs.readFileSync(args[0], 'utf8'));
for (const g of mg.results) {
  if (!g.replay || !fs.existsSync(g.replay)) continue;
  try {
    const m = await one(g.replay, stride); const us = g.side === 'A' ? 0 : 1;
    console.log(JSON.stringify({ map: g.map, side: g.side, salt: g.salt, opp: g.opp, win: g.win, us: m.teams[us], them: m.teams[1 - us],
      draft_us: m.drafts[us], draft_them: m.drafts[1 - us], gems: [m.gems[us], m.gems[1 - us]], round: m.round }));
  } catch (e) { console.log(JSON.stringify({ map: g.map, side: g.side, error: e.message })); }
}
