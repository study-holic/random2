// Engine 1.0.0 MapBuilder wrapper (the map editor's WASM from the cambc 3.0.1 visualiser cache, or $CAMBC_WASM).
import fs from 'node:fs'; import path from 'node:path'; import os from 'node:os'; import { WASI } from 'node:wasi';
export function findWasm() {
  if (process.env.CAMBC_WASM) return process.env.CAMBC_WASM;
  const roots = [path.join(os.homedir(), '.cache', 'cambc', 'assets'), path.join(process.env.LOCALAPPDATA || '', 'cambc', 'cache', 'assets'), 'data/top10/engine'];
  for (const r of roots) { if (!fs.existsSync(r)) continue;
    const stack = [r]; while (stack.length) { const d = stack.pop();
      for (const n of fs.readdirSync(d, { withFileTypes: true })) { const p = path.join(d, n.name);
        if (n.isDirectory()) stack.push(p); else if (/^v1_0_0-.*\.wasm$/.test(n.name)) return p; } } }
  throw new Error('engine 1.0.0 wasm not found');
}
export function openEngine() {
  const wasi = new WASI({ version: 'preview1', args: [], env: {}, returnOnExit: true });
  const inst = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(findWasm())), { wasi_snapshot_preview1: wasi.wasiImport });
  wasi.initialize(inst); const X = inst.exports; const out = X.Engine_Malloc(64);
  const view = () => new DataView(X.memory.buffer);
  const cstr = (p) => { const b = new Uint8Array(X.memory.buffer); let e = p; while (b[e]) e++; return Buffer.from(b.subarray(p, e)).toString(); };
  const check = (rc, what) => { if (rc !== 0) throw new Error(`${what}: engine error ${rc}: ${cstr(X.GetLastErrorMsg())}`); };
  return { X, out, view, check };
}
const SYM = { HORIZONTAL: 0, VERTICAL: 1, ROTATIONAL180: 2 };
export const CAMP = { WOLF: 0, ELK: 1, GEM_CLUSTER: 2, GOLD_CLUSTER: 3, MONOLITH: 4, GOLEM: 5 };
export class Builder {
  constructor(E, w, h, sym = 'HORIZONTAL') { this.E = E; E.check(E.X.MapBuilder_Create(w, h, SYM[sym], E.out), 'create'); this.h = E.view().getBigUint64(E.out, true); this.w = w; this.hgt = h; }
  wall(x, y) { this.E.check(this.E.X.MapBuilder_AddWall(this.h, x, y), `wall ${x},${y}`); }
  hole(x, y) { this.E.check(this.E.X.MapBuilder_AddHole(this.h, x, y), `hole ${x},${y}`); }
  altar(x, y) { this.E.check(this.E.X.MapBuilder_AddAltar(this.h, x, y), `altar ${x},${y}`); }
  camp(kind, x, y) { this.E.check(this.E.X.MapBuilder_AddCamp(this.h, CAMP[kind], x, y, this.E.out), `camp ${kind} ${x},${y}`); return this.E.view().getBigUint64(this.E.out, true); }
  champs(n) { this.E.check(this.E.X.MapBuilder_SetNumChampsPerTeam(this.h, n), 'champs'); }
  spawnA(x0, y0, x1, y1) { const v = this.E.view(), o = this.E.out; v.setInt32(o, x0, true); v.setInt32(o + 4, y0, true); v.setInt32(o + 8, x1, true); v.setInt32(o + 12, y1, true); this.E.check(this.E.X.MapBuilder_SetTeamASpawnArea(this.h, o), 'spawnA'); }
  encode() { const { X, out, view, check } = this.E; check(X.MapBuilder_VerifyIntegrity(this.h), 'verify'); check(X.MapBuilder_Encode(this.h, out, out + 4), 'encode');
    const p = view().getUint32(out, true), n = view().getUint32(out + 4, true); const b = Buffer.from(new Uint8Array(X.memory.buffer, p, n)); X.MapBuilder_Destroy(this.h); return b; }
}
export function validate(E, buf) { const { X, out } = E; const p = X.Engine_Malloc(buf.length); new Uint8Array(X.memory.buffer, p, buf.length).set(buf); const rc = X.Game_FromMapFileBytes(p, buf.length, out, out + 8); X.Engine_Free(p); return rc; }
