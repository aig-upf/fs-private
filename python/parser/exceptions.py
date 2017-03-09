"""
 Some exception classes.
"""


class UnimplementedFeature(Exception):
    pass


class ParseException(Exception):
    pass


class UndeclaredSymbol(ParseException):
    pass


class TypeException(Exception):
    pass
