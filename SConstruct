
import os
from util.build import *

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
vars.Add(PathVariable('prefix', 'Path where the FS library is to be installed', os.getenv('FS_PATH',''), PathVariable.PathIsDir))

# The LAPKT path can be optionally specified, otherwise we fetch it from the corresponding environment variable.
vars.Add(PathVariable('lapkt', 'Path where the LAPKT library is installed', os.getenv('LAPKT', ''), PathVariable.PathIsDir))

env = Environment(variables=vars, ENV=os.environ)
env['CXX'] = os.environ.get('CXX', env['default_compiler'])

if env['edebug']:
	build_dirname = '.build/edebug'
elif env['debug']:
	build_dirname = '.build/debug'
else:
	build_dirname = '.build/prod'
env.VariantDir(build_dirname, '.')

Help(vars.GenerateHelpText(env))
vars.Save('variables.cache', env)

# Base include directories
include_paths = ['src']
isystem_paths = []

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
isystem_paths += ['/usr/local/include', os.environ['HOME'] + '/local/include']

# SConscript('modules/core/SConscript', exports="env sources")   #, variant_dir=build_dirname, src_dir='.', duplicate = 0)


# Process modules and external dependencies
sources = []
for flag, modname in modules:
    if flag not in env or env[flag]:  # Import module if not explicitly disallowed
        print("Importing module: \"{}\"".format(modname))
        SConscript('modules/{}/SConscript'.format(modname), exports="env sources")
    else:
        print("Skipping module \"{}\"".format(modname))


env.Append( CPPPATH = [ os.path.abspath(p) for p in include_paths ] )
env.Append( CCFLAGS = [ '-isystem' + os.path.abspath(p) for p in isystem_paths ] )


# Determine all the build files
build_files = [os.path.join(build_dirname, src) for src in sources]
shared_lib = env.SharedLibrary('lib/' + env['fs_libname'], build_files)
#static_lib = env.Library('lib/' + env['fs_libname'], build_files)

# Save a description of the compilation and linking options to be used when linking the final solver
save_pkg_config_descriptor(env, env['fs_libname'], '{}.pc'.format(env['fs_libname']))

Default([shared_lib])
#Default([static_lib, shared_lib])
