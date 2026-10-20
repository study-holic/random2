// Replay -> compact trajectory JSON (rounds + champion action log). Adapted from ext/tarot/tools/replay/extract.mjs (Tarot Club).
// Extra over Tarot: engine selection by replay header (v0_0_1..3), ability slot/type on casts, consumable mask and item count per champion,
// stun flag, and per-action timing. Output is data; derive_events.py turns it into deaths/tributes/recalls/locks.
// usage: node extract_pilot.mjs <replay.repl26a|bundle.zip ...> --out <dir> [--stride N]
// stride 1 = every timeline position (full action log, ~1.4 ms/step); stride N>1 = round-level snapshots only (cost ~ steps/N).
import fs from 'node:fs';
import path from 'node:path';
import { openEngine, openReplay, replayBytes } from './decode.mjs';

const num = (v) => (typeof v === 'bigint' ? Number(v) : v);
const mask = (a) => (a ? Array.from(a).reduce((m, v, i) => m | ((v ? 1 : 0) << i), 0) : 0);
const cnt = (a) => (a ? Array.from(a).reduce((m, v) => m + (v ? 1 : 0), 0) : 0);
const itemKey = (c) => Array.from(c.items || []).map((v, j) => (v ? j + ':' + v : '')).filter(Boolean).join(',');
const snapshot = (s, final = false) => ({
  r: s.round, ...(final ? { final: true } : {}), sg: s.submittedGems.map(num),
  c: s.champions.map((c) => [c.team, c.type, c.alive ? 1 : 0, c.position?.x ?? -1, c.position?.y ?? -1,
    Math.round(c.health), Math.round(c.maxHealth), c.gems, c.gold, c.kills, mask(c.consumables), cnt(c.items), c.stunSource ? 1 : 0]),
  g: s.groups.map((g) => [g.rewardAvailable ? 1 : 0, g.captureTeam]),
});

export function extract(file, stride = 1) {
  return (async () => {
    const bytes = replayBytes(file);
    const E = await openEngine(bytes);
    const R = openReplay(E, bytes);
    const rounds = [], actions = [];
    let lastRound = -1, prev = null; const prevItems = [];
    try {
      for (let p = 0; p <= R.size; p += stride) {
        R.seek(p);
        const s = R.state(), st = s.step;
        if (s.round !== lastRound && s.champions.length) {
          const snap = snapshot(s), it = [];
          s.champions.forEach((c, i) => { const k = itemKey(c); if (k !== (prevItems[i] ?? '')) { it.push([i, k]); prevItems[i] = k; } });
          if (it.length) snap.it = it;
          rounds.push(snap); lastRound = s.round;
        }
        if (stride === 1 && st.phase === 10 && st.actor.kind === 1 && prev) {
          const a = st.actor.index, c1 = s.champions[a], hit = [];
          s.champions.forEach((c, i) => { const o = prev.champions[i]; if (o && c.health < o.health) hit.push(['c', i, Math.round(o.health - c.health)]); });
          s.camps.forEach((c, i) => { const o = prev.camps[i]; if (o && c.health < o.health) hit.push(['m', i, Math.round(o.health - c.health)]); });
          actions.push([st.round, a, st.kind, st.from?.x ?? -1, st.from?.y ?? -1, st.to?.x ?? -1, st.to?.y ?? -1, c1?.gems ?? 0, c1?.gold ?? 0, hit,
            st.abilitySlot ?? -1, st.abilityType ?? -1, mask(prev.champions[a]?.consumables), mask(c1?.consumables)]);
        }
        prev = s;
      }
      R.seek(R.size);
      const last = R.state();
      rounds.push(snapshot(last, true));
      return { engine: E.patch, steps: R.size, rounds, actions, camps: last.camps.map((c) => [c.type, c.position?.x ?? -1, c.position?.y ?? -1]),
        outcome: last.outcome, finalRound: last.round, drafts: last.drafts.map((d) => Array.from(d).map(num)) };
    } finally { R.close(); }
  })();
}

const args = process.argv.slice(2);
const oi = args.indexOf('--out'), si = args.indexOf('--stride');
const outDir = oi >= 0 ? args[oi + 1] : null;
const stride = si >= 0 ? Number(args[si + 1]) : 1;
const flagIdx = [oi, si].filter((k) => k >= 0).flatMap((k) => [k, k + 1]); // (was: si = -1 dropped args[0] when --stride was absent)
const files = args.filter((x, i) => !flagIdx.includes(i));
if (!outDir || !files.length) { console.error('usage: node extract_pilot.mjs <replay|zip ...> --out <dir>'); process.exit(2); }
fs.mkdirSync(outDir, { recursive: true });
for (const f of files) {
  const out = path.join(outDir, path.basename(f).replace(/\.(zip|repl26a)$/, '') + '.json');
  if (fs.existsSync(out)) continue; // resumable
  const t0 = Date.now();
  try {
    const d = await extract(f, stride);
    fs.writeFileSync(out, JSON.stringify(d));
    console.log(JSON.stringify({ file: path.basename(f), stride, ms: Date.now() - t0, engine: d.engine, steps: d.steps, rounds: d.rounds.length, actions: d.actions.length, bytes: fs.statSync(out).size }));
  } catch (e) { console.log(JSON.stringify({ file: path.basename(f), error: e.message })); }
}
