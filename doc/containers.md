
FS planner - Ready-to-use Containers
====================================

The FS planner strives to be compatible with containerization technologies.
At the moment, we support [Docker](https://www.docker.com/) and [Singularity](singularity.lbl.gov/),
but would be glad to extend this support to other containerization solutions should the need arise.


## Singularity

A ready-to-use singularity image can be built from the main `FS` repository as follows:

```
cd $FS_PATH
sudo singularity build fs-planner.img ./Singularity
```

The image can the be used both run on planning problems or as an interactive shell.
To start a container with an interactive shell, you need to:

```
mkdir -p $HOME/tmp/workspace
singularity shell -C -B $DOWNWARD_BENCHMARKS/:/planning/downward-benchmarks:ro -B $HOME/tmp/workspace:/planning/workspace:rw fs-planner.img
```

This assumes that we have a directory `$DOWNWARD_BENCHMARKS` in the host machine,
which will be mapped into the `/fs-planner/downward-benchmarks` directory in the container
(in read-only mode, `ro`).

From within the container, we can now issue the standard planner commands:

```
/planning/fs-planner/run.py -w /planning/workspace -i /planning/downward-benchmarks/blocks/probBLOCKS-4-2.pddl --driver sbfws --options "successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=sim"
```

The standard planner output files can then be found under the `$HOME/tmp/workspace` directory in the host machine.


## Docker
_*These instructions might be outdated and need to be revised*_

The fastest and recommended way to get started with the planner is by grabbing the ready-to-use Docker image and running it interactively.

In order to do so, you need to [have Docker installed on your machine](https://docs.docker.com/engine/installation),
and then you can pull the image with

```shell
docker pull aigupf/fs
```

Once the image is in your computer, you can log into a fully-functioning Docker container where the planner
and all its software dependencies are already installed for you:


```shell
docker run -it aigupf/fs
python3 preprocessor/runner.py --tag test --instance $FSBENCHMARKS/benchmarks/counters-fn/instance_5.pddl --run --driver=smart
```
