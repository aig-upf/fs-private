#!/bin/bash

set -e

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 REV_NUMBER OUTPUT_DIR" >&2
  exit 1
fi
if ! [ -d "$2" ]; then
  echo "ERROR: $2 is not an existing directory" >&2
  echo "Usage: $0 REV_NUMBER OUTPUT_DIR" >&2
  exit 1
fi

REVISION=$1
OUTPUT_DIR=$2

# the directory of the script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "Current dir is: $DIR"


# the temp directory used, within $DIR
# omit the -p parameter to create a temporal directory in the default location
WORK_DIR=`mktemp -d`

# check if tmp dir was created
if [[ ! "$WORK_DIR" || ! -d "$WORK_DIR" ]]; then
  echo "Could not create temp dir"
  exit 1
else
  echo "> Created temporary directory $WORK_DIR"
fi

# deletes the temp directory
function cleanup {
  rm -rf "$WORK_DIR"
  echo "Deleted temp working directory $WORK_DIR"
}

# register the cleanup function to be called on the EXIT signal
trap cleanup EXIT

cp -r $DIR/. $WORK_DIR
cd $WORK_DIR

# Get rid of undesired changes
git clean -fd
git checkout -- .

# Checkout the desired revision and update the submodules at that point in time
git checkout -b export-branch ${REVISION}
git submodule update

# Export the revision to the desired directory
git archive HEAD | tar -x -C ${OUTPUT_DIR}

# Export all submodules as well
echo "> Exporting submodules"
git submodule foreach 'git archive --prefix=$path/ HEAD | tar -x -C '${OUTPUT_DIR}
