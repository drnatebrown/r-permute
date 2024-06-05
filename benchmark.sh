#!/usr/bin/env bash

function die {
	echo "$1" >&2
	exit 1
}

PFP_BULIDDIR=/home/ext-nbrown/repos/pfp-thresholds/build
R_PERMUTE_BUILDDIR=/home/ext-nbrown/repos/r-permute/build
R_PERMUTE_DIR=/home/ext-nbrown/repos/r-permute/
DATASET_DIR=/home/ext-nbrown/data/chr19
LOG_DIR=$HOME/log/

datasets=(128)
split_params=(2 4 8 16 32 64 128 256 512 1024)

rlbwt_prg=$PFP_BULIDDIR/pfp_thresholds
rp_build=$R_PERMUTE_BUILDDIR/test/src/build_constructor
rp_run=$R_PERMUTE_BUILDDIR/test/src/run_constructor
readlog_prg=$R_PERMUTE_DIR/readlog.sh

for filename in ${datasets[@]}; do
    dataset=$DATASET_DIR/$filename.fa
    test -e $dataset || die "file $dataset not found"

    basestats="RESULT file=${filename}"

    if [[ ! -e $dataset.bwt.heads ]]; then 
		stats="$basestats type=rlebwt "
		logFile=$LOG_DIR/$filename.rlbwt.log
		set -x
		/usr/bin/time --format="Wall Time: %e\nMax Memory: %M" $rlbwt_prg $dataset -r -f > "$logFile" 2>&1
		set +x
		echo -n "$stats"
		echo -n "headssize=$(stat --format="%s" $dataset.bwt.heads) "
		echo -n "lensize=$(stat --format="%s" $dataset.bwt.len) "
        set +e
		$readlog_prg $logFile
        set -e
	fi

    logFile=$LOG_DIR/$filename.rp_build.log
    stats="$basestats type=build "
    set -x
    /usr/bin/time --format="Wall Time: %e\nMax Memory: %M" $rp_build "$dataset" > "$logFile" 2>&1
    set +x
    echo -n "$stats"
    echo -n "disksize=$(stat --format="%s" $dataset.d_construct) "
    $readlog_prg $logFile

    for d in ${split_params[@]}; do
        logFile=$LOG_DIR/$filename.rp_run.log
        stats="$basestats type=deterministic d=${d} "
        set -x
        /usr/bin/time --format="Wall Time: %e\nMax Memory: %M" $rp_run -d "$d" "$dataset" > "$logFile" 2>&1
        set +x
        echo -n "$stats"
        cp ${dataset}.d_col ${dataset}.${d}_col 
        set +e
        $readlog_prg $logFile
        set -e
    done
done
echo 'Finished'
