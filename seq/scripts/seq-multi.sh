#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

trainFile="train.cupt"
testFile="dev.cupt"
#force=0
#trainTestOpts=
evaluate=""
oldVersion=""

function usage {
  echo
  echo "Usage: $progName [options] <input data dir> <configs file or dir> <output dir>"
  echo
  echo "  Batch process the training/testing process for all the datasets in "
  echo "  <input data dir> with all the config files in <configs file or dir>."
  echo "  The tasks to execute are printed to STDOUT."
  echo 
  echo " <input data dir>: contains subdirectories by language, e.g."
  echo "    <input data dir>/FR/train.cupt"
  echo
  echo " <configs file or dir> is either:"
  echo "   - a file containing a list of config files, one by line"
  echo "   - a directory containing conf files (*.conf)"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -T <train file> use <input data dir>/<lang>/<train file> as train file;"
  echo "       default: '$trainFile'"
  echo "    -t <test file> use <input data dir>/<lang>/<test file> as test file;"
  echo "       default: '$testFile'"
  #  echo "    -f force recomputing features even if already there"
#  echo "    -o <train-test.sh options> e.g. -o '-x path-to-xcrf'"
  echo "    -e evaluate with official script"
#  echo "    -z use option -t with train-test-class-method.sh, i.e. use a temporary"
#  echo "       directory for every experiment and clean up."
  echo "    -1 data in old version of shared task (conllu+parsemetsv)"
  echo
}





OPTIND=1
while getopts 'hT:t:e' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
#	"f" ) force=1;;
	"T" ) trainFile="$OPTARG";;
	"t" ) testFile="$OPTARG";;
#	"o" ) trainTestOpts="$OPTARG";;
	"e" ) evaluate="yep";;
	"1" ) oldVersion="yep";;
#	"z" ) tempOpt="-t";;
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
configsFileOrDir="$2"
workDir="$3"

[ -d "$workDir" ] || mkdir "$workDir"
#

configsFile=$(mktemp --tmpdir "$progName.configs.XXXXXXXXXX")
if [ -d "$configsFileOrDir" ]; then
    ls "$configsFileOrDir"/*.conf >"$configsFile"
else
    cat "$configsFileOrDir" >"$configsFile"
fi
checkFile "$configsFile"

n=$(ls "$inputDir"/* | wc -l)
if [ ! -d "$inputDir" ] || [ $n -eq 0 ]; then
    echo "Error: no input dir '$inputDir' found or empty dir" 1>&2
    exit 1
fi

for langDir in "$inputDir"/*; do
    if [ ! -z "$oldVersion" ]; then
	if [ -f "$langDir/train.conllu" ] && [ -f "$langDir/train.parsemetsv" ] && [ -f "$langDir/test.conllu" ]  && [ -f "$langDir/test.parsemetsv" ] ; then
	    x=$(mktemp --tmpdir "$progName.oldversion.XXXXXXXXXX")
	    cut -f 4 "$langDir/train.parsemetsv" >$x
	    paste "$langDir/train.conllu" $x > "$langDir/$trainFile"
	    cut -f 4 "$langDir/test.parsemetsv" >$x
	    paste "$langDir/test.conllu" $x > "$langDir/$testFile"
	    rm -f "$x"
	else
	    echo "Warning: old version in '$langDir': did not find at least one of [train|test].[conllu|parsemetsv], skipping" 1>&2
	fi
    fi
    if [ -f "$langDir/$trainFile" ] && [ -f "$langDir/$testFile" ]; then

	lang=$(basename "$langDir")
	[ -d "$workDir/$lang" ] ||  mkdir "$workDir/$lang"

	nbConfigs=$(cat "$configsFile" | wc -l)
	for configNo in $(seq 1 $nbConfigs); do
	    configFile=$(head -n $configNo "$configsFile" | tail -n 1)
	    checkFile "$configFile"
	    configId=$(basename "${configFile%.conf}")

	    outDir="$workDir/$lang/$configId"

	    if [ ! -s "$outDir/predictions.cupt" ]; then
		[ -d "$outDir" ] || mkdir "$outDir"
		#		params=" $tempOpt -o \"$trainTestOpts\""
		params=""
		if [ ! -z "$evaluate" ]; then
		    params="$params  -e  \"$langDir/$trainFile\""
		fi
#		command="seq-train-test.sh $params -l \"$langDir/$trainFile\" -a  \"$langDir/$testFile\" \"$configFile\" \"$outDir/model\" \"$outDir\" >$outDir/out 2>$outDir/err"
		command="seq-train-test.sh $params -a  \"$langDir/$testFile\" \"$configFile\" \"$outDir/model\" \"$outDir\" >$outDir/out 2>$outDir/err"
		echo "$command"
	    else
		echo "$lang/$configId already done, skipping" 1>&2
	    fi
	done
	
    else
	echo "Warning: ignoring dir '$langDir' (did not find both $trainFile and $testFile)" 1>&2
    fi
done


rm -f "$configsFile"
