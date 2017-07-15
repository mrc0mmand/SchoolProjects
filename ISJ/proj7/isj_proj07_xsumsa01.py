#!/usr/bin/env python3

import math

class TooManyCallsError(Exception):
    """Function was called too many times"""
    pass

class limit_calls(object):
    """Decorator class, which can be used for limiting calls of a decorated
       function
    """
    def __init__(self, max_calls=2, error_message_tail="called too often"):
        """Initialize the decorator

        Arguments:
            max_calls - maximum allowed calls, default is 2
            error_message_tail - error message for TooManyCallsError exception,
                                 default is "called too often"
        """
        self._calls = 0
        self._max_calls = max_calls
        self._error_message_tail = error_message_tail

    def __call__(self, f):
        """Initialize a wrapper function, which will be called with each call
           of function f

        Arguments:
            f - function to be wrapped

        Wrapping function "wrapped_f" counts calls of the wrapped function f. If
        the call counts exceeds the max_calls count, the TooManyCallsError
        exception is thrown with the specified error message.
        """
        def wrapped_f(*args, **kwargs):
            if self._calls == self._max_calls:
                raise TooManyCallsError("function \"{}\" - {}"
                        .format(f.__name__, self._error_message_tail))

            self._calls += 1
            return f(*args, **kwargs)
        return wrapped_f

def ordered_merge(*args, selector=[]):
    """Generator function, which returns an item from an iterable object specified
    by selector.

    Function accepts an arbitrary number of iterable objects and a selector.
    After each __next__ call, function returns a next item from an object
    defined by an index from the selector.

    Arguments:
        *args - an arbitrary number of iterable objects
        selector - list of indexes

    Returns:
        Next item from x-th iterable object, where x is selected from the selector.
        If the selector is empty, an empty list is returned.
    """
    if not selector:
        return []

    indexes = [0] * len(selector)

    for idx in selector:
        yield args[idx][indexes[idx]]
        indexes[idx] += 1

class Log(object):
    """Logging facility, which wraps the final log file with "Begin" and "End"
    messages.
    """
    def __init__(self, filename):
        """Initialize the logging facility"""
        self._filename = filename

    def __enter__(self):
        """When used with "with" keyword, this functions open the specified log
        file and logs the initial "Begin" message
        """
        self._f = open(self._filename, "w")
        self._f.write("Begin\n")
        return self

    def logging(self, message):
        """Log a message specified by the message argument"""
        self._f.write(message + "\n")

    def __exit__(self, type, value, traceback):
        """When the facility is destroyed, this function writes the ending
        message to the log file ("End") and closes the log file
        """
        self._f.write("End\n")
        self._f.close()

## TESTS ##

#@limit_calls(1, 'that is too much')
#def pyth(a,b):
#    c = math.sqrt(a**2 + b ** 2)
#    return c
#
#print(pyth(3,4))
#print(pyth(6,8))

#print(list(ordered_merge('abcde', [1, 2, 3], (3.0, 3.14, 3.141), range(11, 44, 11), selector = [2,3,0,1,3,1])))

#with Log('mylog.txt') as logfile:
#    logfile.logging('Test1')
#    logfile.logging('Test2')
#    a = 1/0
#    logfile.logging('Test3')
