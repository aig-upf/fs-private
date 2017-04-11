#!/usr/bin/python3

"""
    A helper build script - Builds the production library and vanilla solver
"""
import argparse
import glob
import os
import shutil
import subprocess
import sys
import multiprocessing

def parse_arguments(args):
	parser = argparse.ArgumentParser(description='FS Planner Build Script')
	parser.add_argument('--all', action='store_true', help="Flag to compile all modes.")
	parser.add_argument('--debug', action='store_true', help="Flag to compile in debug mode.")
	parser.add_argument('--edebug', action='store_true', help="Flag to compile in extreme debug mode.")
	return parser.parse_args(args)

def single_build(directory, command):
	# First build the LAPKT production library
	#lapkt_dir = os.environ['LAPKT2_PATH']
	#print('\n')
	#print("Building LAPKT library...")
	#sys.stdout.flush()
	#output = subprocess.call(command.split(), cwd=lapkt_dir)
	#if output:
		#sys.exit(output)

	
	# Build the FS planner production library
	print('\n')
	print("Building FS library...")
	sys.stdout.flush()
	output = subprocess.call(command.split(), cwd=directory)
	if output:
		sys.exit(output)	
	
	
    # Build the vanilla generic FS planner
	generic_planner_dir = os.path.join(directory, 'planners', 'generic')
	print('\n')
	print("Building FS vanilla planner...")
	sys.stdout.flush()
	output = subprocess.call(command.split(), cwd=generic_planner_dir)
	if output:
		sys.exit(output)

	print('\n')

def get_command(cpus, debug):
	return 'scons -j {} {}'.format(cpus, debug)
	
def main(args):
	current_dir = os.path.dirname(os.path.abspath(__file__))
	cpus = min(5, multiprocessing.cpu_count())
	print("Starting build process on directory '{}' with {} CPUs.".format(current_dir, cpus))
	
	if args.all:  # We launch a build for each possible debug config
		for debug_flag in ["edebug=1", "debug=1", ""]:
			single_build(current_dir, get_command(cpus, debug_flag))
		
	else:
		debug_flag = "edebug=1" if args.edebug else ("debug=1" if args.debug else "")
		single_build(current_dir, get_command(cpus, debug_flag))

if __name__ == "__main__":
	main(parse_arguments(sys.argv[1:]))
