
FS - Installation Instructions
=================================


### <a name="manual-installation"></a>Manual Installation

Alternatively, you can perform a full installation of the planner and all its dependencies from source.
To begin with, you will need the following software components:

1. The [LAPKT Planning Toolkit](http://lapkt.org/), which provides the base search algorithms used with our heuristics.
You should use the branch `v2_work`, as explained [here](https://github.com/miquelramirez/LAPKT-public/blob/v2-work/aptk2/README.md).

1. The [Gecode CSP Solver](http://www.gecode.org/) (tested with version 4.4.0 only). The recommended way to install it is on `~/local`, i.e. by running `./configure --prefix=~/local` before the actual compilation.


Once you have installed these projects locally, your system needs to be configured with the following environment variables,
e.g. by setting them up in your  `~/.bashrc` configuration file:


```shell
export LAPKT_PATH="${HOME}/projects/code/lapkt"
export LAPKT2_PATH="${LAPKT_PATH}/aptk2"
export FS_PATH="${HOME}/projects/code/fs"

# Local C++ library installations
export LD_LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
if [[ -d ${HOME}/local/lib ]]; then
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/local/lib
fi

# LAPKT and FS libraries
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${FS_PATH}/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${LAPKT2_PATH}/lib
```

Once all this is set up, you can build the project library by doing

```shell
cd $FS_PATH
scons
```

You can run `scons debug=1` to build the debug version of the library, or `scons edebug=1` to build an extremely-verbose debug version.

