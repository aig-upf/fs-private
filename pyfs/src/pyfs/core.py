


class Options(object):
    # def __init__(self):
        # pass
        # self._options = fd._Options(False)

    # def set(self, attr, value):
    #     self._options.set(attr, value)

    @classmethod
    def from_dict(cls, dictionary):
        """ Create an options object from a given dictionary """
        result = fd._Options(False)
        for k, v in dictionary.items():
            result.set(k, v)
        return result
