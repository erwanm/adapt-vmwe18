#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

trainFile=
testFile=
#force=0
trainTestOpts=
evalTrain=
useTempDir=

function usage {
  echo
  echo "Usage: $progName [options] <config file> <model dir> <work dir>"
  echo
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -l <cupt file> train a model with labels given in the last column of this file."
  echo "    -a <cupt file> apply the model and writes predictions to <work dir>/predictions.cupt"
  echo "       (replacing the last column of <cupt file>  in this output)."
  #  echo "    -f force recomputing features even if already there"
  echo "    -o <train-test.sh options> e.g. -o '-x path-to-xcrf'"
  echo "    -e <training data> evaluate with official script (used only with -a)"
  echo "    -t use temp dir for intermediate processing and remove it at the end;"
  echo "    -h this help"
  echo
}






OPTIND=1
while getopts 'hl:a:o:e:t' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
#	"f" ) force=1;;
	"l" ) trainFile="$OPTARG";;
	"a" ) testFile="$OPTARG";;
	"o" ) trainTestOpts="$OPTARG";;
	"e" ) evalTrain="$OPTARG";;
	"t" ) useTempDir="yep";;
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
modelDir="$2"
workDir="$3"


#

checkFile "$configFile"


colNames=$(getConfigParam colNames "$configFile")
colNamesFile=$(writeCommaSepValuesToFile "$colNames" tab)
nbFeatures=$(cat $colNamesFile | wc -w)


[ -d "$workDir" ] || mkdir "$workDir"

[ -d "$modelDir" ] || mkdir "$modelDir"


if [ ! -z "$trainFile" ]; then # features for training

    if [ ! -z "$useTempDir" ]; then
	originalWorkDir="$workDir"
	workDir=$(mktemp -d --tmpdir "$progName.dir.XXXXXXXXX")
    fi

    checkFile "$trainFile"
    
    # retrieve training params from config file
    xcrfLabels=$(getConfigParam xcrfLabels "$configFile")
    methodCateg=$(getConfigParam methodCateg "$configFile")
    overlapKillSmallestExpr=$(getConfigParam overlapKillSmallestExpr "$configFile")


    echo "Generating the BIO-annotated training data (possibly several files if indep categories selected)"
    rm -f "$workDir"/*.bio
    params=""
    if [ "$overlapKillSmallestExpr" != "0" ]; then
	params="$params -k"
    fi
    # methodCateg values: none, joint or indep
    if [  "$methodCateg" == "indep" ]; then
	categoriesComma=$(getConfigParam categories "$configFile")
	categories=$(echo "$categoriesComma" | tr ',' ' ')
	for categ in $categories; do
	    # check that this categ is used in this file
	    n=$(cat "$trainFile" | cut -f $(( $nbFeatures + 1 )) | grep "$categ" | wc -l)
	    if [ $n -gt 0 ]; then
		command="cupt-to-bio-labels $params -c \"$categ\" $xcrfLabels \"$trainFile\" \"$workDir/$categ.bio\""
		evalSafe "$command"
	    fi
	done
    else
	if [ "$methodCateg" == "none" ]; then
	    params="$params -i"
	elif [  "$methodCateg" != "joint" ]; then
	    echo "Error: invalid value for 'methodCateg' in config '$configFile': '$methodCateg'" 1>&2
	    exit 4
	fi
	# none or joint
	command="cupt-to-bio-labels $params $xcrfLabels \"$trainFile\" \"$workDir/$methodCateg.bio\""
	evalSafe "$command"
    fi

    # 2. training
    echo "Training..."
    for bioTrainFile in "$workDir"/*.bio; do
	name=$(basename "${bioTrainFile%.bio}")
	bioDir="$workDir/$name"
	modelFile="$modelDir/$name.xcrf-model"
	echo "  - training on $bioTrainFile, model written to $modelFile"
	[ -d "$bioDir" ] || mkdir "$bioDir"
	command="train-test.sh $trainTestOpts -l \"$bioTrainFile\" \"$configFile\" \"$modelFile\" \"$bioDir\""
	evalSafe "$command"
    done

    if [ ! -z "$useTempDir" ]; then
	# nothing to copy after training
	rm -rf "$workDir"
	workDir="$originalWorkDir"
    fi

fi


    
if [ ! -z "$testFile" ]; then # features for testing

    if [ ! -z "$useTempDir" ]; then
	originalWorkDir="$workDir"
	workDir=$(mktemp -d --tmpdir "$progName.dir.XXXXXXXXX")
    fi


    checkFile "$testFile"

    # retrieve training params from config file
    methodCateg=$(getConfigParam methodCateg "$configFile")


    # testing
    echo "Testing..."
    rm -f "$workDir"/*.predictions.cupt
    n=$(ls "$modelDir"/*.xcrf-model | wc -l)
    if [ $n == 0 ]; then
	echo "Error: no model found in '$modelDir'" 1>&2
	exit 1
    else
	echo "Info: $n model(s) found in '$modelDir'"
    fi
    for modelFile in "$modelDir"/*.xcrf-model; do
	echo "  - applying model $modelFile to $testFile"
	name=$(basename "${modelFile%.xcrf-model}")
	bioDir="$workDir/$name" # same as for training normally
	[ -d "$bioDir" ] || mkdir "$bioDir"
	command="train-test.sh $trainTestOpts -a \"$testFile\" \"$configFile\" \"$modelFile\" \"$bioDir\""
	evalSafe "$command"
	command="bio-to-cupt-labels \"$bioDir/predictions.bio\" \"$bioDir.predictions.cupt\""
	evalSafe "$command"
    done

    if [  "$methodCateg" == "indep" ]; then
	echo "Merging independent categories"
	# merge independent predictions by category
	command="merge-independent-labels \"$workDir/predictions.cupt\" \"$workDir\"/*.predictions.cupt"
	evalSafe "$command"
    else
	mv "$workDir/$methodCateg.predictions.cupt" "$workDir/predictions.cupt"
    fi

    if [ ! -z "$useTempDir" ]; then
	mv "$workDir/predictions.cupt" "$originalWorkDir"
#	if [ ! -z "$evalTrain" ]; then
#	    mv "$workDir"/eval*.out "$originalWorkDir"
#	fi
	rm -rf "$workDir"
	workDir="$originalWorkDir"
    fi

    
    echo "Checking cupt output"
    command="validate_cupt.py --input \"$workDir/predictions.cupt\""
    evalSafe "$command"

  
    if [ ! -z "$evalTrain" ]; then
	echo "Evaluating"
	command="evaluate.py --gold \"$testFile\" --pred \"$workDir/predictions.cupt\" >\"$workDir/eval.out\""
	evalSafe "$command"
	command="evaluate.py --train \"$evalTrain\" --gold \"$testFile\" --pred \"$workDir/predictions.cupt\" >\"$workDir/eval.long.out\""
	evalSafe "$command"
    fi


    
fi

rm -f $colNamesFile
