// Instantiate each bundled engine (v0_0_1..3) with a synthetic header to confirm the WASM and PanelChrome decoder load. No replay needed.
import { openEngine } from './decode.mjs';
for (const p of [1, 2, 3]) {
  const h = new Uint8Array(16); h[8] = 0; h[9] = 0; h[10] = p;
  try { const E = await openEngine(h); console.log('engine v0_0_' + p, 'loaded; snapshotSize', E.X.Game_Visualiser_SnapshotSize()); } catch (e) { console.log('engine v0_0_' + p, 'FAIL', e.message); }
}
