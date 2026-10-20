// s3_contest.h (v5 S3): contest plays (s3_contest.cpp): P_S3_INTERDICT, P_S3_MONO, P_S3_ESCORT.
#pragma once

class Bot;
struct TurnView;

namespace S3C {

void init(Bot& b);
bool pre(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies);
bool buy(Bot& b);   // buys an altar lock when possible; never skips the default purchase

}  // namespace S3C
