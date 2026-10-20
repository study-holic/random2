// Thin wrapper over the official engine WASM's MapBuilder_* exports (the map editor's API), used to write arena maps.
// The WASM comes from the cambc 2.2.5 visualiser (fetched from a local `cambc view` into tools/micro/.cache).
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const CACHE = path.join(HERE, '.cache');

export async function openBuilderEngine(patch = 3) {
  const { WASI } = await import('node:wasi');
  const f = fs.readdirSync(CACHE).find((n) => new RegExp(`^v0_0_${patch}-.*\.wasm$`).test(n));
  const wasi = new WASI({ version: 'preview1', args: [], env: {}, returnOnExit: true });
  const mod = await WebAssembly.compile(fs.readFileSync(path.join(CACHE, f)));
  const inst = await WebAssembly.instantiate(mod, { wasi_snapshot_preview1: wasi.wasiImport });
  wasi.initialize(inst);
  const X = inst.exports;
  const out = X.Engine_Malloc(64);
  const view = () => new DataView(X.memory.buffer);
  const cstr = (p) => { const b = new Uint8Array(X.memory.buffer); let e = p; while (b[e]) e++; return Buffer.from(b.subarray(p, e)).toString(); };
  const check = (rc, what) => { if (rc !== 0) throw new Error(`${what}: engine error ${rc}: ${cstr(X.GetLastErrorMsg())}`); };
  return { X, out, view, check };
}

const SYM = { HORIZONTAL: 0, VERTICAL: 1, ROTATIONAL180: 2 };
const CAMP = { WOLF: 0, ELK: 1, GEM_CLUSTER: 2, GOLD_CLUSTER: 3, MONOLITH: 4 };

export class Builder {
  constructor(E, w, h, sym = 'ROTATIONAL180') {
    this.E = E;
    E.check(E.X.MapBuilder_Create(w, h, SYM[sym], E.out), 'create');
    this.h = E.view().getBigUint64(E.out, true);
    this.w = w; this.hgt = h;
  }
  wall(x, y) { this.E.check(this.E.X.MapBuilder_AddWall(this.h, x, y), `wall ${x},${y}`); }
  hole(x, y) { this.E.check(this.E.X.MapBuilder_AddHole(this.h, x, y), `hole ${x},${y}`); }
  bush(x, y) { this.E.check(this.E.X.MapBuilder_AddBush(this.h, x, y), `bush ${x},${y}`); }
  altar(x, y) { this.E.check(this.E.X.MapBuilder_AddAltar(this.h, x, y), `altar ${x},${y}`); }
  camp(kind, x, y) { this.E.check(this.E.X.MapBuilder_AddCamp(this.h, CAMP[kind], x, y, this.E.out), `camp ${kind} ${x},${y}`); }
  champs(n) { this.E.check(this.E.X.MapBuilder_SetNumChampsPerTeam(this.h, n), 'champs'); }
  spawnA(minX, minY, maxX, maxY) {
    const v = this.E.view(); const o = this.E.out;
    v.setInt32(o, minX, true); v.setInt32(o + 4, minY, true); v.setInt32(o + 8, maxX, true); v.setInt32(o + 12, maxY, true);
    this.E.check(this.E.X.MapBuilder_SetTeamASpawnArea(this.h, o), 'spawnA');
  }
  encode() {
    const { X, out, view, check } = this.E;
    check(X.MapBuilder_VerifyIntegrity(this.h), 'verify');
    check(X.MapBuilder_Encode(this.h, out, out + 4), 'encode');
    const p = view().getUint32(out, true), n = view().getUint32(out + 4, true);
    return Buffer.from(new Uint8Array(X.memory.buffer, p, n));
  }
}
