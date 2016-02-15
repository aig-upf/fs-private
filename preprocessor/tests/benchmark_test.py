"""
 This test suites performs an across-the-board compilation for a number of selected instances.
 Expensive, but necessary.
"""

import os
import generator


benchmarks = os.getenv("FSBENCHMARKS")

instances = [
    'blocksworld-strips/sample.min.pddl',
    'block-grouping-strips/instance_5_5_2_1.pddl',
    'counters-strips-ex/instance_5.pddl',
    'counters-fn/instance_5.pddl',
    'n-puzzle-fn-v1-nested/eight01x.pddl',
    'blocksworld-fn-nested/instance_5_1.pddl',
    'sokoban-sat11-strips/p01.pddl',
    'visitall-sat11-strips/problem12.pddl',
    'graph-coloring-agent-fn/instance_5_4_1.8_1.pddl'
]


def pytest_generate_tests(metafunc):
    # This is called once for each test method, and allows us to pass different parameters to the test method,
    # in this case as many instances as desired, so that each instance is mapped into a different test.
    # @see http://pytest.org/latest/example/parametrize.html#parametrizing-test-methods-through-per-class-configuration
    if metafunc.function != test_instance:
        return
    argnames = ['instance']
    argvalues = [[inst] for inst in instances]
    metafunc.parametrize(argnames, argvalues)


def test_instance(instance):
    """
    Run the whole preprocessor on the given problem instance.
    :param instance: The planning problem instance
    :return:
    """
    assert benchmarks
    filename = os.path.join(benchmarks, 'benchmarks', instance)
    args = ['--tag', 'pytest', '--edebug', '--instance', filename]
    args = generator.parse_arguments(args)
    generator.main(args)
