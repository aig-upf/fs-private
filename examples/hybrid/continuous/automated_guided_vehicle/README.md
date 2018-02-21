# Supervisory Control for the Automated Guided Vehicle

This example illustrates the modelling capabilities of FS hybrid module. A
full description of the supervisory control system can be found on
Chapter 4, Section 2.3 of Lee & Seshia's ["Introduction to Embedded Systems:
A Cyber-Physical Approach"](http://leeseshia.org/releases/LeeSeshia_DigitalV2_2.pdf).
Two versions of the control system are provided:

 - ```domain_sc.pddl``` - this version of the domain includes the simple controller
 proposed by Lee & Seshia implemented by having events whose preconditions correspond
 with the guards of the FSM, and their effects setting the mode for the low-level
 control signals. The decisions available to the planner, at any given t, are whether
 to stop the vehicle or set it on a straight course.
 - ```domain.pddl``` - in this version of the domain the control policy given
 by Lee & Seshia has been removed, and instead the planner can decide whether to
 steer towards either left or right.

 FS will have a hard time producing complete plans for very long tracks. This is
 known and makes evident that computing off-line plans, i.e. switches between modes
 of control signals, is hardly going to be a feasible approach for delivering
 control systems that operate under the constraints posed by so-called large
 volume control systems on CPU time, memory available and robustness over very
 large horizons.

## Components of the example

 - ```data```, ```test```, ```tiny```, ```tiny_2_segments``` - layouts of three
 "test" tracks, each of the tracks is associated  with instance files.
 ```test.pddl```, ```tiny.pddl```, ```tiny_2_segments.pddl```.
 - ```domain.pddl``` and ```domain_sc.pddl``` - domain files with the specifications
 of events/reactions, global constraints and differential constraints.
 - ```external.hxx``` and ```external.cxx``` - ```C++``` implementation of the
 function $e(t)$, function $(@e ?v - vehicle)$, the distance to the track at time t.

## Example command for running the ```AGV``` example

```
 ./run.py --hybrid --disable-static-analysis --instance examples/hybrid/continuous/automated_guided_vehicle/test.pddl --domain examples/hybrid/continuous/automated_guided_vehicle/domain.pddl --driver sbfws --options horizon=1.0,dt=0.5,integrator=runge_kutta_2,bfws.rs=sim,width.force_generic_evaluator=true,bfws.using_feature_set=true,features.project_away_time=true,width.simulation=2
```

and the output should be something similar to

