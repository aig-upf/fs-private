
Some info on current branches:

* `fs`: The main `FS` planner stable branch.
* `delayed`: Some quick&dirty refactoring to allow the necessary changes for the delayed evaluation version of LAPKT to compile.
* `fs-private`: The main `FS` planner experimental branch, with the latest, possibly untested and non-compiling, additions. This branch is private and should not make it to the public repository.
* `extensional`: Private branch, refactoring to replace the current CSP model, which involves element constraints for handling existential variables and nested fluents, with an alternative model that uses only extensional (aka "table") constraint, much more in line with the descriptions on the last papers.
* `extensional_plus`: Private branch, created from `extensional` at the point where I started heavily modifying the C++ code (as opposed to the previous commits, which where focused on the Python preprocessor)
* `atom_iteration`: Private branch, created from `extensional_plus` at the point where I started modifying the code to use a "unreached atom per unreached atom" strategy to build the RPG, as opposed to the previous "action per action" strategy.
* `pddl_vars`: Private branch, created from `atom_iteration` at the point where we start using an approach reminiscent of the PDDL `:vars` construct; in which 
we no longer need to ground actions but rather from each lifted action effect create a CSP and post a "novelty" constraint (different to the previous novelty constraints)
that enforces that each of the solutions of the CSP produces a novel atom.
