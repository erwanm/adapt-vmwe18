#!/bin/bash

source lib-mwe.sh

set -e

progName=$(basename "$BASH_SOURCE")

#withCateg=
#runTasks=
additionalLangs=

function usage {
  echo
  echo "Usage: $progName [options] <name> <dir from process-apply-best-to-test.sh>"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -a <dir for languages no deps>"
#  echo "    -c with categories (i.e. exclude 'none'); default: without."
#  echo "    -r start tasks in parallel at the end instead of only printing to a file"
  echo
}







OPTIND=1
while getopts 'ha:' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"a" ) additionalLangs="$OPTARG";;
#	"r" ) runTasks="yep";;
#	"l" ) trainFile="$OPTARG";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 2 ]; then
    echo "Error: expecting 2 args." 1>&2
    printHelp=1
fi
if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi

name="$1"
dir="$2"

rm -rf "$name.zip"
d=$(mktemp -d --tmpdir "$progName.XXXXXXXXXX")
for langDir in "$dir"/*; do
    if [ -d "$langDir" ]; then
	lang=$(basename "$langDir")
	if [ -s "$langDir/predictions.cupt" ]; then
	    mkdir "$d/$lang"
	    cp "$langDir/predictions.cupt" "$d/$lang/test.system.cupt"
	else
	    echo "Warning: cannot find $langDir/predictions.cupt" 1>&2
	fi
    fi
done

if [ ! -z "$additionalLangs" ]; then
    cp -R "$additionalLangs"/* $d
fi

pushd "$d" >/dev/null
zip -r "$name.zip" *
popd >/dev/null
mv "$d/$name.zip" .
rm -rf "$d"
