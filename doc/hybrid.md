# Hybrid Planning Support Docs

## How to run the planner

### Discrete Transitions

```
./run.py --hybrid --t hybrid_test --instance ~/Sandboxes/fs-benchmarks/benchmarks/robot-navigation-num-fn/sample.pddl --driver sbfws --options bfws.rs=sim,bfws.using_feature_set=true,width.force_generic_evaluator=true
```

### Continuous Transitions

```
./run.py --hybrid --instance ~/Sandboxes/mpc-planning-experiments/benchmarks/walkbot/instance_001.pddl --domain ~/Sandboxes/mpc-planning-experiments/benchmarks/walkbot/domain.pddl --driver sbfws --options dt=0.5,integrator=runge_kutta_2,bfws.rs=sim,width.force_generic_evaluator=true,bfws.using_feature_set=true,features.project_away_numeric=true,features.project_away_time=true,features.elliptical_2d=true,width.simulation=2,sim.log=true
```
