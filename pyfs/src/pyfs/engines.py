
from . import extension as cext


def breadth_first_search(model):
    return cext.create_breadth_first_search_engine(model)
