
import os

# Set up a few paths and import the main runner
FS_PYTHON_PATH = os.path.dirname(os.path.abspath(__file__))
FS_PATH = os.path.abspath(os.path.join(FS_PYTHON_PATH, '..'))
FS_WORKSPACE = os.path.join(FS_PATH, 'workspace')
FS_BUILD = os.path.join(FS_PATH, '.build')

from .parser import runner
