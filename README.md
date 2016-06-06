
The FS Functional STRIPS planner
=================================

`FS` is a classical planner that works with the Functional STRIPS planning language [[Geffner, 2000]](#ref-geffner-fstrips-2000),
a modeling language based on the quantifier-free
fragment of first-order logic that includes constant, function and predicate symbols, but no variable symbols. The increased expressiveness
of the Functional STRIPS language with respect to propositional languages such as standard STRIPS (which is indeed subsumed by Functional STRIPS)
often results in problem encodings which are more compact, more readable, have fewer ground actions
and preserve the structural properties of the problem in a manner which allows the derivation of more effective heuristics.

Along with the core of the Functional STRIPS language, the `FS` planner supports certain extensions which are useful both
from the expressive _and_ the computational point of view. These include _existential quantification_,
_state constraints_, a fairly large library of _global constraints_, and the possibility of using _externally-defined symbols_
and _built-in arithmetic symbols_.

This documentation covers a number of practical issues related to the use of the `FS` planner. The planner, however, has 
been used and described in a number of academic publications that [can be found here](http://gfrances.github.io/pubs/),
the most recent of which are [[Francès and Geffner, 2015]](#ref-frances-modeling-2015) and [[Francès and Geffner, 2016a]](#ref-frances-existential-2016)
and [[Francès and Geffner, 2016b]](#ref-frances-effective-2016).

* [Installation](#installation)
* [Usage](#usage)
* [Credits](#credits)
* [References](#references)



## <a name="references"></a>Installation
In order to install and run the `FS` planner, you need the following software components:

1. The [LAPKT Planning Toolkit](http://lapkt.org/), which provides the base search algorithms used with our heuristics.
You should use the branch `v2_work`, as explained [here](https://github.com/miquelramirez/LAPKT-public/blob/v2-work/aptk2/README.md).

1. A [custom version](https://bitbucket.org/gfrances/downward-aig) of the Fast Downward PDDL 3.0 parser (written in Python), modified with the purpose of fully supporting the functional capabilities of the language and allowing for constraints and external procedures to be used on the specification of the domain.

1. The [Gecode CSP Solver](http://www.gecode.org/) (tested with version 4.4.0 only). The recommended way to install it is on `~/local`, i.e. by running `./configure --prefix=~/local` before the actual compilation.


Once you have installed these projects locally, your system needs to be configured with the following environment variables,
e.g. by setting them up in your  `~/.bashrc` configuration file:


```shell
export LAPKT_PATH="${HOME}/projects/code/lapkt"
export LAPKT2_PATH="${LAPKT_PATH}/aptk2"
export FD_AIG_PATH="${HOME}/projects/code/downward/downward-aig"
export FS_PATH="${HOME}/projects/code/fs"

# Local C++ library installations
export LD_LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
if [[ -d ${HOME}/local/lib ]]; then
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/local/lib
fi

# LAPKT and FS libraries
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${FS_PATH}/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${LAPKT2_PATH}/lib

# AIG Fast Downward PDDL Parser
if [[ -d ${FD_AIG_PATH}/src/translate ]]; then
	export PYTHONPATH="${FD_AIG_PATH}/src/translate:$PYTHONPATH"
fi

```

Once all this is set up, you can build the project library by doing

```shell
cd $FS_PATH
scons
```

You can run `scons debug=1` to build the debug version of the library, or `scons edebug=1` to build an extremely-verbose debug version.


## <a name="usage"></a>Usage

`FS` is invoked through a high-level Python script which parses any FSTRIPS problem specification and generates certain data which is necessary for the
main planner module to run. The main Python preprocessing script is `$FS_PATH/preprocessor/runner.py`, and can be invoked like
(replace `$BENCHMARKS` by an appropriate directory):

```shell
python3 runner.py --tag foo --instance $BENCHMARKS/fn-simple-sokoban/instance_6.pddl --run --driver=smart
```

Where `instance_6.pddl` is a Functional STRIPS (or standard STRIPS) instance file, and 
`foo` is an arbitrary name that will be used to determine the output directory where the executable solver, related data, and results will be left, which in this case will be
`$FS_PATH/generated/test/fn-simple-sokoban/instance_6`. Read below for further details on the semantics of the `--run` and `--driver` options.
If externally-defined symbols are used, the parsing process involves the automatic generation of a bunch of C++ classes that 
is then compiled and linked against the binary solver.
If `--run` is not used, only the parsing stage is run, in which case all files necessary to run the solver are left on the abovementioned directory, and nothing else is done.
If that is the case, we can run the automatically generated `solver.bin` executable from that directory (add the `-h` flag for further options :

```shell
cd $FS_PATH/generated/test/fn-simple-sokoban/instance_6
./solver.bin
```

Note that only the non-debug executable is built by default, but you can invoke the `generator.py` script with flags `--debug` and `--edebug` to control the debug level
of the resulting executable.


Once the planner for a particular problem instance has been compiled, and we are about to run it on the particular planner directory,
a number of options can be specified on the command line, the most prominent of them being the search driver.


### Search Driver

The planner is pre-configured with a number of "search drivers" that specify the global search strategy
(search algorithm plus heuristic, if necessary) that will be followed in the search for a plan.
The `--driver` command-line option is thus mandatory; for instance, to use the planner with the `smart` driver we would invoke 
`./solver.bin --driver=smart`. The following are the main available drivers:

* `lite`: The lite driver is a greedy best-first search which works with the _non-constrained_ RPG heuristics, i.e. computes the _standard_
h_FF and h_MAX heuristics, the only difference being that it can work with the more compact Functional STRIPS encodings, thus not being restricted to
predicative STRIPS.

* `native`: The native driver implements a greedy best-first search coupled with a _constrained_ RPG heuristic that can be
either the h_FF (heuristic=hff) or the h_MAX heuristic (heuristic=hmax). The particularity of this driver is that the CSPs into which
the computation of the heuristic is mapped are not solved by Gecode, but rather by a native, hand-coded simplified approach which might yield some performance gain, since it avoids the overhead of interacting with Gecode. The downside of this approach is that only a certain subset of 
FSTRIPS is accepted (namely, that which results in very simple CSPs), and only approximated CSP resolution is used.

* `standard`: A greedy best-first search with one of the two _constrained_ RPG heuristics, which is computed with a 1-CSP-per-ground-action model.

* `smart`: The smart driver implements a greedy best-first search coupled with a _constrained_ RPG heuristic that can be
either the h_FF (heuristic=hff) or the h_MAX heuristic (heuristic=hmax). When it comes to computing the heuristic, though, 
this driver does not need to work on a "1-CSP-per-ground-action" basis --- the limitation of this being that
the number of ground actions, and thus of CSPs, is exponential in the number of parameters of the action schemas.
Instead, this driver works on a "1-CSP-per-action-effect" basis, but the grounding is "smart", i.e. it only grounds as much as necessary
so that the effect head is a state variable. Among other things, this means that the number of CSPs is now
exponential only in the number of action parameters in the head of the effect, which will tipically be smaller.
The `smart` driver accepts conditional effects.

* `lifted`: The lifted driver implements a fully-lifted greedy-best first search, meaning that actions are never grounded, but instead
the constraint-based nature of the planner are used to model the task of deciding which actions are applicable in a given state as a particular CSP which is then solved whenever we need to expand a node during the search. This can yield a benefit in problems with a huge number
of ground actions, which usually will not work well with traditional planners that ground all the action schemas, as they will never go beyond the grounding phase.

* `smart_lifted`: This driver conducts a fully-lifted search as the `lifted` driver above, 
This is thus equivalent to the `smart` driver above, but using lifted search as in the `lifted` driver.

* `unreached_atom`: An experimental driver which performs a greedy best-first search on a version of the constrained RPG heuristics which is computed
in a somewhat different manner that iterates through atoms that have not yet been reached in the RPG, trying to achieve them one by one.
Seems to perform better than other options in some domains, but not in general.

* `iw`: Iterated Width search.

* `novelty_best_first`: A Greedy best-first search with a novelty-based heuristic. Namely, the search favors states with
(1) lower novelty, (2) higher number of satisfied goal atoms, if novelty is equal, and (3) lower accumulated cost, the two
first factors being equal.

* `bfs`: A blind, standard breadth-first search.


### Other Options

There are some other important options that can also be specified on the command line. Some of them are particular to each driver,
some of them are general to more than one (perhaps to all) drivers.
These options are not mandatory and, if not provided, the defaut value for each of them is the one read from the
[`defaults.json` configuration file](https://github.com/aig-upf/fs/blob/master/planners/generic/defaults.json) which is located in the planner directory.
If you want to override the value of a particular option through the command line, you can do so with the `--options` parameter,
which accepts a list of comma-separated name=value pairs, as in e.g.
`./solver.bin --driver=smart --options="heuristic=hff,novelty=false"`.

The most common configuration options are:

* `heuristic`: Usually either `hff` or `hmax`.
* `evaluation`: Either `eager`, `delayed`, or `delayed_for_unhelpful`. The first two are standard, the last one means that only
  those nodes that are considered helpful are evaluated eagerly, the rest are evaluated only once they are opened.
* `goal_resolution`: Either `full` or `approximate`: whether to fully or only approximately solve the action goal CSPs.
* `precondition_resolution`: Same than `goal_resolution` but for action precondition formulas.
* `novelty`: Either `true` or `false`.
Some CSP models for the computation of  support some kind of extra constraint to enforce that the solutions
of the CSP do indeed map into atoms which are novel in the RPG. This variable controls the usage of these constraints.


Besides, there are some other obscure / experimental options, mostly for internal usage and testing:
* `plan_extraction`: Either `propositional` or `extended`. The type of plan extraction procedure.
* `goal_value_selection`: Either `min_val` and `min_hmax`. The type of CSP value selection to use in goal CSPs. 
* `action_value_selection`: Same than `goal_value_selection`, but for action CSPs.
* `support_priority`: Either `first` or `min_hmaxsum`. Which support sets should be given priority.



## <a name="credits"></a>Credits

The `FS` planner is partially built upon the [Lightweight Automated Planning Toolkit](http://www.lapkt.org)
and the PDDL parser from the [Fast Downward](http://www.fast-downward.org) distribution.

## <a name="references"></a>References

* <a name="ref-frances-modeling-2015">Francès, G., and Geffner, H. (2015)</a>,
[_Modeling and Computation in Planning: Better Heuristics from More Expressive Languages_](http://gfrances.github.io/pubs/2015-icaps-better-heuristics-more-expressive-languages/), ICAPS 2015.

* <a name="ref-frances-existential-2016">Francès, G., and Geffner, H. (2016a)</a>,
[_E-STRIPS: Existential Quantification in Planning and Constraint Satisfaction_](http://gfrances.github.io/pubs/2016-ijcai-existential-quantification-planning-csp/), IJCAI 2016.

* <a name="ref-frances-effective-2016">Francès, G., and Geffner, H. (2016b)</a>,
[_Effective Planning with More Expressive Languages_](http://gfrances.github.io/pubs/2016-ijcai-effective-planning-more-expressive-languages/), IJCAI 2016.


* <a name="ref-geffner-fstrips-2000">Geffner, H. (2000)</a>,
[_Functional STRIPS: A more flexible lan-
guage for planning and problem solving_](http://www.tecn.upf.es/~hgeffner/).
In Minker, J., ed., Logic-Based Artificial Intelligence. Kluwer. 187–205.




