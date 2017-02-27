#!/bin/bash

set -e

command -v module >/dev/null 2>&1 && module load gcc/6.2.0

cd $LAPKT && rm -rf include lib
cd $LAPKT2_PATH && ./build.py && ./install.py
cd $LAPKT_NOVELTY_PATH && ./build.py && ./install.py

cd $FS_PATH && ./build.py
