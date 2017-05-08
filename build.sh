#!/bin/bash

set -e

command -v module >/dev/null 2>&1 && module load gcc/6.2.0

cd $LAPKT && rm -rf include lib
cd $LAPKT2_PATH && ./build.py --all  && ./install.py --all
cd $LAPKT_NOVELTY_PATH && ./build.py --all  && ./install.py --all

cd $FS_PATH && ./build.py --all
