
import os

# Import all symbols from the C++ extension
from ._pyfs import *

# from .core import Options

_MODULE_PATH = os.path.dirname(os.path.abspath(__file__))
# BUILD_PATH = os.path.abspath(os.path.join(MODULE_PATH, '..',))

# print("pyfs module successfully imported")

