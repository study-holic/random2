// v2 route table (from w3; collision routing removed 23:14: held-out collision 9 -> 8 failed R4, and the ladder only has collision_v2): true sends the map to B (b-v418d), false to A (v1). Old and v2 ids route alike.
// Final 22:54 BST: B only where cmpcells admitted it over v1-base (3 salts each, panel of six):
// pilgrimage dW +3.33 dStrong +2.00 dGem +2609; collision dW +2.33 dStrong +0.67 dGem +1301.
// Not admitted, so A: eight (+1.00), lanes (+0.67, and B hit the fuel limit 146 times), interlock (-0.33).
#pragma once
inline bool w3_route_b(va::mapid::MapId id) {
    using M = va::mapid::MapId;
    switch (id) {
        case M::pilgrimage:
            return true;
        default:
            return false;
    }
}
