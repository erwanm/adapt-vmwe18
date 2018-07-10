#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

withCateg=


function usage {
  echo
  echo "Usage: $progName [options] <MWE|Tok> <results tsv> <output dir>"
  echo
  echo " Reads pairs  <config dir> <expe dir> from STDIN, one pair by line separated by tab."
  echo " an <expe dir> contains lang directories: <expe dir>/<lang>/<config id>/eval.out"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -c with categories (i.e. exclude 'none'); default: without."
  echo
}







OPTIND=1
while getopts 'ch' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"c" ) withCateg="yep";;
#	"l" ) trainFile="$OPTARG";;
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
if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi


level="$1"
input="$2"
outputDir="$3"


pairsFile=$(mktemp --tmpdir "$progName.XXXXXXXXXX")
while read pair; do
    echo "$pair"
done >$pairsFile

[ -d "$outputDir" ] || mkdir "$outputDir"

languages=$(tail -n +2  "$input" | cut -f 2 | sort -u)
nbCols=$(head -n 1 "$input" | wc -w)
#echo "nbCols=$nbCols" 1>&2
# warning! hoping that other columns don"t contain these strings...
filter="| grep '\s$level\s'"
if [ ! -z "$withCateg" ]; then
    filter="$filter | grep -v '\snone\s'"
fi

#echo "languages=$languages" 1>&2

for lang in $languages; do
    langDir="$outputDir/$lang"
    echo "$lang..." 1>&2
    mkdir "$langDir"
    command="cat \"$input\" $filter | grep  '\s$lang\s' | sort -k $nbCols,${nbCols}n | tail -n 1" 
 #   echo "DEBUG: '$command'" 1>&2
    eval "$command" >"$langDir"/line.tsv
    idSystem=$(cut -f 1-3 "$langDir"/line.tsv)
    cat "$input" | grep "$idSystem" >"$langDir"/lines.tsv
    prevCol=$(( $nbCols - 1 ))
    cut -f "1-2,${prevCol}-${nbCols}" "$langDir"/lines.tsv >"$langDir"/perf.out
    expeDir=$(cut -f 1 "$langDir"/line.tsv)
    configId=$(cut -f 3 "$langDir"/line.tsv)
#    echo "configId=$configId" 1>&2
#    echo "expeDir=$expeDir" 1>&2
    configDir=$(cat $pairsFile | grep "\s$expeDir" | cut -f 1)
#    echo "configDir=$configDir" 1>&2
    configFile="$configDir/$configId.conf"
    if [ -f "$configFile" ]; then
	cp "$configFile" "$langDir/config.conf"
    else
	echo "Error: cannot find config file '$configFile'" 1>&2
    fi
    modelDir="$expeDir/$lang/$configId/model"
    if [ -d "$modelDir" ]; then
	cp -R "$modelDir" "$langDir"
    else
	echo "Error: cannot find model dir '$modelDir'" 1>&2
    fi
    cat "$langDir"/perf.out
done

#cat "$outputDir"/*/perf.out

#echo "pairsFile=$pairsFile" 1>&2
rm -f $pairsFile
