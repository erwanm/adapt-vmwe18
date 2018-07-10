#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

trainFile="train.cupt"
testFile="dev.cupt"

function usage {
  echo
  echo "Usage: $progName [options] <input data dir> <max size in sentences> <output dir>"
  echo
  echo "  copies the whole dir but with reduced train file."
  echo 
  echo " <input data dir>: contains subdirectories by language, e.g."
  echo "    <input data dir>/FR/train.cupt"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -T <train file> use <input data dir>/<lang>/<train file> as train file;"
  echo "       default: '$trainFile'"
  echo "    -t <test file> use <input data dir>/<lang>/<test file> as test file;"
  echo "       default: '$testFile'"
  #  echo "    -f force recomputing features even if already there"
  echo
}





OPTIND=1
while getopts 'hT:t:' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
#	"f" ) force=1;;
	"T" ) trainFile="$OPTARG";;
	"t" ) testFile="$OPTARG";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 3 ]; then
    echo "Error: expecting 3 args." 1>&2
    printHelp=1
fi
if [ -z "$trainFile" ] &&  [ -z "$testFile" ]; then
    echo "$progName error: one of -l and -a must be supplied." 1>&2
    printHelp=1
#elif  [ ! -z "$trainFile" ] &&  [ ! -z "$testFile" ]; then
#    echo "$progName error: cannot use both -l and -a." 1>&2
#    printHelp=1
fi

if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi


inputDir="$1"
newSize="$2"
workDir="$3"

rm -rf "$workDir"
mkdir "$workDir"

n=$(ls "$inputDir"/* | wc -l)
if [ ! -d "$inputDir" ] || [ $n -eq 0 ]; then
    echo "Error: no input dir '$inputDir' found or empty dir" 1>&2
    exit 1
fi

for langDir in "$inputDir"/*; do
    if [ -f "$langDir/$trainFile" ] && [ -f "$langDir/$testFile" ]; then

	lang=$(basename "$langDir")
	[ -d "$workDir/$lang" ] ||  mkdir "$workDir/$lang"

	echo "Processing $lang..."
	if [ -s "$langDir/$testFile" ]; then
	    split-conllu-sentences.pl -c "$newSize" -b  "$workDir/$lang/tmp." 1 "$langDir/$trainFile"
	    mv "$workDir/$lang/tmp.0" "$workDir/$lang/$trainFile"
	    cp "$langDir/$testFile"  "$workDir/$lang/$testFile"
	else
	    echo "Warning: no test file here, using 20% of the train file for test" 1>&2
	    split-conllu-sentences.pl -p -b  "$workDir/$lang/tmp." 0.8 "$langDir/$trainFile"
	    # for special first line
	    head -n 1 "$langDir/$trainFile" >"$workDir/$lang/$testFile"
	    cat "$workDir/$lang/tmp.2" >>"$workDir/$lang/$testFile"
	    split-conllu-sentences.pl -c "$newSize" -b  "$workDir/$lang/tmp2." 1 "$workDir/$lang/tmp.1"
	    mv "$workDir/$lang/tmp2.0" "$workDir/$lang/$trainFile"
	    rm -f "$workDir/$lang"/tmp.?
	fi
	
    else
	echo "Warning: ignoring dir '$langDir' (did not find both $trainFile and $testFile)" 1>&2
    fi
done


rm -f "$configsFile"
