#!/bin/bash





function checkFile {
    f="$1"
    if [ ! -f "$f" ]; then
	echo "Error: cannot find file '$f'" 1>&2
	exit 1
    fi
}



function absPath {
    f="$1"
    pushd $(dirname "$f") >/dev/null
    echo $(pwd)/$(basename "$f")
    popd >/dev/null
}



# returns a temporary file
function writeCommaSepValuesToFile {
    values="$1"
    sepTab="$2"
    tmp=$(mktemp --tmpdir "$progName.XXXXXXXXX")
    if [ -z "$sepTab" ]; then
	echo "$values" | tr ',' '\n' >$tmp
    else
	echo "$values" | tr ',' '\t' >$tmp
    fi
    echo "$tmp"
}



function getConfigParam {
    name="$1"
    configFile="$2"

    v=$(grep "$name=" "$configFile" | grep -v "^#" |  sed "s/$name=//" | tail -n 1)
    if [ -z "$v" ]; then
	echo "Error: no parameter '$name' found in config file '$configFile'" 1>&2
	exit 1
    fi
    echo "$v"
}


function evalSafe {
    command="$1"
#    echo "DEBUG: '$command'"  1>&2
    eval "$command"
    if [ $? -ne 0 ]; then
	echo "Error: aborting, an error occurred with command '$command'" 1>&2
	exit 3
    fi
}


