#pragma once
#include <cambc.h>

void play_draft();

// EXP-073: the draft-time centre-map decision (also stored in draft memory).
struct CentreDraft { bool yes = false; cambc::vec2 altar{0, 0}; };
CentreDraft centre_draft_decision();
