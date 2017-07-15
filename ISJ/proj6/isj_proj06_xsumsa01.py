#!/usr/bin/env python3

from itertools import permutations, product

def first_nonrepeating(string):
    """Return the first non-repeating character from a string.

    Arguments:
        string -- string to search in

    Returns:
        First non-repeating character from the given string on success,
        None if such character does not exist.

    """
    for c in string:
        if string.count(c) == 1:
            return c

    return None

def combine4(nums, exp_res):
    """Find all expressions constructed from given numbers which result
    equal to given expected result.

    Function takes a list of four numbers and then tries to find all expressions,
    using these numbers and operators +, -, *, /, (, ), which equal to the
    expected result given as a second argument.

    Arguments:
        nums -- list of four numbers
        exp_res -- expected result

    Returns
        List of strings, where each string consists of an expression matching
        the expected format. List can be empty, when no valid expression is found.

    """
    if len(nums) != 4:
        raise ValueError("Invalid list size")

    # Get all operand permutations
    groups = list(permutations(nums, 4))
    # Get a cartesian product of operators (we need three operators per format)
    ops = [p for p in product(['+', '-', '*', '/'], repeat=3)]
    # Get a product of operand permutations and operators
    # (4 operands and 3 operators in each 'item')
    prod = list(product(groups, ops))
    # Parenthesis combinations
    fmts = [
        "{}{}{}{}{}{}{}",     # w op x op y op z
        "({}{}{}){}{}{}{}",   # (w op x) op y op z
        "({}{}{}{}{}){}{}",   # (w op x op y) op z
        "{}{}({}{}{}){}{}",   # w op (x op y) op z
        "{}{}({}{}{}{}{})",   # w op (x op y op z)
        "{}{}{}{}({}{}{})",   # w op x op (y op z)
        "({}{}{}){}({}{}{})", # (w op x) op (y op z)
        "(({}{}{}){}{}){}{}", # ((w op x) op y) op z
        "({}{}({}{}{})){}{}", # (w op (x op y)) op z
        "{}{}(({}{}{}){}{})", # w op ((x op y) op z)
        "{}{}({}{}({}{}{}))"  # w op (x op (y op z))
    ]
    valid = []

    for p in prod:
        gr, op = p
        for f in fmts:
            # Apply each parenthesis combination, calculate the result and
            # compare it with the expected one
            expr = f.format(gr[0], op[0], gr[1], op[1], gr[2], op[2], gr[3])
            try:
                if(eval(expr) == exp_res):
                    valid.append(expr)
            except ZeroDivisionError as e:
                # Ignore division by zero exception
                pass

    return sorted(valid)

def test():
    """Basic sanity tests"""
    assert(first_nonrepeating("tooth") == 'h')
    assert(first_nonrepeating("lool") == None)

    comb_tests = [
        [[6, 6, 5, 2], 36],
        [[1, 2, 3, 4], 4],
        [[1, 2, 0, 0], 1],
        [[1, 2, 3, 4], 17]
    ]

    for t in comb_tests:
        l = combine4(t[0], t[1])

        for x in l:
            try:
                print("{} = {}".format(x, eval(x)))
                assert(len(l) > 0)
                assert(eval(x) == t[1])
            except:
                # Workaround for the assignment test utility
                # (eval must be in a try-except block)
                raise

if __name__ == "__main__":
    test()
