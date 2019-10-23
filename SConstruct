
import os
from build.scons.util import *

# read variables from the cache, a user's custom.py file or command line arguments
vars = Variables(['variables.cache', 'custom.py'], ARGUMENTS)
vars.Add(BoolVariable('debug', 'Debug build', 'no'))
vars.Add(BoolVariable('edebug', 'Extreme debug', 'no'))

# Planner modules:
vars.Add(BoolVariable('ompl_support', 'Include OMPL driver support in library', 'yes'))
vars.Add(BoolVariable('gecode_support', 'Include Gecode solver dependencies', 'yes'))
vars.Add(BoolVariable('hybrid_support', 'Include Soplex LP solver adapter and dependencies', 'yes'))

vars.Add(EnumVariable('default_compiler', 'Preferred compiler', 'clang++', allowed_values=('g++', 'clang++')))
vars.Add(PathVariable('fs', 'Path to FS sources', os.getcwd(), PathVariable.PathIsDir))
vars.Add(PathVariable('prefix', 'Path where the FS library is to be installed', '.', PathVariable.PathIsDir))

env = Environment(variables=vars, ENV=os.environ)
env['CXX'] = os.environ.get('CXX', env['default_compiler'])


# Set up some directories
env['fs_src'] = os.path.join(env['fs'], 'src')
env['fs_vendor'] = os.path.join(env['fs'], 'vendor')
env['build_basename'] = '.build'


if env['edebug']:
    build_suffix = 'edebug'
    exe_name = 'solver.edebug.bin'
elif env['debug']:
    build_suffix = 'debug'
    exe_name = 'solver.debug.bin'
else:
    build_suffix = 'prod'
    exe_name = 'solver.bin'

build_dirname = os.path.join(env['build_basename'], build_suffix)
env.VariantDir(build_dirname, '.')

Help(vars.GenerateHelpText(env))
vars.Save('variables.cache', env)

# Possible modules
# Compilation flag, module name, use-by-default?
modules = [
    ("core",           "core"),
    ("lapkt2",         "lapkt2"),
    ("gecode_support", "gecode"),
    ("hybrid_support", "hybrid"),
    ("ompl_support",   "ompl")
]

# include local by default # MRJ: This probably should be acquired from an environment variable
include_paths = ['src']
isystem_paths = ['/usr/local/include', os.path.expanduser('~/local/include')]


# Process modules and external dependencies
sources = []
for flag, modname in modules:
    if flag not in env or env[flag]:  # Import module if not explicitly disallowed
        print("Importing module: \"{}\"".format(modname))
        SConscript('build/scons/{}.sconscript'.format(modname), exports="env sources")
    else:
        print("Skipping module \"{}\"".format(modname))

# Add main() binary
include_paths += ["./planners/generic/"]
sources += ["./planners/generic/main.cxx"]

env.Append( CPPPATH = [ os.path.abspath(p) for p in include_paths ] )
env.Append( CCFLAGS = [ '-isystem' + os.path.abspath(p) for p in isystem_paths ] )


# Determine all the build files
build_files = [os.path.join(build_dirname, src) for src in sources]
#shared_lib = env.SharedLibrary(os.path.join(env['build_basename'], env['fs_libname']), build_files)
#static_lib = env.Library(os.path.join(env['build_basename'], env['fs_libname']), build_files)
#Default([shared_lib])
#Default([static_lib])
env.Program(exe_name, build_files)
