#!/usr/bin/env python3
"""ISJ Project 4

This module contains functions for string manipulation and checks.
See docstring of each function for more details.

"""

import itertools

def balanced_paren(parenstr):
    """Check if parenthesis are properly paired in the given string.

    Arguments:
        parenstr -- string to check

    Returns:
        True when parenthesis are paired correctly, False otherwise.
    """
    stack = []
    pushc = "([{<"
    popc = ")]}>"

    for c in parenstr:
        if c in pushc:
            stack.append(c)
        if c in popc:
            # If the stack is empty or the closing parenthesis doesn't
            # correspond to the opening one, return False
            if not len(stack) or popc.index(c) != pushc.index(stack.pop()):
                return False

    return (len(stack) == 0)

def caesar_list(word, key=[1, 2, 3]):
    """Caesar cipher

    Shift each character in the given word by a n places to the right,
    where n is a number from the key list. If the key list is read in
    a circular way if it's not long enough for the given word.

    Arguments:
        word -- word to shift; must contain only lowercase letters
        key -- list of shift values

    Returns:
        Shifted string or a ValueError if the string doesn't contain
        only lowercase letters.
    """
    if not all(ord(char) >= 97 and ord(char) <= 122 for char in word):
        raise ValueError

    key_iter = itertools.cycle(key)
    base = ord('a')
    res = ""

    for c in word:
        # Subtract an ASCII value of 'a' from the ASCII value of the
        # currently processed letter, add a shift offset, apply modulo
        # 26 (as there's 26 lowercase letters) and re-add the ASCII
        # value of 'a' (+- some typecasts).
        res += chr(((ord(c) - base + key_iter.__next__()) % 26) + base)

    return res

def caesar_varnumkey(*args):
    """Caesar cipher with a variable argument list

    This function does the same thing as caesar_list, but instead of
    accepting two arguments (string and key list) it accepts each
    key as another argument.

    E.g. caesar_list("test", [1, 2, 3]) can be translated to
    caesar_varnumkey("test", 1, 2, 3).
    """
    if(len(args) < 1):
        raise IndexError("Not enough arguments")

    if len(args) == 1:
        res = caesar_list(args[0])
    else:
        res = caesar_list(args[0], list(args[1:]))

    return res

## Tests ##
"""
print("PHASE 1")
tests1 = {
    "123" : True,
    "[{}]" : True,
    "12<4<[a]b>>5" : True,
    "{1<2(>3)}" : False,
    "{11}}" : False,
    "{{{{" : False,
    "{<{<{<{<{<{<a>}{}<><<<>>[[[]]]>{}>}>}>}>}>}" : True,
    "}>}]}}>>}" : False
}

for key in tests1:
    print("Test: {}".format(key))
    assert(balanced_paren(key) == tests1[key])

print("\nPHASE 2")
tests2 = {
    "egg" : ["abc", [4, 5]],
    "ace" : ["xyz", [3, 4, 5, 6, 7]],
    "bcd" : ["aaa", [1, 2, 3]],
    "bcd" : ["aaa", None]
}

for key in tests2:
    print("Test: {}".format(tests2[key]))
    if tests2[key][1]:
        res = caesar_list(tests2[key][0], tests2[key][1])
    else:
        res = caesar_list(tests2[key][0])
    print("Result: {}".format(res))
    assert(key == res)

print("\nPHASE 3")
tests3 = {
    "egg" : ["abc", 4, 5],
    "ace" : ["xyz", 3, 4, 5, 6, 7],
    "bcd" : ["aaa", 1, 2, 3],
    "bcd" : ["aaa"]
}

for key in tests3:
    print("Test: {}".format(tests3[key]))
    res = caesar_varnumkey(*tests3[key])
    print("Result: {}".format(res))
    assert(key == res)
"""
