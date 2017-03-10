"""
 This test suite performs an end-to-end compilation for a number of selected instances.
 Expensive, but necessary.
"""

import os
import sys

# Import FS python module
FS_PATH = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, FS_PATH)

from testhelper import FSTester, run_planner


tester = FSTester()


def pytest_generate_tests(metafunc):
    # This is called once for each test method, and allows us to pass different parameters to the test method,
    # in this case as many instances as desired, so that each instance is mapped into a different test.
    # @see http://pytest.org/latest/example/parametrize.html#parametrizing-test-methods-through-per-class-configuration
    # if metafunc.function != test_runner:
    #     return
    tester.parametrize(metafunc)


def test_sbfws(instance, options):
    run_planner(instance, driver='sbfws', options=options)
