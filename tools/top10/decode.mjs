// Headless replay decoder for the v3 pilot. Adapted from ext/tarot/tools/replay/decode.mjs (Tarot Club, public repo):
// the cambc 2.2.5 visualiser bundle ships three engine WASMs (v0_0_1, v0_0_2, v0_0_3); the replay header byte 10 is the
// engine patch number, so the matching WASM replays the game deterministically and PanelChrome's snapshot decoders
// turn each state into objects. Replays are only ever passed to the official engine as bytes; nothing from a replay is executed.
// Engine assets live in data/v3/engine/cache2_2_5 (fetched from a local `cambc view` server, see notes/v3-data-scope.md).
import fs from 'node:fs';
import path from 'node:path';
import zlib from 'node:zlib';
import { pathToFileURL } from 'node:url';

export const CACHE = process.env.CAMBC_ENGINE_CACHE || 'C:/cambc-solo/data/top10/engine';
const pick = (re) => fs.readdirSync(CACHE).find((f) => re.test(f));

const engines = new Map();
export async function openEngine(bytes) {
  const v100 = bytes[8] === 1 && bytes[9] === 0 && bytes[10] === 0;
  const patch = v100 ? 100 : bytes[10];
  if (!v100 && (bytes.subarray(8, 10).some((b) => b !== 0) || ![1, 2, 3].includes(patch))) throw new Error(`unsupported engine header ${[...bytes.subarray(8, 11)]}`);
  if (engines.has(patch)) return engines.get(patch);
  const { WASI } = await import('node:wasi');
  const PC = await import(pathToFileURL(path.join(CACHE, pick(/^PanelChrome-.*\.js$/))).href);
  const wasi = new WASI({ version: 'preview1', args: [], env: {}, returnOnExit: true }); // no preopens: sandboxed
  const mod = await WebAssembly.compile(fs.readFileSync(path.join(CACHE, pick(new RegExp(v100 ? `^v1_0_0-.*\.wasm$` : `^v0_0_${patch}-.*\.wasm$`)))));
  const inst = await WebAssembly.instantiate(mod, { wasi_snapshot_preview1: wasi.wasiImport });
  wasi.initialize(inst);
  const X = inst.exports;
  const view = () => new DataView(X.memory.buffer);
  const outBuf = X.Engine_Malloc(64), args = X.Engine_Malloc(128);
  const cstr = (p) => { const b = new Uint8Array(X.memory.buffer); let e = p; while (b[e]) e++; return Buffer.from(b.subarray(p, e)).toString(); };
  const check = (rc) => { if (rc !== 0) throw new Error(`engine error ${rc}: ${cstr(X.GetLastErrorMsg())}`); };
  const vals = Object.values(PC);
  const spectator = vals.find((v) => v && typeof v === 'object' && v.team === 2 && v.champion === 0xffffffff);
  const analytics = vals.find((v) => typeof v === 'function' && String(v).includes('Invalid analytics snapshot count'));
  const headerCandidates = vals.filter((v) => typeof v === 'function' && v.length === 2 && /^function \w+\(e,t\)\{(?:return \w+\(e,t,\w+,\d+\)|let n=e\.getUint32\(t\+4,!0\);if\(!\w+\(n\)\)throw Error\(`Unsupported game snapshot ABI`\);return \w+\(e,t,n,\d+\))\}$/.test(String(v)));
  if (!spectator || !analytics || !headerCandidates.length) throw new Error('visualiser decoder layout changed');
  const E = { X, view, outBuf, args, check, spectator, analytics, headerCandidates, patch };
  engines.set(patch, E);
  return E;
}

export function openReplay(E, bytes) {
  const { X, view, outBuf, args, check, spectator, analytics } = E;
  const p = X.Engine_Malloc(bytes.length);
  new Uint8Array(X.memory.buffer, p, bytes.length).set(bytes);
  check(X.Game_FromReplayFileBytes(p, bytes.length, outBuf));
  X.Engine_Free(p);
  const handle = view().getBigUint64(outBuf, true);
  const snap = X.Engine_Malloc(X.Game_Visualiser_SnapshotSize());
  check(X.Game_Runner_SetCheckpointsEnabled(handle, 1));
  check(X.Game_Visualiser_Timeline_GetMain(handle, outBuf));
  const main = view().getBigUint64(outBuf, true);
  check(X.Game_Visualiser_Timeline_Info(handle, main, args, args + 8, args + 16));
  const size = view().getUint32(args, true);
  const persp = () => { const t = args + 48, v = view(); v.setUint32(t, spectator.team, true); v.setUint32(t + 4, spectator.champion, true); return t; };
  const decodeHeader = () => {
    for (const h of E.headerCandidates) {
      try { const hdr = h(view(), snap); if (typeof hdr?.round === 'number' && Array.isArray(hdr?.drafts)) { E.headerCandidates = [h]; return hdr; } } catch {}
    }
    throw new Error('no header decoder matched the snapshot');
  };
  return {
    size,
    seek(pos) { check(X.Game_Visualiser_Timeline_Seek(handle, main, pos, 0)); },
    state() { check(X.Game_Visualiser_ReadState(handle, persp(), snap)); return analytics(view(), snap, decodeHeader()); },
    close() { X.Engine_Free(snap); X.Game_Destroy(handle); },
  };
}

// Minimal zip reader (platform bundles are single-entry zips holding replay.repl26a); no external unzip needed.
function unzipEntry(buf, wanted) {
  let eocd = -1;
  for (let i = buf.length - 22; i >= Math.max(0, buf.length - 70000); i--) if (buf.readUInt32LE(i) === 0x06054b50) { eocd = i; break; }
  if (eocd < 0) throw new Error('zip: no end record');
  let off = buf.readUInt32LE(eocd + 16);
  for (let n = buf.readUInt16LE(eocd + 10); n > 0; n--) {
    if (buf.readUInt32LE(off) !== 0x02014b50) throw new Error('zip: bad central entry');
    const method = buf.readUInt16LE(off + 10), csize = buf.readUInt32LE(off + 20), usize = buf.readUInt32LE(off + 24);
    const nl = buf.readUInt16LE(off + 28), el = buf.readUInt16LE(off + 30), cl = buf.readUInt16LE(off + 32), lho = buf.readUInt32LE(off + 42);
    const name = buf.toString('utf8', off + 46, off + 46 + nl);
    if (name === wanted) {
      const ds = lho + 30 + buf.readUInt16LE(lho + 26) + buf.readUInt16LE(lho + 28);
      const raw = buf.subarray(ds, ds + csize);
      const out = method === 0 ? raw : method === 8 ? zlib.inflateRawSync(raw, { maxOutputLength: 1 << 28 }) : null;
      if (!out || out.length !== usize) throw new Error('zip: unsupported or corrupt entry');
      return out;
    }
    off += 46 + nl + el + cl;
  }
  throw new Error('zip: entry not found ' + wanted);
}

export function replayBytes(file) {
  const b = fs.readFileSync(file);
  return b.subarray(0, 2).toString('latin1') === 'PK' ? unzipEntry(b, 'replay.repl26a') : b;
}
