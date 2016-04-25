
The FS Functional STRIPS planner
=================================

`FS` is a classical planner that accepts the Functional STRIPS planning language, along with certain extensions
which are useful both from the expressive _and_ the computational point of view, including the use of
state constraints, of a fairly large library of global constraints, and the possibility of using external procedures.

The `FS` planner has been used and described in a number of publications that [can be found here](http://gfrances.github.io/pubs/),
the most recent of which are 

* Francès, G., and Geffner, H.,
"[E-STRIPS: Existential Quantification in Planning and Constraint Satisfaction](http://gfrances.github.io/pubs/2016-ijcai-existential-quantification-planning-csp/)" (IJCAI 2016)

* Francès, G., and Geffner, H.,
"[Effective Planning with More Expressive Languages](http://gfrances.github.io/pubs/2016-ijcai-effective-planning-more-expressive-languages/)" (IJCAI 2016).



-- Guillem Francès <guillem.frances@upf.edu>

Installation
--------------
In order to install and run the `FS` planner, you need the following software components:

1. The [LAPKT Planning Toolkit](http://lapkt.org/), which provides the base search algorithms used with our heuristics.

1. A [custom version](https://bitbucket.org/gfrances/downward-aig) of the Fast Downward PDDL 3.0 parser (written in Python), modified with the purpose of fully supporting the functional capabilities of the language and allowing for constraints and external procedures to be used on the specification of the domain.

1. The [Gecode](http://www.gecode.org/) CSP Solver (Tested with version 4.4.0 only). The recommended way to install it is on `~/local`, i.e. by running `./configure --prefix=~/local` before the actual compilation.

<!-- 1. The Clingo ASP Solver, from [Potassco](http://potassco.sourceforge.net/), the Potsdam Answer Set Solving Collection. Clingo seems to require at least `gcc` > 4.8; in order to build the C++ library you need to run `scons libclingo`. -->


Once you have installed these projects locally, your system needs to be configured with the following environment variables, e.g. by setting them up in your  `~/.bashrc` configuration file:


```shell
export LAPKT_PATH="${HOME}/projects/code/lapkt"
export FD_AIG_PATH="${HOME}/projects/code/downward/downward-aig"
export FS0_PATH="${HOME}/projects/code/fs0"
export CLINGO_PATH="${HOME}/lib/clingo-4.5.4-source"

# Local C++ library installations
export LD_LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
if [[ -d ${HOME}/local/lib ]]; then
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/local/lib
fi

# AIG Fast Downward PDDL Parser
if [[ -d ${FD_AIG_PATH}/src/translate ]]; then
	export PYTHONPATH="${FD_AIG_PATH}/src/translate:$PYTHONPATH"
fi

```

Once all this is set up, you can build the `FS0` library by doing

```shell
cd $FS0_PATH
scons
```

You can run `scons debug=1` to build the debug version of the library, or `scons edebug=1` to build an extremely-verbose debug version.


Solving planning instances
----------------------------------

The actual process of solving a planning problem involves a preprocessing phase in which a Python script is run to parse a PDDL 3.0 problem specification and generate certain data, as well as a bunch of C++ classes that need to be compiled against the `FS0` main library. The main Python preprocessing script is `$FS0_PATH/preprocessor/generator`.
You can bootstrap the whole process by running e.g. (replace `$BENCHMARKS` by an appropriate directory):

```shell
python3 generator.py --set test --instance $BENCHMARKS/fn-simple-sokoban/instance_6.pddl
```

Where `instance_6` is a PDDL3.0 instance file, and 
`test` is an arbitrary name that will be used to determine the output directory where the executable solver will be left, which in this case will be
`$FS0_PATH/generated/test/fn-simple-sokoban/instance_6`.
In order to solve the instance, we need to run the automatically generated `solver.bin` executable from that directory (add the `-h` flag for further options :

```shell
cd $FS0_PATH/generated/test/fn-simple-sokoban/instance_6
./solver.bin
```

Note that only the non-debug executable is built by default, but you can invoke the `generator.py` script with flags `--debug` and `--edebug` to control the debug level
of the resulting executable.


