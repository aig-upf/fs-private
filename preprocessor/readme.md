
# Preprocessor

The `preprocessor` directory contains Python code responsible
to map any given pair of PDDL instance and domain files into a representation
which is suitable for the `FS` planner. This representation is mostly based
on a number of json-encoded data structures that define the particular PDDL instance,
along with (possibly) certain automatically-generated C++ code, when
externally- defined symbols are used.

The `tests` directory also contain a number of `pytest` tests that can be run
with `py.test -v tests`.

