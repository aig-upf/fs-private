"""
"""
from compilation.component_processor import BaseComponentProcessor


class FormulaProcessor(BaseComponentProcessor):
    def __init__(self, task, formula):
        super().__init__(task)
        self.formula = formula

    def init_data(self):
        return dict(conditions=[])

    def process(self):
        self.process_conditions(self.formula)
        return self.data

    def get_parameters(self):
        return dict()
