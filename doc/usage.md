
FS planner - Usage
=================================


The `FS` planner is usually invoked through a Python script which parses any FSTRIPS
problem specification and generates certain data which is necessary for the
main planner to run. The Python script is `run.py`; you can obtain a list of possible options
by invoking:

```shell
./run.py -h
```

Assuming you have some PDDL / FSTRIPS benchmarks in the `$BENCHMARKS` directory, a typical invokation
of the planner will look like this

```shell
./run.py -i $BENCHMARKS/blocks/probBLOCKS-6-0.pddl --driver sbfws --options "bfws.rs=sim"
```

By default, the production release of the planner is invoked. If you want for instance to use the debug
release, you can add the `--debug` flag.


## Problems with externally-defined symbols
 
If externally-defined symbols are used, the parsing process involves the automatic generation of
some of C++ code that is then compiled and linked against the main planner library to produce an 
actual _domain-dependent_ binary planner which incorporates the provided external definitions.

## Search Modes

The planner is pre-configured with a number of search modes or "drivers" that specify the global search strategy
(search algorithm plus heuristic, if necessary) that will be followed in the search for a plan.
The `--driver` command-line option is thus mandatory; for instance, to use the planner with the `smart` driver we would invoke
`./solver.bin --driver=smart`. The following are some of the available drivers:

<!---
* `lite`: The lite driver is a greedy best-first search which works with the _non-constrained_ RPG heuristics, i.e. computes the _standard_
h_FF and h_MAX heuristics, the only difference being that it can work with the more compact Functional STRIPS encodings, thus not being restricted to
predicative STRIPS.


* `native`: The native driver implements a greedy best-first search coupled with a _constrained_ RPG heuristic that can be
either the h_FF (heuristic=hff) or the h_MAX heuristic (heuristic=hmax). The particularity of this driver is that the CSPs into which
the computation of the heuristic is mapped are not solved by Gecode, but rather by a native, hand-coded simplified approach which might yield some performance gain, since it avoids the overhead of interacting with Gecode. The downside of this approach is that only a certain subset of
FSTRIPS is accepted (namely, that which results in very simple CSPs), and only approximated CSP resolution is used.

* `standard`: A greedy best-first search with one of the two _constrained_ RPG heuristics, which is computed with a 1-CSP-per-ground-action model.
-->

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

<!---
* `unreached_atom`: An experimental driver which performs a greedy best-first search on a version of the constrained RPG heuristics which is computed
in a somewhat different manner that iterates through atoms that have not yet been reached in the RPG, trying to achieve them one by one.
Seems to perform better than other options in some domains, but not in general.
-->


* `iw`: Iterated Width search.

* `sbfws`: A Greedy best-first search with a novelty-based heuristic. Namely, the search favors states with
(1) lower novelty, (2) higher number of satisfied goal atoms, if novelty is equal, and (3) lower accumulated cost, the two
first factors being equal.

* `bfs`: A blind, standard breadth-first search.


