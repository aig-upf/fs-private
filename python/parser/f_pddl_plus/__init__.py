from fstrips.tasks import Task

def create_f_pddl_task( domain, instance ) :
    from antlr4_parsers.fstrips.tasks import Task
    task = Task.parse( domain, instance)
    # MRJ: Comment to avoid wanton verbosity
    task.dump()
    return task
