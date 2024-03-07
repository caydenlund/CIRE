# Test result codes.

class ResultCode(object):
    """Test result codes."""

    # We override __new__ and __getnewargs__ to ensure that pickling still
    # provides unique ResultCode objects in any particular instance.
    _instances = {}
    def __new__(cls, name, isFailure):
        res = cls._instances.get(name)
        if res is None:
            cls._instances[name] = res = super(ResultCode, cls).__new__(cls)
        return res
    def __getnewargs__(self):
        return (self.name, self.isFailure)

    def __init__(self, name, isFailure):
        self.name = name
        self.isFailure = isFailure

    def __repr__(self):
        return '%s%r' % (self.__class__.__name__,
                         (self.name, self.isFailure))

PASS        = ResultCode('PASS', False)
FLAKYPASS   = ResultCode('FLAKYPASS', False)
XFAIL       = ResultCode('XFAIL', False)
FAIL        = ResultCode('FAIL', True)
XPASS       = ResultCode('XPASS', True)
UNRESOLVED  = ResultCode('UNRESOLVED', True)
UNSUPPORTED = ResultCode('UNSUPPORTED', False)
TIMEOUT     = ResultCode('TIMEOUT', True)