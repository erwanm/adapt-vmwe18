#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

trainFile=
testFile=
#force=0
evalTrain=""

function usage {
  echo
  echo "Usage: $progName [options] <config file> <model file> <work dir>"
  echo
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -l <cupt file> train a model with labels given in the last column of this file."
  echo "    -a <cupt file> apply the model and writes predictions to <work dir>/predictions.cupt"
  echo "       (adding the predicted labels after the last column of <cupt file> in this output)."
  echo "    -e <training data> evaluate with official script (used only with -a)"
   #  echo "    -f force recomputing features even if already there"
  echo "    -h this help"
  echo
}







OPTIND=1
while getopts 'hl:a:e:' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
#	"f" ) force=1;;
	"l" ) trainFile="$OPTARG";;
	"a" ) testFile="$OPTARG";;
        "e" ) evalTrain="$OPTARG";;
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


configFile="$1"
modelFile="$2"
workDir="$3"


#

checkFile "$configFile"



[ -d "$workDir" ] || mkdir "$workDir"

if [ ! -z "$trainFile" ]; then # features for training

    checkFile "$trainFile"
    labels=$(getConfigParam labels "$configFile")

    cat "$trainFile" | grep -v "^#" | tr ' ' '_'  >"$workDir/train.cupt"
    
    echo "Converting training data to '$labels' labels..."
    command="cupt-to-bio-labels -i $labels \"$workDir/train.cupt\" \"$workDir/train.bio\""
    eval "$command"
    
    echo "Training..."
    command="crf-config-train.sh -t \"$workDir/template.wapiti\" \"$configFile\" \"$workDir/train.bio\" \"$modelFile\""
    eval "$command"
fi

if [ ! -z "$testFile" ]; then # features for testing

    checkFile "$testFile"
    labels=$(getConfigParam labels "$configFile")

    # "cut -f 1-10" to remove gold label column
    cat "$testFile" | grep -v "^#" | tr ' ' '_'  | cut -f 1-10 >"$workDir/test-input.cupt"
    
    echo "Applying..."
    eval "crf-config-test.sh  \"$configFile\"  \"$modelFile\" \"$workDir/test-input.cupt\" \"$workDir/test-output.bio\""
    eval "$command"

    echo "Converting from '$labels' labels to cupt format..."
    command="bio-to-cupt-labels \"$workDir/test-output.bio\" \"$workDir/predictions.cupt.0\""
    evalSafe "$command"

    # reinjecting first line with columns labels
    head -n 1 "$testFile" >"$workDir/predictions.cupt"
    cat "$workDir/predictions.cupt.0" >>"$workDir/predictions.cupt"

    
    echo "Checking cupt output"
    command="validate_cupt.py --input \"$workDir/predictions.cupt\""
    evalSafe "$command"
    
    if [ ! -z "$evalTrain" ]; then
	echo "Evaluating"
	command="evaluate.py --train \"$evalTrain\" --gold \"$testFile\" --pred \"$workDir/predictions.cupt\" >\"$workDir/eval.out\""
	evalSafe "$command"
    fi
    
    
fi

