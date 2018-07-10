#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

withCateg=
runTasks=

function usage {
  echo
  echo "Usage: $progName [options] <shared task dir> <config-experiment pairs file> <MWE|Tok> <output dir>"
  echo
  echo " Reads pairs  <config dir> <expe dir> from STDIN, one pair by line separated by tab."
  echo " an <expe dir> contains lang directories: <expe dir>/<lang>/<config id>/eval.out"
  echo "  Prints tasks to <ouput dir>.models/apply-tasks.list"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -c with categories (i.e. exclude 'none'); default: without."
  echo "    -r start tasks in parallel at the end instead of only printing to a file"
  echo
}







OPTIND=1
while getopts 'crh' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"c" ) withCateg="yep";;
	"r" ) runTasks="yep";;
#	"l" ) trainFile="$OPTARG";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 4 ]; then
    echo "Error: expecting 4 args." 1>&2
    printHelp=1
fi
if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi

inputDir="$1"
pairsFile="$2"
level="$3"
outputDir="$4"

rm -rf "$outputDir"
mkdir "$outputDir"
rm -rf "$outputDir.models"
mkdir "$outputDir.models"

cat "$pairsFile" | collect-results-multiple-experiments.sh "$outputDir.models"/results.tsv

params=""
if [ ! -z "$withCateg" ]; then
    params="$params -c"
fi
command="cat \"$pairsFile\" | select-best-by-lang-from-results.sh $params \"$level\" \"$outputDir.models/results.tsv\" \"$outputDir.models\""
eval "$command"

command="process-multiple-datasets-apply.sh -z -o \"-j '-Xms6g -Xmx6g'\" \"$inputDir\" \"$outputDir.models\" \"$outputDir\" >\"$outputDir.models/apply-tasks.list\""
eval "$command"

echo "Models dir = $outputDir.models, tasks list = $outputDir.models/apply-tasks.list" 1>&2

if [ ! -z "$runTasks" ]; then
    echo "Starting tasks in parallel..." 1>&2
    cat "$outputDir.models/apply-tasks.list" | while read task; do
	eval "$task &"
    done
fi
