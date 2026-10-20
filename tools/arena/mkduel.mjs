// Duel arenas for bots/arena-duel (the w14t layout, built from scratch on the engine 1.0.0 MapBuilder; x-mirror symmetry):
//   50x65; team A spawn x 0..3, y 22..25; connector x 4..5, y 6..43; corridor c = rows 6+8c..11+8c, x 6..43, walled apart.
//   Spec as holes in the wall block: row 50+2s = five 4-bit picks of side s (15 = none); row 51+2s = 16 policy bits,
//   4-bit item level at x 18, 2-bit gem target (x20 gems) at x 22; team-fight mode: rows 54/55 = 3-bit corridor per pick, flag 17,54.
//   Gem clusters: six single-camp groups per corridor end (x 7, 9, 11 on the corridor's top and bottom rows), 42 gems a side.
// usage: node tools/arena/mkduel.mjs <specs.json> <outdir>   specs = [{name, champs, picksA, picksB, polA, polB, gemsA, gemsB, groupsA?, groupsB?}]
import fs from 'node:fs'; import path from 'node:path';
import { openEngine, Builder, validate } from './mb1.mjs';
export const PICK = { ryth: 0, vesper: 1, vendetta: 2, ci: 3, stellar: 4, prism: 5, marcella: 6, willow: 7, ren: 8, varg: 9, tavi: 10, hor: 11 };
export function makeDuel(E, spec) {
  const W = 50, H = 65; const b = new Builder(E, W, H, 'HORIZONTAL');
  b.champs(spec.champs ?? 5); b.spawnA(0, 22, 3, 25);
  const holes = new Set();
  const setBits = (y, x0, n, val) => { for (let i = 0; i < n; i++) if ((val >> i) & 1) holes.add(`${x0 + i},${y}`); };
  for (let s = 0; s < 2; s++) {
    const picks = s === 0 ? spec.picksA : spec.picksB;
    for (let k = 0; k < 5; k++) setBits(50 + 2 * s, 2 + 4 * k, 4, k < picks.length ? picks[k] : 15);
    setBits(51 + 2 * s, 2, 16, s === 0 ? spec.polA : spec.polB);
    setBits(51 + 2 * s, 18, 4, s === 0 ? (spec.levelA ?? 0) : (spec.levelB ?? 0));
    setBits(51 + 2 * s, 22, 2, Math.round((s === 0 ? (spec.gemsA ?? 0) : (spec.gemsB ?? 0)) / 20));
  }
  setBits(57, 2, 3, spec.jit ?? 0);   // salt for the bots' start jitter (identical maps replay identical games)
  if (spec.groupsA) {
    for (let k = 0; k < 5; k++) setBits(54, 2 + 3 * k, 3, spec.groupsA[k] ?? 4);
    for (let k = 0; k < 5; k++) setBits(55, 2 + 3 * k, 3, spec.groupsB[k] ?? 4);
    setBits(54, 17, 1, 1);
  }
  const corridor = (y) => { for (let c = 0; c < 5; c++) if (y >= 6 + 8 * c && y <= 11 + 8 * c) return true; return false; };
  for (let y = 0; y < H; y++) for (let x = 0; x < W / 2; x++) {
    const open = (x <= 3 && y >= 22 && y <= 25) || ((x === 4 || x === 5) && y >= 6 && y <= 43) || (x >= 6 && corridor(y));
    if (x === 1 && y === 26) { b.altar(x, y); continue; }   // tribute altar beside the spawn (keeps carried gems at the target)
    if (open) continue;
    if (holes.has(`${x},${y}`)) b.hole(x, y); else b.wall(x, y);
  }
  if (spec.gemFarm !== false) for (let c = 0; c < 5; c++) { const lo = 6 + 8 * c; for (const x of [7, 9, 11]) for (const y of [lo, lo + 5]) b.camp('GEM_CLUSTER', x, y); }
  return b.encode();
}
if (process.argv[1] && process.argv[1].endsWith('mkduel.mjs')) {
  const specs = JSON.parse(fs.readFileSync(process.argv[2], 'utf8')); const outdir = process.argv[3]; fs.mkdirSync(outdir, { recursive: true });
  const E = openEngine(); let bad = 0;
  for (const s of specs) { const buf = makeDuel(E, s); const rc = validate(E, buf); if (rc) { bad++; console.log('invalid', s.name, rc); } fs.writeFileSync(path.join(outdir, s.name + '.map26a'), buf); }
  console.log(`${specs.length} maps, ${bad} invalid -> ${outdir}`);
}
