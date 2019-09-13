
# Development Notes for the SDD Module

## Installation

The build scripts expects the UCLA SDD library to be installed in the system.
The header files are included in this repo, since some modifications were needed, so you do not need to do anything
about them; you will need however to compile the dynamic library for the project.
The easiest way to is to [download the code](http://reasoning.cs.ucla.edu/sdd/),
compile it (`cd libsdd-2.0; scons`), and place the resulting `libsdd.so` file somewhere in your system that is included
in the `LD_LIBRARY_PATH` library, such as `/usr/lib/` or `~/local/lib/`.