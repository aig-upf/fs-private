import os

import generator

benchmarks = os.getenv("FSBENCHMARKS")

instances = [
    'blocksworld-strips/sample.min.pddl',
    'block-grouping-strips/instance_5_5_2_1.pddl',
]


def test_all_instances():
    assert benchmarks
    for instance in instances:
        filename = os.path.join(benchmarks, 'benchmarks', instance)
        args = ['--tag', 'pytest', '--edebug', '--instance', filename]
        args = generator.parse_arguments(args)

        generator.main(args)
