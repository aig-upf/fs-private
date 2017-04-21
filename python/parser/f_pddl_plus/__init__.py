from .fstrips.tasks import Task

def parse_f_pddl_plus_task( domain, instance ) :
    from .fstrips.tasks import Task
    task = Task.parse( domain, instance)
    # MRJ: Comment to avoid wanton verbosity
    task.dump()
    return task
