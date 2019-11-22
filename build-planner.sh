#! /bin/bash -l
### Set name.
#SBATCH --job-name=build-planner
### Redirect stdout and stderr.
#SBATCH --output=slurm.log
#SBATCH --error=slurm.err
### Let later steps append their logs to the output and error files.
#SBATCH --open-mode=append
### Set partition.
#SBATCH --partition=short
### Set quality-of-service group.
#SBATCH --qos=normal
### Set time and memory limit (time in minutes)
#SBATCH --mem-per-cpu=32000M
#SBATCH --time=60
### Number of tasks.
#SBATCH --array=1-1
### Adjustment to priority ([-2147483645, 2147483645]).
#SBATCH --nice=0
### Send mail? Mail type can be e.g. NONE, END, FAIL, ARRAY_TASKS.
#SBATCH --mail-type=NONE
#SBATCH --mail-user=
### Extra options.
### Force the broadwell architecture
#SBATCH --constraint="bdw"

# The following directives will trigger the load of the appropriate GCC and Python versions
module purge
LMOD_DISABLE_SAME_NAME_AUTOSWAP=no module load Boost/1.65.1-foss-2017a-Python-3.6.4
LMOD_DISABLE_SAME_NAME_AUTOSWAP=no module load SCons/3.0.1-foss-2017a-Python-3.6.4
source /homedtic/gfrances/lib/virtualenvs/fs-sdd/bin/activate

export LIBRARY_PATH=$LIBRARY_PATH:${HOME}/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/local/lib

cd "/homedtic/gfrances/projects/code/fs-sdd"
"/homedtic/gfrances/lib/virtualenvs/fs-sdd/bin/python" "./build.py" "-pd"
