# Hybrid Planning Support Docs

## How to compile

### Pre-requisites

Some of the components of the FS module supporting hybrid planning require
the open-source LP solver [soplex](http://soplex.zib.de/). The sources for
the version of ```soplex``` we use in the planner can be accessed from the
public github repo ```aig-upf/soplex```.

Once the repo has been downloaded or cloned, follow the instructions in
the Readme.md file provided in the root directory of ```aig-upf/soplex```.

### FS Build process customization

In order to setup FS build system so the planner is built with **full** support
for hybrid and numeric it is necessary to edit the ```Python``` module ```custom.py```,
and change the value of the variable ```hybrid_support``` from ```False``` to ```True```.

## How to run the planner

FS doesn't make a difference *per se* between hybrid and numeric planning, but
in order to activate the language and compilation features that allow FS to
handle hybrid and numeric planning problems, the switch  ```hybrid``` needs to
be specified from the command line.

### Discrete Transitions

We have provided a sample domain in the folder ```examples/hybrid/discrete```
that showcases the language available to modellers.

```
./run.py --hybrid --t hybrid_test --instance examples/hybrid/discrete/robot-navigation-num-fn/sample.pddl --driver sbfws --options bfws.rs=sim,bfws.using_feature_set=true,width.force_generic_evaluator=true
```

### Continuous Transitions

Options that affect hybrid planning with continuous time dynamics:

 - ```dt```: the discretization step
 - ```integrator```: which is the numerical method used to calculate the
 evolution of dynamics over the discretisation step

The following command also exposes options available to set what features are
to be used to determine the width of states:

```
./run.py --hybrid --instance examples/hybrid/continuous/walkbot/instance_001.pddl --domain examples/hybrid/continuous/walkbot/domain.pddl --driver sbfws --options dt=0.5,integrator=runge_kutta_2,bfws.rs=sim,width.force_generic_evaluator=true,bfws.using_feature_set=true,features.project_away_numeric=true,features.project_away_time=true,features.elliptical_2d=true,width.simulation=2,sim.log=true
```

The interpretation of plans with continuous transitions is not straightforward,
as the plans, along with the dynamics of the system, result in a hybrid system
whose evolution needs to be simulated by means of numeric integration. The ```FS```
planner, when the parameters ```sim.log=true``` is specified, simulates the plans
producing detailed predictions of state values and timings of instantaneous
control actions and activation of exogenous events and reactions. This
output can be find on a JSON document called ```plan.simulation.json```, which
can be found on the folder

```
workspaces/<yymmdd>/<domain>/<instance>
```

where:

 - ```yymmdd``` is the current system date,
 - ```domain``` is the name of the domain as given in the PDDL specification of
  the planning domain,
 - ```instance``` is the name of the instance as given in the PDDL specification
 of initial states, goals and cost functions,

For the command above, would be ```workspaces/180222/walkbot/instance_001```.

The JSON document ```plan.simulation.json``` contains the following attributes:

 - domain, instance: nombre del dominio y la instance
 - duration: simulated timestamp for the goal condition being reached
 - control_events: timestamped list of instantaneous control actions
 - exo_events: timestamped list of events (reactions)
 - trajectory: sequence of states, each state an assignment of values over state variables

Along with the simulation of the plan found, ```FS``` generates very diverse
and sometimes exhaustive information on grounding, search, and other internal
processes invoked as plans are executed.
