// Probe maps for bots/arena-probe: scenario n -> width 31 + 2n, height 31, rotational symmetry, A spawn x0..3 y0..3.
// usage: node tools/arena/mkprobe.mjs <outdir>
import fs from 'node:fs'; import path from 'node:path';
import { openEngine, Builder, validate } from './mb1.mjs';
const outdir = process.argv[2] || 'maps-arena'; fs.mkdirSync(outdir, { recursive: true });
const E = openEngine();
const S = [
  { name: 'range_wolf', champs: 1, camps: [['WOLF', 16, 2]] },
  { name: 'range_elk', champs: 1, camps: [['ELK', 16, 2]] },
  { name: 'range_monolith', champs: 1, camps: [['MONOLITH', 16, 2]] },
  { name: 'rewards', champs: 5, camps: [['GEM_CLUSTER', 10, 2], ['GOLD_CLUSTER', 10, 6], ['WOLF', 14, 2], ['ELK', 14, 8],
      ['GEM_CLUSTER', 8, 12], ['GEM_CLUSTER', 9, 12], ['GEM_CLUSTER', 10, 12], ['GOLD_CLUSTER', 14, 12], ['GOLD_CLUSTER', 15, 12], ['GOLD_CLUSTER', 16, 12]] },
  ...[4, 5, 6, 7, 8].map((n) => ({ name: `target${n}`, champs: 1, camps: [['ELK', 15 + n, 15]] })),
  { name: 'respawn_own', champs: 1, camps: [['GEM_CLUSTER', 8, 2], ['GEM_CLUSTER', 8, 8], ['GEM_CLUSTER', 8, 14]] },
  { name: 'respawn_enemy', champs: 1, camps: [['GEM_CLUSTER', 25, 15]] },
  { name: 'contest', champs: 1, camps: [['GEM_CLUSTER', 26, 14], ['GEM_CLUSTER', 26, 15]] },   // 26,16 is the mirror of 26,14
  { name: 'contest_adjacent', champs: 1, camps: [['GEM_CLUSTER', 27, 15]] },
  { name: 'ally_adjacent', champs: 2, camps: [['GEM_CLUSTER', 28, 15]] },
];
S.forEach((s, n) => {
  const b = new Builder(E, 31 + 2 * n, 31, 'ROTATIONAL180'); b.champs(s.champs); b.spawnA(0, 0, 3, 3);
  b.altar(1, 6);
  for (const [k, x, y] of s.camps) b.camp(k, x, y);
  const buf = b.encode(); const rc = validate(E, buf);
  fs.writeFileSync(path.join(outdir, `p${String(n).padStart(2, '0')}_${s.name}.map26a`), buf);
  console.log(n, s.name, buf.length, rc);
});
