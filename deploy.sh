#!/bin/bash

set -e

PLANNER_PATH=$(dirname $(realpath -s $0))
PLANNER_DIR=$(basename $PLANNER_PATH)
FS_DEPLOY_DIRNAME=/tmp/fs_deploy_`head /dev/urandom | tr -dc A-Za-z0-9 | head -c10`
CURRENT_DATE=$(date '+%Y-%m-%d %H:%M:%S')

echo "Deploying FS planner from \"$PLANNER_PATH\" by using temp directory \"$FS_DEPLOY_DIRNAME\""
rm -rf $FS_DEPLOY_DIRNAME && mkdir -p $FS_DEPLOY_DIRNAME && cd $FS_DEPLOY_DIRNAME

echo "Rsync'ing all files"
rsync -az $PLANNER_PATH \
        --exclude '.git' --exclude '.build' \
        --exclude '.idea' --exclude '*.img' \
        --exclude 'workspace' --exclude 'CMakeFiles' \
        .

#mkdir fs-ipc-2018

echo "Consolidating files"
cd $PLANNER_DIR
rm -rf vendor/*
cp -R submodules/lapkt-base vendor/
cp -R submodules/lapkt-novelty vendor/
cp -R submodules/rapidjson vendor/
rm -rf submodules


echo "Cloning Bitbucket repo and updating it"
cd ..
git clone --branch=ipc-2018-seq-sat git@bitbucket.org:gfrances/fs-ipc-2018.git
cp -R $PLANNER_DIR/* fs-ipc-2018/
cd fs-ipc-2018
git status
git add .
git commit -m "Automated deploy and push on $CURRENT_DATE"
git push

rm -rf $FS_DEPLOY_DIRNAME
