import argparse
import glob
import os
import sys
import subprocess

def parse_arguments() :
    parser = argparse.ArgumentParser( description = "Invokes generator.py on all pddl files in a given folder")
    parser.add_argument( '--set', required=True, help='The name of the benchmark set')
    parser.add_argument( '--instances', required=True, help ='Path to the folder where instances to be compiled are located')
    parser.add_argument( '--planner', required=False, help='Directory containing planner sources')

    args = parser.parse_args()
    return args

def main() :
    args = parse_arguments()
    command_template = 'python3 generator.py --set {0} --instance {1} --planner {2}'
    if not os.path.exists( args.instances ) :
        print( 'Could not find folder {0}'.format(args.instances), file=sys.stderr)
        print( 'Bailing out!', file=sys.stderr)
        sys.exit(1)

    if not os.path.exists( args.planner ) :
        print('Could not find planner source directory {0}'.format( args.planner), file=sys.stderr)
        print('Bailing out!', file=sys.stderr)
        sys.exit(1)

    # grab pddl files from folder
    instance_files = glob.glob( os.path.join( args.instances, '*.pddl' ) )
    for instance_file in instance_files :
        if 'domain.pddl' in instance_file : continue
        subprocess.call( command_template.format( args.set, instance_file, args.planner ), shell=True)


if __name__ == '__main__' :
    main()
