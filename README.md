
The FS0 Functional STRIPS planner
=================================

`FS0` is a classical planner that accepts a subset of the Functional STRIPS planning language, along with a number
of other extensions such as a limited library of global constraints and the possibility of using
external procedures.

-- Guillem Francès <guillem.frances@upf.edu>

Installation
--------------
In order to run `FS0`, you need to have access to 

1. A [custom version](https://bitbucket.org/gfrances/downward-aig) of the Fast Downward PDDL 3.0 parser (written in Python), modified with the purpose of fully supporting the functional capabilities of the language and allowing for constraints and external procedures to be used on the specification of the domain.

1. A [custom version](https://bitbucket.org/gfrances/lwaptk) of the LWAPTK toolkit, slightly modified to correct some bugs and achieve full integration with our planner.

Once you have cloned both projects, your system needs to be configured with the following environment variables, e.g. by setting them up in your  `~/.bashrc` configuration file:


```

Custom locations
#####################################################################
export LWAPTKDEV="${HOME}/projects/code/aptk/lwaptk-dev"
export FD_AIG_PATH="${HOME}/projects/code/downward/downward-aig"
export FS0_PATH="${HOME}/projects/code/fs0"
#####################################################################

# AIG Fast Downward PDDL Parser
if [[ -d ${FD_AIG_PATH}/src/translate ]]; then
	export PYTHONPATH="${FD_AIG_PATH}/src/translate:$PYTHONPATH"
fi

```

Once all this is set up, you can build the `FS0` library by doing

```
cd $FS0_PATH && scons
```

You can run `scons debug=1` to build de debug version of the library.


Solving planning instances
----------------------------------

The actual process of solving a planning problem involves a preprocessing phase in which a Python script is run to parse a PDDL 3.0 problem specification and generate certain data, as well as a bunch of C++ classes that need to be compiled against the `FS0` main library. The main Python preprocessing script is `$FS0_PATH/preprocessor/generator`. You can bootstrap the whole process by running

```
python3 generator.py --set test --instance $HOME/projects/code/planning-vm-experiments/benchmarks/tests-vpm/fn-simple-sokoban/instance_6.pddl
```

Where `instance` is a PDDL3.0 instance file, and 
`test` is an arbitrary name that will be used to determine the output directory where the executable solver will be left, which in this case will be:

```
$FS0_PATH/generated/test/fn-simple-sokoban/instance_6
```

Thus, we can now execute the solver `solver.bin` on that directory and have the problem instance solved.

