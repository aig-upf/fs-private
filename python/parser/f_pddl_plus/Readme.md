# ANTLR 4 Parser for Functional Strips


## Installing the thing


Instructions for installing ANTLRv4 retrieved from https://theantlrguy.atlassian.net/wiki/display/ANTLR4/Getting+Started+with+ANTLR+v4

* Install Java (version 1.6 or higher)
* Download

$ cd /usr/local/lib
$ curl -O http://www.antlr.org/download/antlr-4.5-complete.jar

Or just download in browser from website:
    http://www.antlr.org/download.html
and put it somewhere rational like /usr/local/lib.

* Add antlr-4.5-complete.jar to your CLASSPATH:

$ export CLASSPATH=".:/usr/local/lib/antlr-4.5-complete.jar:$CLASSPATH"

It's also a good idea to put this in your .bash_profile or whatever your startup script is.

* Create aliases for the ANTLR Tool, and TestRig.

$ alias antlr4='java -Xmx500M -cp "/usr/local/lib/antlr-4.5-complete.jar:$CLASSPATH" org.antlr.v4.Tool'
$ alias grun='java org.antlr.v4.runtime.misc.TestRig'

## Python Support

Install with:

$ pip install antlr4-python2-runtime

for Python 2.7, if you are using Python 3 then use:

$ pip3 install antlr4-python3-runtime

Or, you can download and untar the appropriate package from:

https://pypi.python.org/pypi/antlr4-python2-runtime

https://pypi.python.org/pypi/antlr4-python3-runtime

The runtimes are provided in the form of source code, so no additional installation is required.

See [Python runtime targets](https://theantlrguy.atlassian.net/wiki/display/ANTLR4/Python+Target) for more information.

In order to build the parsers, get into the antlr4_parsers folder and
issue the command:

$ ./build_parser.py --grammar FunctionalStrips.g4 --deploy fstrips

which will compile the grammar and generate the corresponding Python classes inside
the module fstrips

##TODO List

 - Ensure that the "direct" inference backend is not compatible with domains that require the type "number"

##DONE List

- Introduce code to support normalization of ArithmeticTerms (and Relational expressions). The normal
form we're targetting is
 - Move multiplication inwards
 - All relational expressions are compare with 0, that is, where one has x + y + 1 > z, you move into x + y + (-1)z + 1 > 0, etc.

- Make explicit that FunctionalTerms have now a type, this affects the handling of arithmetic expressions
(class ArithmeticTerm in src/languages/fstrips/builtin.hxx ).
 - Work on the back-end:
   - Arithmetic Terms refactor:
     - Term "bounds": what to do with them when they are not defined?

- Implement appropiate extensions to the Gecode translation framework to handle float variables
and constraints.
 - Revise component_translator.hxx/cxx
 - Revise csp_translator.hxx/cxx

- Make explicit that FunctionalTerms have now a type, this affects the handling of arithmetic expressions
(class ArithmeticTerm in src/languages/fstrips/builtin.hxx ).
 - Front-end:
   - Typing is now enforced in a more consistent manner for Arithmetic and Relational expressions
   - Added field args_type to relational atoms involving numeric expressions
 - Work on the back-end:
   - Arithmetic Terms refactor:
     - Added attribute value_type to ArithmeticTerm to distinguish the type the term needs to be evaluated to
     - Changed implementation of interpret() methods for subclasses of ArithmeticTerm to be sensitive to
       the new types
   - Relational Terms refactoring:
     - Propagate correctly what are the types lhs and rhs need to be coerced to in order
       to compare the correct interpretation

  - Introduce mechanisms to pack and unpack floats in the ObjectIdx
    - ObjectIdx promoted from a typedef into a full-blown class
    - ObjectIdx uses type-punning (without violating strict aliasing rules) to become an integer or a float upon request
    - Refactor ProblemInfo/ObjectType to accomodate the number type


- Remove the constraints on NumericConstraint, where it is required that it is an integral value

- Handle implicit type coercion: i.e. the expression x + 1 has type 'int' when x is an int, and has
type 'float' when x is a float.
 - DONE: the front-end will abort any attempts to mix up integral and float types

- Upgrade the grammar from ANTLR v3 to v4 (see [here](https://github.com/antlr/antlr4/issues/464) for directions
on how to proceed).
  - Done!

- Study how does FS0 use Malte's PDDL parser

  - Done!

- Add support for language components:

  - Actions

  - Goals

  - Initial States

  - State Constraints

  - Bounds Constraints

    - Almost done: current implementation doesn't take advantage of access
    to the structure of the int[lb..ub] construct.
