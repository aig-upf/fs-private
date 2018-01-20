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
CURRENT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"

echo "> Exporting revision ${REVISION} to directory ${OUTPUT_DIR}"
echo "> Exporting main repository"

# Checkout the desired revision and update the submodules
git checkout ${REVISION}
git submodule update

# Export the revision to the desired directory
git archive HEAD | tar -x -C ${OUTPUT_DIR}

# Export all submodules as well
echo "> Exporting submodules"
git submodule foreach --recursive 'git archive --prefix=$path/ HEAD | tar -x -C '${OUTPUT_DIR}

# Checkout to the previous branch and update the submodules
# to leave everything as it was before
echo "Restoring repo state"
git checkout ${CURRENT_BRANCH}
git submodule update

