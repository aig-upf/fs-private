

import os

# read variables from the cache, a user's custom.py file or command line arguments
vars = Variables(['variables.cache', 'custom.py'], ARGUMENTS)
vars.Add(BoolVariable('debug', 'Debug build', 'no'))

default_lapkt_path = os.getenv('LWAPTKDEV', '')
vars.Add(PathVariable('lapkt', 'Path where the LAPKT library is installed', default_lapkt_path, PathVariable.PathIsDir))

env = Environment(variables=vars, ENV=os.environ, CXX='clang')

build_dirname = 'build/debug' if env['debug'] else 'build/prod'
env.VariantDir(build_dirname, '.')

Help(vars.GenerateHelpText(env))

env.Append(CCFLAGS = ['-Wall', '-pedantic', '-std=c++0x' ])  # Flags common to all options

if env['debug']:
    env.Append(CCFLAGS = ['-g', '-DDEBUG' ])
    lib_name = 'fs0-debug'
else:
    env.Append(CCFLAGS = ['-Ofast', '-DNDEBUG' ])
    lib_name = 'fs0'


src_files =  [str(f) for f in Glob('src/*.cxx')]
build_files = [build_dirname + '/' + src for src in src_files]

env.Append(CPPPATH = ['src', env['lapkt'] + '/include'])

shared_lib = env.SharedLibrary('lib/' + lib_name, build_files)
static_lib = env.Library('lib/' + lib_name, build_files)

#Default([static_lib, shared_lib])
Default([static_lib])
