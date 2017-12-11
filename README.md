
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

1. [Installation](#installation)
1. [Usage](#usage)
1. [Credits](#credits)
1. [References](#references)



## <a name="references"></a>Installation

The easiest way to use the planner is by [manually compiling the planner source code](doc/installation.md).
Alternatively, you can build and/or use a [ready-to-use image](doc/containers.md) in some of the containerization solutions
that we support.


## <a name="usage"></a>Usage
You can find a high-level overview of the planner usage options [here](doc/usage.md)


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