```
********************************************************************************
WARNING! Fixing PYTHONHASHSEED to 1 to obtain more reliable results
********************************************************************************

Problem domain:               automated_guided_vehicle
Problem instance:             test
Working directory:            /home/bowman/Sandboxes/fs-planner/workspace/180222/automated_guided_vehicle/test
Parsing domain description...
Parsing problem description...
Processing domain AST...
Processing instance AST...
Post-Processing...
Domain specifies continuous change...
Setting up 'clock_time' state variable
Setting up 'second_law_thermodynamics' process
Checking metric expression is a grounded functional term...
Checking the goal is a ground formula
Creating FS+ task: Processing objects...
Creating FS+ task: Processing types...
Creating FS+ task: Processing symbols...
Creating FS+ task: Processing state variables...
Creating FS+ task: Processing initial state...
Creating FS+ task: Processing actions...
Creating FS+ task: Processing processes...
Creating FS+ task: Processing events...
Creating FS+ task: Processing the goal...
Creating FS+ task: Processing state constraints...
Creating FS+ task: Processing axioms...
Creating FS+ task: Processing metric...
Compilation command:          python2 /usr/bin/scons

scons: Reading SConscript files ...
scons: done reading SConscript files.
scons: Building targets ...
g++ -o external.o -c -std=c++14 -g -DNDEBUG -Wall -pedantic -O3 -isystem/usr/local/include -isystem/home/bowman/local/include -isystem/home/bowman/local/include -DNDEBUG -DFS_HYBRID -I/home/bowman/Sandboxes/fs-planner/vendor/rapidjson/include -I/home/bowman/Sandboxes/fs-planner/vendor/lapkt-base/src -I/home/bowman/Sandboxes/fs-planner/vendor/lapkt-novelty/src -I/home/bowman/Sandboxes/fs-planner/src -Iinclude -I. external.cxx
g++ -o main.o -c -std=c++14 -g -DNDEBUG -Wall -pedantic -O3 -isystem/usr/local/include -isystem/home/bowman/local/include -isystem/home/bowman/local/include -DNDEBUG -DFS_HYBRID -I/home/bowman/Sandboxes/fs-planner/vendor/rapidjson/include -I/home/bowman/Sandboxes/fs-planner/vendor/lapkt-base/src -I/home/bowman/Sandboxes/fs-planner/vendor/lapkt-novelty/src -I/home/bowman/Sandboxes/fs-planner/src -Iinclude -I. main.cxx
g++ -o solver.bin -lstdc++ -Wl,--no-as-needed external.o main.o -L/home/bowman/Sandboxes/fs-planner/.build -L/usr/local/lib -Llib -lboost_program_options -lboost_serialization -lboost_system -lboost_timer -lboost_chrono -lrt -lboost_filesystem -lm -lgecodesearch -lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport -lsoplex -lgmp -lz -lpthread -lfs
scons: done building targets.
Running solver:               /home/bowman/Sandboxes/fs-planner/workspace/180222/automated_guided_vehicle/test/solver.bin
Command line arguments:       --driver sbfws --options horizon=1.0,dt=0.5,integrator=runge_kutta_2,bfws.rs=sim,width.force_generic_evaluator=true,bfws.using_feature_set=true,features.project_away_time=true,width.simulation=2

Loading problem data
[INFO][ 0.00438] Number of objects: 12
[INFO][ 0.00441] Number of state variables: 15
[INFO][ 0.00442] Number of problem atoms: 8
[INFO][ 0.00444] Number of action schemata: 10
[INFO][ 0.00445] Number of (perhaps partially) ground actions: 0
[INFO][ 0.00446] Number of goal atoms: 5
[INFO][ 0.00447] Number of state constraint atoms: 3
[INFO][ 0.00463] Deriving control to search engine...
[INFO][ 0.00610] Starting search. Results written to .
[INFO][ 0.02984] Expansions: 84
[INFO][ 0.02986] Generations: 84
[INFO][ 0.02986] Evaluations: 0
[INFO][ 0.02987] w_{#g}(n)=1: 16
[INFO][ 0.02988] w_{#g,#r}(n)=1: 19
[INFO][ 0.02989] w_{#g,#r}(n)=2: 8
[INFO][ 0.02989] w_{#g,#r}(n)>2: 41
[INFO][ 0.02990] Expansions with #g decrease: 16
[INFO][ 0.02990] Generations with #g decrease: 16
[INFO][ 0.02991] Total number of simulations: 1
[INFO][ 0.02991] Total simulation time: 0.016000
[INFO][ 0.02992] Total nodes expanded during simulations: 199
[INFO][ 0.02992] Total nodes generated during simulation: 1115
[INFO][ 0.02993] Avg. simulation time: 0.00
[INFO][ 0.02994] Avg. nodes expanded during simulations: 199.00
[INFO][ 0.02994] Avg. nodes generated during simulation: 1115.00
[INFO][ 0.02995] Avg. number of subgoals reached during simulations: 4.00
[INFO][ 0.02996] Simulation nodes reused in the search: 0
[INFO][ 0.02996] Type of R set: 0
[INFO][ 0.02997] Reachable subgoals in initial state: N/A
[INFO][ 0.02998] Max. # reachable subgoals in any simulation: 4
[INFO][ 0.02998] Avg. # reachable subgoals in any simulation: 4.00
[INFO][ 0.02999] |R|_0: N/A
[INFO][ 0.02999] |R|_max: 105
[INFO][ 0.03000] |R|_avg: 105.00
[INFO][ 0.03001] Number of width-1 tables created during simulation: 1
[INFO][ 0.03001] Number of width-2 tables created during simulation: 1
[INFO][ 0.03002] Number of width-1 tables created during search: 13
[INFO][ 0.03002] Number of width-2 tables created during search: 10
[INFO][ 0.03003] Total Planning Time: 0.028 s.
[INFO][ 0.03004] Actual Search Time: 0.02 s.
[INFO][ 0.03008] Peak mem. usage: 70112 kB.
[INFO][ 0.03009] Search Result: No plan was found.
[INFO][ 0.03032] Number of objects: 12
[INFO][ 0.03033] Number of state variables: 15
[INFO][ 0.03033] Number of problem atoms: 8
[INFO][ 0.03034] Number of action schemata: 11
[INFO][ 0.03035] Number of (perhaps partially) ground actions: 16
[INFO][ 0.03035] Number of goal atoms: 5
[INFO][ 0.03036] Number of state constraint atoms: 3
Peak memory: 70112 KB
No plan was found
```
