// s3_contest.h (v5 S3): contest plays (pack siege, interdiction, lock). Off unless map_contest.cpp enables it.
#pragma once
#include <cambc.h>
#include <vector>

class Bot;
struct TurnView;

namespace S3C {
void enable();
void init(Bot& b);
bool pre(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies);
bool buy(Bot& b);
}  // namespace S3C
