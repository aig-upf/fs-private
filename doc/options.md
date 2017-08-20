## FS Planner Options Atlas

### Features for Width

 - ```features.project_away_time```: the variable representing the timeline *clock_time* is not
 taken by default as a feature to determine the width of a state.
 - ```features.project_away_numeric```: variables of type *float* are not used by default as
 features to determine the width of a state.

### Dynamics

 - ```dynamics.decompose_ode```: This option enables dependency analysis between the ODEs determined to
be active on a given time step, grouping them according to their topological order.
