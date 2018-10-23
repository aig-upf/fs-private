import os

debug = False
edebug = False

gecode_support = True
ompl_support = False
hybrid_support = True

lapkt = os.getenv('LAPKT', '')

default_compiler = 'g++'

prefix = os.path.join(os.getenv('HOME'), 'local/lib')
