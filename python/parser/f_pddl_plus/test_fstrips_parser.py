#!/usr/bin/python3
import argparse
import os

from antlr4 import *
from fstrips.tasks import Task

def parse_arguments() :
    parser = argparse.ArgumentParser(description='Calls parser on domain and instance file')
    parser.add_argument( '--domain', required=True, help='Domain file' )
    parser.add_argument( '--instance', required=True, help ='Instance file')
    args = parser.parse_args()
    if not os.path.exists( args.domain ) :
        print("Could not find domain file '{0}'".format(args.domain))
        sys.exit(1)
    if not os.path.exists( args.instance ) :
        print("Could not open instance file '{1}'".format(args.instance))
    return args

def main():

    args = parse_arguments()


    task = Task.parse( args.domain, args. instance )
    task.dump()

if __name__ == '__main__':
    main()
