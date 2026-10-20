// Sweep35 stall metrics from replays (tools/cloud/sweep35.md). One snapshot per round (first state seen of each round,
// sampled every --stride timeline positions). Per team:
//   stall_rounds: alive champion-rounds in runs of >= 20 rounds with no change of position, gems, gold, health or kills
//                 (a channel, capture, fight, heal or purchase changes one of them within 20 rounds);
//   altar_stall_rounds: rounds in runs of >= 20 where a carrier (gems > 0) stays within 3 tiles of an altar and its gems
//                 never fall (no tribute completes).
// usage: node tools/cloud/stalls.mjs <maps35.json> <mapgate.json ...> [--stride N] > out.jsonl
import fs from 'node:fs';
import { openEngine, openReplay, replayBytes } from '../top10/decode.mjs';
const num = (v) => (typeof v === 'bigint' ? Number(v) : v);
const RUN = 20;
async function one(file, altars, stride) {
  const b = replayBytes(file); const E = await openEngine(b); const R = openReplay(E, b);
  try {
    const snaps = []; let last = -1;
    for (let p = 0; p <= R.size; p += stride) {
      R.seek(p); const s = R.state();
      if (!s.champions.length || s.round === last) continue;
      last = s.round;
      snaps.push(s.champions.map((c) => [c.team, c.alive ? 1 : 0, c.position?.x ?? -1, c.position?.y ?? -1, num(c.gems), num(c.gold),
        Math.round(c.health), num(c.kills)]));
    }
    const n = snaps.length ? snaps[0].length : 0;
    const out = [0, 1].map(() => ({ stall_rounds: 0, altar_stall_rounds: 0, worst_stall: 0, worst_at: null }));
    const nearAltar = (x, y) => altars.some(([ax, ay]) => Math.max(Math.abs(ax - x), Math.abs(ay - y)) <= 3);
    for (let i = 0; i < n; ++i) {
      const team = snaps[0][i][0];
      let run = 0, arun = 0, start = 0;
      const close = (r) => {
        if (run >= RUN) { out[team].stall_rounds += run; if (run > out[team].worst_stall) { out[team].worst_stall = run; out[team].worst_at = start; } }
        run = 0;
      };
      const aclose = () => { if (arun >= RUN) out[team].altar_stall_rounds += arun; arun = 0; };
      for (let r = 1; r < snaps.length; ++r) {
        const a = snaps[r - 1][i], c = snaps[r][i];
        if (!a || !c) { close(r); aclose(); continue; }
        const same = a[1] && c[1] && a[2] === c[2] && a[3] === c[3] && a[4] === c[4] && a[5] === c[5] && a[6] === c[6] && a[7] === c[7];
        if (same) { if (run === 0) start = r; run++; } else close(r);
        const carrier = c[1] && c[4] > 0 && nearAltar(c[2], c[3]) && a[1] && c[4] >= a[4];
        if (carrier) arun++; else aclose();
      }
      close(snaps.length); aclose();
    }
    return { rounds: snaps.length, teams: out };
  } finally { R.close(); }
}
const args = process.argv.slice(2); const si = args.indexOf('--stride'); const stride = si >= 0 ? Number(args[si + 1]) : 8;
const files = args.filter((x, i) => i > 0 && x !== '--stride' && (si < 0 || i !== si + 1));
const maps = JSON.parse(fs.readFileSync(args[0], 'utf8'));
for (const f of files) {
  const mg = JSON.parse(fs.readFileSync(f, 'utf8'));
  for (const g of mg.results) {
    if (!g.replay || !fs.existsSync(g.replay)) continue;
    try {
      const m = await one(g.replay, maps[g.map]?.altars ?? [], stride); const us = g.side === 'A' ? 0 : 1;
      console.log(JSON.stringify({ bot: mg.bot, opp: g.opp, map: g.map, side: g.side, salt: g.salt, us: m.teams[us], them: m.teams[1 - us], rounds: m.rounds }));
    } catch (e) { console.log(JSON.stringify({ bot: mg.bot, opp: g.opp, map: g.map, side: g.side, salt: g.salt, error: e.message })); }
  }
}
