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
	parser.add_argument('--debug', action='store_true', help="Flag to compile in debug mode.")
	parser.add_argument('--edebug', action='store_true', help="Flag to compile in extreme debug mode.")
	return parser.parse_args(args)

def main(args):
	
	current_dir = os.path.dirname(os.path.abspath(__file__))
	cpus = multiprocessing.cpu_count() - 1
	debug_flag = "edebug=1" if args.edebug else ("debug=1" if args.debug else "")
	scons_command = 'scons -j {} {}'.format(cpus, debug_flag)
	
	print("Starting build process on directory '{}' with {} CPUs.".format(current_dir, cpus))
	
	# First build the LAPKT production library
	lapkt_dir = os.environ['LAPKT2_PATH']
	print('\n')
	print("Building LAPKT production library...")
	sys.stdout.flush()
	output = subprocess.call(scons_command.split(), cwd=lapkt_dir)

	
	# Build the FS planner production library
	print('\n')
	print("Building FS production library...")
	sys.stdout.flush()
	output = subprocess.call(scons_command.split(), cwd=current_dir)
	
	
    # Build the vanilla generic FS planner
	generic_planner_dir = os.path.join(current_dir, 'planners', 'generic')
	print('\n')
	print("Building FS vanilla planner...")
	sys.stdout.flush()
	output = subprocess.call(scons_command.split(), cwd=generic_planner_dir)
	print('\n')


if __name__ == "__main__":
	main(parse_arguments(sys.argv[1:]))
