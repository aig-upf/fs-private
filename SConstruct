
import os

# read variables from the cache, a user's custom.py file or command line arguments
vars = Variables(['variables.cache', 'custom.py'], ARGUMENTS)
vars.Add(BoolVariable('debug', 'Debug build', 'no'))
vars.Add(BoolVariable('fdebug', 'Debug FS0', 'no'))

# The LAPKT path can be optionally specified, otherwise we fetch it from the corresponding environment variable.
vars.Add(PathVariable('lapkt', 'Path where the LAPKT library is installed', os.getenv('LAPKT_PATH', ''), PathVariable.PathIsDir))

def which(program):
	""" Helper function emulating unix 'which' command """
	for path in os.environ["PATH"].split(os.pathsep):
		path = path.strip('"')
		exe_file = os.path.join(path, program)
		if os.path.isfile(exe_file) and os.access(exe_file, os.X_OK):
			return exe_file
	return None

env = Environment(variables=vars, ENV=os.environ, CXX='clang' if which('clang') else 'g++')

build_dirname = 'build/debug' if env['debug'] else 'build/prod'
env.VariantDir(build_dirname, '.')

Help(vars.GenerateHelpText(env))

env.Append(CCFLAGS = ['-Wall', '-pedantic', '-std=c++11' ])  # Flags common to all options

if env['debug']:
	env.Append(CCFLAGS = ['-g', '-DDEBUG' ])
	if env['fdebug']:
		env.Append(CCFLAGS = ['-DFS0_DEBUG'])
	lib_name = 'fs0-debug'
else:
	env.Append(CCFLAGS = ['-Ofast', '-DNDEBUG' ])
	lib_name = 'fs0'

source_dirs = ['src', 'src/constraints', 'src/utils', 'src/heuristics']
src_files = []
for d in source_dirs:
	src_files += [str(f) for f in Glob(d + '/*.cxx')]
         
# Source dependencies from LAPKT search interfaces
Export('env')
aptk_search_interface_objs = SConscript( os.path.join( env['lapkt'], 'aptk2/search/interfaces/SConscript' ) )
aptk_heuristics_objs = SConscript(  os.path.join( env['lapkt'], 'aptk2/heuristics/interfaces/SConscript' ) ) 
aptk_heuristics_objs += SConscript( os.path.join( env['lapkt'], 'aptk2/heuristics/novelty/SConscript' ) )
aptk_tools_objs = SConscript( os.path.join( env['lapkt'], 'aptk2/tools/SConscript' ) )
  
build_files = [build_dirname + '/' + src for src in src_files]
build_files += aptk_search_interface_objs
build_files += aptk_tools_objs
build_files += aptk_heuristics_objs

env.Append(CPPPATH = ['src', env['lapkt']])

shared_lib = env.SharedLibrary('lib/' + lib_name, build_files)
static_lib = env.Library('lib/' + lib_name, build_files)

#Default([static_lib, shared_lib])
Default([static_lib])


