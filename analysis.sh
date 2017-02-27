#!/bin/bash

set -e

scan-build -o analysis scons debug=1
