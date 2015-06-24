import pp
import logging
import sys
import argparse
import glob
import os

def run_task( benchmark, domain, instance, folder, timeout ) :
    import benchmark
    import os
    import json

    current_dir = os.getcwd()
    os.chdir( folder )
    log = 'trace.log'
    command = './nfs-planner.bin'
    rv, time = benchmark.run( command, timeout, 4096, benchmark.Log(log) )
    command_info = {}
    command_info['exit_code'] = rv
    command_info['wall_time'] = time

    if rv == 0 :
        if os.path.exists( 'search.json ') :
            with open( 'search.json') as input :
                data = json.loads( input.read() )
                for k, v in data :
                    command_info[k] = v

    os.chdir( current_dir )
    data_folder = 'data'
    data_folder = os.path.join( data, domain )
    if not os.path.exists( data_folder ) :
        os.makedirs(data_folder)

    info_filename  = os.path.join(data_folder,'{0}.json'.format(instance))
    if not os.path.exists( info_filename ) :
        info = {}
        info['domain'] = domain
        info['instance'] = instance
        info['planners_data'] = {}
    else :
        info = json.loads( info_filename )
    info['planners_data'][command] = command_info

    with open ( info_filename, 'w' ) as out:
        print( json.dumps( info, separators=(',',':'), indent=4, sort_keys = True), file=out)

def main() :

    parser = argparse.ArgumentParser( description = "Driver script for running planner(s) over benchmarks")
    parser.add_argument( '--benchmark', required=True, help='Path to benchmarks')
    parser.add_argument( '--domain', required=False, help='Name of domain to be tested')
    parser.add_argument( '--timeout', required=False, help='Planner time out (defaults to 1800 secs)')
    parser.add_argument( '--cpus', required=False, help='Number of cpus to be used (defaults to 2)')
    parser.add_argument( '--secret', required=True, help='Server password')

    args = parser.parse_args()

    if not os.path.exists( args.benchmark ) :
        print ( 'Could not find directory {0}'.format(args.benchmark), file=sys.stderr )
        sys.exit(1)

    if args.timeout is None :
        args.timeout = 1800

    tasks = []
    if args.domain is not None :
        domain_dirs = [ os.path.join( args.benchmark, args.domain ) ]
    else :
        domain_dirs = glob.glob( os.path.join( args.benchmark, '*'))
        domain_dirs = [ dom for dom in domain_dirs if os.path.isdir(dom) ]

    for domain_dir in domain_dirs :
        if not os.path.exists( domain_dir ) :
            print( 'Could not find domain directory {0}'.format( domain_dir ), file=sys.stderr)
            sys.exit(1)
        instances = glob.glob( os.path.join( domain_dir, '*') )
        instances = [ inst for inst in instances if os.path.isdir( inst ) ]
        for inst in instances :
            tasks.append( (os.path.basename(args.benchmark), os.path.basename(domain_dir), os.path.basename(inst), inst, args.timeout) )

    num_cpus = int(args.cpus)
    ppservers = ()
    jobserver = pp.Server( num_cpus, ppservers=ppservers, socket_timeout=2, secret=args.secret)
    print ('Tasks to be executed: {0}'.format(len(tasks)))
    jobs = [  (task, job_server.submit( run_task, (task), (), ())) for task in tasks ]

    for input, job in jobs :
        print( 'Output of {0} is {1}'.format( input, job() ) )

    job_server.print_stats()

if __name__ == '__main__' :
    main()
