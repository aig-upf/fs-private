
# Development Notes for the SDD Module

## Installation

### Python components
The following instructions have been tested on Python 3.{6,7} virtual environments.

1. Clone from <https://github.com/wannesm/PySDD> and run  `pip install -e .` on the root folder.
(_Ignore the installation instructions that require you to manually build and install the SDD libraries_!)
1. Go to the root directori of _this_ project and run `pip install -r requirements.txt`

### Updating the dependencies
To update dependencies for already-installed packages, simply run

    pip install --upgrade -r requirements.txt

### C++ components

The build scripts expects the UCLA SDD library to be installed in the system.
The header files are included in this repo, since some modifications were needed, so you do not need to do anything
about them; you will need however to compile the dynamic library for the project.
The easiest way to is to [download the code](http://reasoning.cs.ucla.edu/sdd/),
compile it (`cd libsdd-2.0; scons`), and place the resulting `libsdd.so` file somewhere in your system that is included
in the `LD_LIBRARY_PATH` library, such as `/usr/lib/` or `~/local/lib/`.

## Usage

### Example Usage

    # Running FS-SDD
    ./run.py --debug --sdd -i $DOWNWARD_BENCHMARKS/blocks/probBLOCKS-4-0.pddl --driver bfs-sdd
