#!/usr/bin/env bash

set -ex

# Use ccache to avoid unnecessary recompilations
CC="ccache gcc" python setup.py build -j4 --debug
pip install -e .
