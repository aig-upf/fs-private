#!/bin/bash
# The purpose of this script is to export a certain snapshot of the code at a certain revision in time.
# In other words, it does the same as a "git archive" of a certain revision, but taking into account the
# submodules of the main repository and exporting them also as they were on that revision.
# To do this safely, i.e. without risking the loss of any code in the working tree or in the index,
# we first copy the whole source tree in a temporary directory and then manipulate it.

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

# Let's create a temporary working directory
WORK_DIR=`mktemp -d`
if [[ ! "$WORK_DIR" || ! -d "$WORK_DIR" ]]; then
  echo "ERROR: Could not create temp dir"
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

# Copy the entire working tree to the temp directory to manipulate it safely
#cp -r $DIR/. $WORK_DIR
rsync -az --exclude=CMakeFiles --exclude=experiments --exclude=workspace --exclude='*.bin' --exclude='.build' --exclude=".idea" \
      $DIR/. $WORK_DIR
cd $WORK_DIR

echo "> Checking out revision $REVISION"
# First get rid of undesired changes in working tree and index,
git clean -fd
git reset --hard HEAD
# Then checkout the desired revision and update the submodules at that point in time
git checkout -b export-branch ${REVISION}
git submodule update

# Export the revision to the desired directory
echo "> Exporting main repository"
git archive HEAD | tar -x -C ${OUTPUT_DIR}

# Export all submodules as well
echo "> Exporting submodules"
git submodule foreach 'git archive --prefix=$path/ HEAD | tar -x -C '${OUTPUT_DIR}
