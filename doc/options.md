## FS Planner Options Atlas

### Computation of width

 - ```width.max```: maximum value of width sought (defaults to 1). Used by the variants
 of _IW(k)_ algorithm, where the _k_ value is given by the value of this option.
 - ```width.force_generic_evaluator```: when set to _true_, a heuristic method is used
 to determine which representation to use for variable valuations. This has a massive
 performance impact when the domains of the variables are small (e.g. Boolean).
 - ``` ```

### Features for Width

 - ```features.project_away_time```: the variable representing the timeline *clock_time* is not
 taken by default as a feature to determine the width of a state.
 - ```features.project_away_numeric```: variables of type *float* are not used by default as
 features to determine the width of a state.
 - ```features.joint_goal_error```: uses the goal error signal as a feature to compute width.

### Dynamics

 - ```dynamics.decompose_ode```: This option enables dependency analysis between the ODEs determined to
be active on a given time step, grouping them according to their topological order.

### Reward signal

- ```reward.goal_error```: Reward signal follows from the cost function that measures how dissimilar
is the current stat from any given goal state.

### Algorithm specific
