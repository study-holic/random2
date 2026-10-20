// usage: node probe_steps.mjs <replay> : distinct (phase,kind) step records, ability types, stun and item examples
import { openEngine, openReplay, replayBytes } from './decode.mjs';
const bytes = replayBytes(process.argv[2]);
const E = await openEngine(bytes); const R = openReplay(E, bytes);
const num = (v) => (typeof v === 'bigint' ? Number(v) : v);
const seen = new Map(); const ab = new Map(); let stunEx = []; let t0 = Date.now(); let prev = null; const itemEx = [];
for (let p = 0; p <= R.size; p++) {
  R.seek(p); const s = R.state(), st = s.step;
  const key = `${st.phase}/${st.kind}`;
  if (!seen.has(key)) seen.set(key, { n: 0, ex: JSON.stringify(st, (k, v) => num(v)) });
  seen.get(key).n++;
  if (st.abilityType !== undefined) { const k = `slot${st.abilitySlot}/type${st.abilityType}/kind${st.kind}/phase${st.phase}`; ab.set(k, (ab.get(k) || 0) + 1); }
  s.champions.forEach((c, i) => { if (c.stunSource && stunEx.length < 3) stunEx.push([s.round, i, JSON.stringify(c.stunSource, (k, v) => num(v))]); });
  if (prev) s.champions.forEach((c, i) => { const o = prev.champions[i]; if (o && itemEx.length < 6 && Array.from(c.items).join() !== Array.from(o.items).join()) itemEx.push([s.round, i, c.type, Array.from(c.items).map((v, j) => v ? j + ':' + v : '').filter(Boolean).join(' '), 'gold', o.gold, c.gold]); });
  prev = s;
}
console.log('ms', Date.now() - t0, 'steps', R.size);
for (const [k, v] of [...seen].sort()) console.log(k, v.n, v.ex.slice(0, 260));
console.log([...ab].sort().map(([k, v]) => k + '=' + v).join('\n'));
console.log('stun', JSON.stringify(stunEx)); console.log('items', JSON.stringify(itemEx));
R.close();
