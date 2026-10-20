// Arena maps for the w14 duel lab, written with the engine's own MapBuilder (the map editor's API).
// Base: maps/pilgrimage.map26a (50x65, x-mirror symmetry, 5 champions); every tile is cleared, then:
//   team A spawn x 0..3, y 22..25 (team B mirrored at x 46..49); connector x 4..5, y 6..43; five corridors y 6+8c..11+8c,
//   x 6..43, 2-tile walls between;
//   the duel spec as holes in the wall block: row 50+2s = five 4-bit picks of side s; row 51+2s = 16 policy bits + 4-bit level.
// usage (module): import { makeArena } ...; makeArena(E, {picksA, picksB, polA, polB, levelA, levelB}) -> Buffer
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { openBuilderEngine } from './mapbuilder.mjs';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const BASE = path.join(HERE, '..', '..', 'maps-backup-1019', 'pilgrimage.map26a');   // pinned (maps/ is replaced over time)
export const PICK = { ryth: 0, vesper: 1, vendetta: 2, ci: 3, stellar: 4, prism: 5, marcella: 6, willow: 7, ren: 8, varg: 9, tavi: 10 };

export function makeArena(E, spec, opts = {}) {
  const { X, out, view, check } = E;
  const bytes = fs.readFileSync(BASE);
  const p = X.Engine_Malloc(bytes.length);
  new Uint8Array(X.memory.buffer, p, bytes.length).set(bytes);
  check(X.MapBuilder_FromFileBytes(p, bytes.length, out), 'from');
  X.Engine_Free(p);
  const h = view().getBigUint64(out, true);
  const W = 50, H = 65;
  for (let y = 0; y < H; y++) for (let x = 0; x < W; x++) for (const fn of ['MapBuilder_RemoveWall', 'MapBuilder_RemoveBush', 'MapBuilder_RemoveHole', 'MapBuilder_RemoveAltar']) X[fn](h, x, y);
  for (;;) { check(X.MapBuilder_GetNumCamps(h, out), 'nc'); if (!view().getUint32(out, true)) break; check(X.MapBuilder_GetCampHandle(h, 0, out), 'ch'); check(X.MapBuilder_RemoveCamp(h, view().getBigUint64(out, true)), 'rm'); }
  check(X.MapBuilder_SetNumChampsPerTeam(h, opts.champs ?? 5), 'champs');
  const v = view(); v.setInt32(out, 0, true); v.setInt32(out + 4, 22, true); v.setInt32(out + 8, 3, true); v.setInt32(out + 12, 25, true);
  check(X.MapBuilder_SetTeamASpawnArea(h, out), 'spawn');
  const corridor = (y) => { for (let c = 0; c < 5; c++) if (y >= 6 + 8 * c && y <= 11 + 8 * c) return true; return false; };
  const holes = new Set();
  const setBits = (y, x0, n, val) => { for (let i = 0; i < n; i++) if ((val >> i) & 1) { holes.add(`${x0 + i},${y}`); holes.add(`${W - 1 - (x0 + i)},${y}`); } };
  for (let s = 0; s < 2; s++) {
    const picks = s === 0 ? spec.picksA : spec.picksB;
    for (let k = 0; k < 5; k++) setBits(50 + 2 * s, 2 + 4 * k, 4, k < picks.length ? picks[k] : 15);
    setBits(51 + 2 * s, 2, 16, s === 0 ? spec.polA : spec.polB);
    setBits(51 + 2 * s, 18, 4, s === 0 ? (spec.levelA ?? 0) : (spec.levelB ?? 0));
  }
  if (spec.groupsA) {   // team-fight mode: side s's pick slot k fights in corridor groups_s[k] (row 54 + s); flag row 54 x 17
    for (let k = 0; k < 5; k++) setBits(54, 2 + 3 * k, 3, spec.groupsA[k] ?? 4);
    for (let k = 0; k < 5; k++) setBits(55, 2 + 3 * k, 3, spec.groupsB[k] ?? 4);
    setBits(54, 17, 1, 1);
  }
  for (let y = 0; y < H; y++) for (let x = 0; x < W; x++) {
    const spawnBlock = (x <= 3 || x >= W - 4) && y >= 22 && y <= 25;   // the engine accepts 4x4/5x5 spawn areas, not 2x38
    const connector = ((x >= 4 && x <= 5) || (x >= W - 6 && x <= W - 5)) && y >= 6 && y <= 43;
    const open = spawnBlock || connector || (x >= 6 && x <= W - 7 && corridor(y));
    if (open) continue;
    if (holes.has(`${x},${y}`)) X.MapBuilder_AddHole(h, x, y); else X.MapBuilder_AddWall(h, x, y);
  }
  if (spec.gold) {   // item-level arenas: two 2-cluster gold groups per corridor end (x 10-11 on the top and bottom rows);
    // camps are stored on the canonical half and mirrored by the engine (adding x 38-39 is a no-op)
    for (let c = 0; c < 5; c++) {
      const lo = 6 + 8 * c;
      for (const [x, y] of [[10, lo], [11, lo], [10, lo + 5], [11, lo + 5]]) check(X.MapBuilder_AddCamp(h, 3, x, y, out), `gold ${x},${y}`);
    }
  }
  check(X.MapBuilder_VerifyIntegrity(h), 'verify');
  check(X.MapBuilder_Encode(h, out, out + 4), 'encode');
  const q = view().getUint32(out, true), n = view().getUint32(out + 4, true);
  const buf = Buffer.from(new Uint8Array(X.memory.buffer, q, n));
  X.MapBuilder_Destroy(h);
  return buf;
}

export function validate(E, buf) {
  const { X, out } = E;
  const p = X.Engine_Malloc(buf.length);
  new Uint8Array(X.memory.buffer, p, buf.length).set(buf);
  const rc = X.Game_FromMapFileBytes(p, buf.length, out);
  X.Engine_Free(p);
  return rc;
}

if (process.argv[1] && process.argv[1].endsWith('mkarena.mjs')) {
  const E = await openBuilderEngine(3);
  const P = PICK;
  const buf = makeArena(E, { picksA: [P.stellar, P.ryth, P.varg, P.tavi, P.ren], picksB: [P.ryth, P.varg, P.stellar, P.ren, P.tavi], polA: 0, polB: 0 });
  console.log('bytes', buf.length, 'game rc', validate(E, buf));
  fs.writeFileSync(path.join(HERE, '..', '..', 'maps-lab', 'smoke.map26a'), buf);
}
