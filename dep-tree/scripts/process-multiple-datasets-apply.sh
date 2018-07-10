#!/bin/bash

source lib-mwe.sh

set -e


#
# this script is suppsoed to be merged with process-multiple-datasets (as an option in the other one)
#
#
#



progName=$(basename "$BASH_SOURCE")

testFile="test.blind.cupt"
#force=0
trainTestOpts=
#evaluate=""
#oldVersion=""
configFileName="config.conf"

function usage {
  echo
  echo "Usage: $progName [options] <input data dir> <input dir> <output dir>"
  echo
  echo "  prints tasks to execute."
  echo 
  echo " <input data dir>: contains subdirectories by language, e.g."
  echo "    <input data dir>/FR/train.cupt"
  echo
  echo " <input dir> contains languages, each contains:"
  echo "      - dir <input dir>/<lang>/model"
  echo "      - file <input dir>/<lang>/config.conf"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -t <test file> use <input data dir>/<lang>/<test file> as test file;"
  echo "       default: '$testFile'"
  #  echo "    -f force recomputing features even if already there"
  echo "    -o <train-test.sh options> e.g. -o '-x path-to-xcrf'"
#  echo "    -e evaluate with official script"
  echo "    -z use option -t with train-test-class-method.sh, i.e. use a temporary"
  echo "       directory for every experiment and clean up."
#  echo "    -1 data in old version of shared task (conllu+parsemetsv)"
  echo "    -h this help"
  echo
}



function containsHeadColumn {
    f="$1"
}



OPTIND=1
while getopts 'ht:o:z' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
#	"f" ) force=1;;
#	"T" ) trainFile="$OPTARG";;
	"t" ) testFile="$OPTARG";;
	"o" ) trainTestOpts="$OPTARG";;
#	"e" ) evaluate="yep";;
#	"1" ) oldVersion="yep";;
	"z" ) tempOpt="-t";;
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
inputModelDir="$2"
workDir="$3"

[ -d "$workDir" ] || mkdir "$workDir"
#


n=$(ls "$inputDir"/* | wc -l)
if [ ! -d "$inputDir" ] || [ $n -eq 0 ]; then
    echo "Error: no input dir '$inputDir' found or empty dir" 1>&2
    exit 1
fi

for langDir in "$inputDir"/*; do
    if [ -f "$langDir/$testFile" ]; then

	lang=$(basename "$langDir")

	configFile="$inputModelDir/$lang/$configFileName"
	if [ -d "$inputModelDir/$lang" ] && [ -d "$inputModelDir/$lang/model" ] && [ -s "$configFile" ]; then

	    [ -d "$workDir/$lang" ] ||  mkdir "$workDir/$lang"

	    outDir="$workDir/$lang"

	    params=" $tempOpt -o \"$trainTestOpts\""
	    command="train-test-class-method.sh $params -a  \"$langDir/$testFile\" \"$configFile\" \"$inputModelDir/$lang/model\" \"$outDir\" >$outDir/out 2>$outDir/err"
	    echo "$command"
	else
	    echo "Warning: skipping directory '$langDir', directory $inputModelDir/$lang does not contain model and/or config file." 1>&2
	fi
	
    else
	echo "Warning: ignoring dir '$langDir' (did not find both $trainFile and $testFile)" 1>&2
    fi
done


rm -f "$configsFile"
