#!/usr/bin/env bash

function die {
    echo "$1" >&2
    exit 1
}
[[ ! -r $1 ]] && die "cannot read log file $1"

logFile=$1

replacements=(
'^malloc_count ### exiting.* peak: \([0-9.,]\+\),.*' 'mem'
'^Wall Time:\s\+\([0-9]\+\)' 'time'
'^Max Memory:\s\+\([0-9]\+\)' 'mrs'
)
replacements_it=0
while [[ $replacements_it -lt ${#replacements[@]} ]]; do
    subst_it=0
    ((subst_it=replacements_it+1))
    pattern=${replacements[$replacements_it]}
    replace=${replacements[$subst_it]}
    value=$(grep "$pattern" "$logFile" | sed "s@$pattern@\1@")
    [[ -n $value ]] && echo -n "$replace=$value "
    replacements_it=$((replacements_it+2))
done

replacements=(
'^Text runs:\s\+\([0-9]\+\)' 'textruns'
'^Text length:\s\+\([0-9]\+\)' 'textlength'
'^Scan max:\s\+\([0-9]\+\)' 'maxscan'
'^Scan after:\s\+\([0-9]\+\)' 'maxafter'
'^Runs added:\s\+\([0-9]\+\)' 'rowsadded'
'^Time build:\s\+\([0-9]\+\)' 'timebuild'
'^Time run:\s\+\([0-9]\+\)' 'timerun'
)

replacements_it=0
while [[ $replacements_it -lt ${#replacements[@]} ]]; do
    subst_it=0
    ((subst_it=replacements_it+1))
    pattern=${replacements[$replacements_it]}
    replace=${replacements[$subst_it]}
    value=$(grep "$pattern" "$logFile" | sed "s@$pattern@\1@")
    [[ -n $value ]] && echo -n "$replace=$value "
    replacements_it=$((replacements_it+2))
done
echo ""

