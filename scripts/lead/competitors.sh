#!/bin/bash
# competitors.sh: shallow-clone the public competitor repos (project/competitor-repos.md), list their bots,
# copy every C++ bot into the 3.0.5 workspace as comp-<owner>-<bot>, build it in release mode, and log the outcome.
set -u
LIST=/home/claude/src/random2-main/project/competitor-repos.md
D=/home/claude/competitors
WS=/home/claude/ws305
C=/home/claude/v305/bin/cambc
OUT=/home/claude/runs/competitors.tsv
export SSL_CERT_FILE=/root/.ccr/ca-bundle.crt CAMBC_NO_UPDATE_CHECK=1
mkdir -p $D
printf "repo\tlast_commit\tbot\tlanguage\tbuild\n" > $OUT
for url in $(grep -o -E "github\.com/[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+" $LIST | sort -u); do
  owner=$(echo $url | cut -d/ -f2); repo=$(echo $url | cut -d/ -f3); dir=$D/${owner}__${repo}
  [ -d $dir/.git ] || timeout 180 git clone -q --depth 1 https://$url $dir > /dev/null 2>&1
  if [ ! -d $dir/.git ]; then printf "%s\t-\t-\t-\tclone failed\n" "$owner/$repo" >> $OUT; continue; fi
  last=$(git -C $dir log -1 --format=%cI)
  found=0
  for b in $(find $dir -maxdepth 4 -type d -name src -not -path "*/sdk/*" -not -path "*/node_modules/*" 2>/dev/null); do
    bot=$(dirname $b); name=$(basename $bot)
    if ls $b/*.cpp > /dev/null 2>&1; then lang=cpp; elif ls $b/*.py > /dev/null 2>&1 || ls $bot/*.py > /dev/null 2>&1; then lang=python; elif [ -f $bot/Cargo.toml ]; then lang=rust; else continue; fi
    found=1
    if [ $lang = cpp ]; then
      tag=$(echo "comp-${owner}-${name}" | tr 'A-Z' 'a-z' | tr -c 'a-z0-9-\n' '-' | cut -c1-48)
      rm -rf $WS/bots/$tag; mkdir -p $WS/bots/$tag; cp -r $bot/src $WS/bots/$tag/
      if (cd $WS && timeout 600 $C build $tag --release > /tmp/cb-$tag.log 2>&1) && grep -q "^built" /tmp/cb-$tag.log; then res="OK $tag"; else res="FAIL $(grep -m1 -E 'error' /tmp/cb-$tag.log | cut -c1-90)"; fi
    else res="skipped ($lang)"; fi
    printf "%s\t%s\t%s\t%s\t%s\n" "$owner/$repo" "$last" "$name" "$lang" "$res" >> $OUT
  done
  [ $found = 0 ] && printf "%s\t%s\t-\t-\tno bot found\n" "$owner/$repo" "$last" >> $OUT
done
echo complete >> $OUT
