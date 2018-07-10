#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

trainFile=
testFile=
#force=0
xcrfDir="xcrf-src-1.0"
javaOpts="-Xms2g -Xmx2g"


function usage {
  echo
  echo "Usage: $progName [options] <output tsv>"
  echo
  echo " Reads pairs  <config dir> <expe dir> from STDIN, one pair by line separated by tab."
  echo " an <expe dir> contains lang directories: <expe dir>/<lang>/<config id>/eval.out"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo
}







OPTIND=1
while getopts 'h' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
#	"f" ) force=1;;
#	"l" ) trainFile="$OPTARG";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 1 ]; then
    echo "Error: expecting 1 args." 1>&2
    printHelp=1
fi
if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi




output="$1"


header=$(mktemp --tmpdir "$progName.XXXXXXXXXX")
content=$(mktemp --tmpdir "$progName.XXXXXXXXXX")
while read pair; do
    configDir=$(echo "$pair" | cut -f 1)
    expeDir=$(echo "$pair" | cut -f 2)
    expeId=$(basename "$expeDir")
    f=$(mktemp --tmpdir "$progName.XXXXXXXXXX")
    ls "$expeDir"/*/*/eval.out | collect-results.pl "$configDir" "$f" 1>&2
    col1=$(mktemp --tmpdir "$progName.XXXXXXXXXX")
    echo "expeId" >"$col1"
    n=$(cat "$f" | wc -l)
    for i in $(seq 2 $n); do
	echo "$expeId"
    done >>"$col1"
    paste "$col1" "$f"  | head -n 1 >$header
    paste "$col1" "$f"  | tail -n +2 >>$content
    rm -f $f $col1
done 

cat $header >"$output"
cat "$content" >>"$output"

rm -f $header $content
