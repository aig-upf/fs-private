#!/bin/bash

# Simply forward the call to the python generator making sure that python3 with a fixed hash seed is invoked.
echo "PYTHONHASHSEED=1 python3 -OO generator.py --set $1 --instance $2"
PYTHONHASHSEED=1 python3 -OO generator.py --set $1 --instance $2

