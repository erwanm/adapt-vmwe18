#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

trainFile=
testFile=
#force=0
xcrfDir="xcrf-1.0"
javaOpts="-Xms2g -Xmx2g"


function usage {
  echo
  echo "Usage: $progName [options] <config file> <model file> <work dir>"
  echo
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -l <cupt file> train a model with labels given in the last column of this file."
  echo "    -a <cupt file> apply the model and writes predictions to <work dir>/predictions.bio"
  echo "       (replacing the last column of <cupt file>  in this output)."
  echo "    -x <xcrf dir> directory where XCRF software is located. default: '$xcrfDir'"
  #  echo "    -f force recomputing features even if already there"
  echo "    -j <java VM  options>; default: '$javaOpts' "
  echo "    -h this help"
  echo
}







OPTIND=1
while getopts 'hl:a:x:j:' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
#	"f" ) force=1;;
	"l" ) trainFile="$OPTARG";;
	"a" ) testFile="$OPTARG";;
	"x" ) xcrfDir="$OPTARG";;
	"j" ) javaOpts="$OPTARG";;
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
checkFile "$xcrfDir/lib/xcrf.jar"


colNames=$(getConfigParam colNames "$configFile")
colNamesFile=$(writeCommaSepValuesToFile "$colNames" tab)
nbFeatures=$(cat $colNamesFile | wc -w)


[ -d "$workDir" ] || mkdir "$workDir"

if [ ! -z "$trainFile" ]; then # features for training

    checkFile "$trainFile"
    classesFile="$workDir/training.classes"
    cut -f $(( $nbFeatures + 1 )) $trainFile >"$classesFile"
    featuresFile="$workDir/train-data.tsv"
    cut -f 1-$nbFeatures $trainFile >"$featuresFile"
    
    # retrieve training params from config file
    cliqueLevel=$(getConfigParam cliqueLevel "$configFile")
    maxNbTests=$(getConfigParam maxNbTests "$configFile")
    minFreqFeature=$(getConfigParam minFreqFeature "$configFile")
    attributes=$(getConfigParam attributes "$configFile")
    attributesFile=$(writeCommaSepValuesToFile "$attributes")
    neighbourhoods=$(getConfigParam neighbourhoods "$configFile")
    neighbourhoodsFile=$(writeCommaSepValuesToFile "$neighbourhoods")
    selectN=$(getConfigParam selectN "$configFile")
    selectMethod=$(getConfigParam selectMethod "$configFile")
    weightClique=$(getConfigParam weightClique "$configFile")
    
    echo "Generating xml trees"
    xmlTreesDir="$workDir/input-trees-training"
    [ -d "$xmlTreesDir"  ] || mkdir "$xmlTreesDir"
    command="conll-tree-to-xml -n $colNamesFile -t \"$classesFile\" -o \"$xmlTreesDir/\" -k $attributesFile \"$featuresFile\""
    evalSafe "$command"

    echo "Generating features for XCRF"
    if [ "$weightClique" != "0" ]; then
	params="-w"
    fi
    command="generate-xcrf-features-for-training $params -s $selectN -S $selectMethod -c $cliqueLevel -a $attributesFile -t $maxNbTests -m $minFreqFeature \"$xmlTreesDir/\" $neighbourhoodsFile \"$workDir/xcrf-input.xml\""
    evalSafe "$command"

    echo "Running XCRF training"
    xcrfInputFeatures=$(absPath "$workDir/xcrf-input.xml")
    trainingData=$(absPath "$xmlTreesDir/")
    xcrfModel=$(absPath "$modelFile")
    
    cp -R "$xcrfDir" "$workDir"
    xcrfLocalDir="$workDir"/$(basename "$xcrfDir")
    pushd "$xcrfLocalDir" >/dev/null
    command="java $javaOpts -jar lib/xcrf.jar train -v -d class dummy class \"$xcrfInputFeatures\" \"$trainingData\" \"$xcrfModel\""
    evalSafe "$command"
    popd >/dev/null
    cp "$xcrfLocalDir/treecrf.log" "$workDir/treecrf.train.log"

    rm -rf "$xcrfLocalDir" $attributesFile $neighbourhoodsFile
fi

if [ ! -z "$testFile" ]; then # features for testing

    checkFile "$testFile"
    featuresFile="$workDir/test-data.tsv"
    cut -f 1-$nbFeatures $testFile >"$featuresFile"
    
    # retrieve training params from config file
    attributes=$(getConfigParam attributes "$configFile")
    attributesFile=$(writeCommaSepValuesToFile "$attributes")

    echo "Generate xml trees"
    xmlTreesDir="$workDir/input-trees-testing"
    [ -d "$xmlTreesDir" ] || mkdir "$xmlTreesDir"
    command="conll-tree-to-xml -n $colNamesFile -o \"$xmlTreesDir/\" -k $attributesFile \"$featuresFile\""
    evalSafe "$command"

    
    echo "Running XCRF testing"
    testData=$(absPath "$xmlTreesDir/")
    xcrfModel=$(absPath "$modelFile")
    xcrfOutputDir=$(absPath "$workDir/xml-trees-predictions")
    pushd "$xcrfDir" >/dev/null
    command="java $javaOpts -jar lib/xcrf.jar annotate -dAnnWriter class dummy class \"$xcrfModel\" \"$testData\" \"$xcrfOutputDir\""
    eval "$command"
    if [ $? -ne 0 ]; then
	popd >/dev/null
	echo "Error: aborting, an error occurred with command '$command'" 1>&2
	exit 3
    fi
    popd >/dev/null

    echo "Extract class and convert back to connlu"
    command="get-class-from-xml -c O \"$featuresFile\" \"$xcrfOutputDir\" \"$workDir/predictions.bio\""
    evalSafe "$command"
    
    rm -f $attributesFile

fi

rm -f $colNamesFile
