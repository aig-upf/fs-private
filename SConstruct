
import fnmatch
import os

CLINGO = False

# read variables from the cache, a user's custom.py file or command line arguments
vars = Variables(['variables.cache', 'custom.py'], ARGUMENTS)
vars.Add(BoolVariable('debug', 'Debug build', 'no'))
vars.Add(BoolVariable('edebug', 'Extreme debug', 'no'))

# The LAPKT path can be optionally specified, otherwise we fetch it from the corresponding environment variable.
vars.Add(PathVariable('lapkt', 'Path where the LAPKT library is installed', os.getenv('LAPKT', ''), PathVariable.PathIsDir))

def which(program):
	""" Helper function emulating unix 'which' command """
	for path in os.environ["PATH"].split(os.pathsep):
		path = path.strip('"')
		exe_file = os.path.join(path, program)
		if os.path.isfile(exe_file) and os.access(exe_file, os.X_OK):
			return exe_file
	return None

def locate_source_files(base_dir, pattern):
	matches = []
	for root, dirnames, filenames in os.walk(base_dir):
		for filename in fnmatch.filter(filenames, pattern):
			matches.append(os.path.join(root, filename))
	return matches

# Read the preferred compiler from the environment - if none specified, choose CLANG if possible
default_compiler = 'clang++' if which("clang++") else 'g++'
gcc = os.environ.get('CXX', default_compiler)

env = Environment(variables=vars, ENV=os.environ, CXX=gcc)

if env['edebug']:
	build_dirname = 'build/edebug'
elif env['debug']:
	build_dirname = 'build/debug'
else:
	build_dirname = 'build/prod'
env.VariantDir(build_dirname, '.')

Help(vars.GenerateHelpText(env))

env.Append(CCFLAGS = ['-Wall', '-pedantic', '-std=c++11' ])  # Flags common to all options
env.Append(CCFLAGS = ['-Wno-deprecated-register' ]) # Get rid of annoying warning message from the Jenkins library
	

# Extreme debug implies normal debug as well
if env['debug'] or env['edebug']:
	env.Append(CCFLAGS = ['-g', '-DDEBUG' ])
	lib_name = 'fs-debug'
else:
	env.Append(CCFLAGS = ['-g', '-O3', '-DNDEBUG' ])
	lib_name = 'fs'

# Additionally, extreme debug implies a different name plus extra compilation flags
if env['edebug']:
	env.Append(CCFLAGS = ['-DEDEBUG'])
	lib_name = 'fs-edebug'


# Base include directories
include_paths = ['src', os.path.join(env['lapkt'], 'include')]
isystem_paths = []
	
# Gecode tweaks
isystem_paths += ['/usr/local/include'] # MRJ: This probably should be acquired from an environment variable
isystem_paths += [os.environ['HOME'] + '/local/include']

sources = locate_source_files('src', '*.cxx')

clingo_path = os.getenv('CLINGO_PATH', '')
if CLINGO:
	if not clingo_path:
		raise RuntimeError("You need to specify the path where Clingo is installed in the environment variable CLINGO_PATH")
	# Clingo paths
	isystem_paths += [clingo_path + '/' + subdir for subdir in ["libclasp", "libprogram_opts", "libclingo", "libgringo"]]
	env.Append( CCFLAGS = [ '-DWITH_THREADS=0'] )  # Needed by Clingo
else:
	forbidden = ['src/search/drivers/asp_engine.cxx', 'src/utils/printers/asp.cxx']
	sources = [s for s in sources if (not s.startswith('src/asp') and s not in forbidden)]  # Filter out clingo-specific sources

env.Append( CPPPATH = [ os.path.abspath(p) for p in include_paths ] )
env.Append( CCFLAGS = [ '-isystem' + os.path.abspath(p) for p in isystem_paths ] )

# Determine all the build files
build_files = [build_dirname + '/' + src for src in sources]

shared_lib = env.SharedLibrary('lib/' + lib_name, build_files)
#static_lib = env.Library('lib/' + lib_name, build_files)

Default([shared_lib])
#Default([static_lib, shared_lib])
