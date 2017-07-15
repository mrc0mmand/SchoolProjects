#!/usr/bin/env python3

from itertools import permutations

class Polynomial:
    """Polynomial

    Class for manipulation with polynomials of abritrary order.

    A polynomial can be created in three following ways:
    1) By passing a list of coefficients
        px = Polynomial([1, 2, 3, 4])
    2) By passing each coefficient as a separate argument:
        px = Polynomial(1, 2, 3, 4)
    3) Using keyword arguments (kwargs):
        px = Polynomial(x1=1, x2=2, x3=3, x4=4)

    Note: all coefficient lists begin with the lowest order (i.e. x^0)

    """
    def __init__(self, *args, **kwargs):
        """Construct a Polynomial object.

        See class docstring for more information.

        """
        self._koef_list = []

        if args and len(args) == 1:
            # List was passed
            self._koef_list = args[0]
        elif args:
            self._koef_list = args
        else:
            # Parse kwargs
            try:
                # Find the largest keyword argument (e.g. x11)
                midx = int(max(int(x[1:]) for x in kwargs.keys()))
            except ValueError as e:
                raise ValueError("Invalid arguments ({})".format(e))

            # Create a list from the keyword arguments
            for i in range(0, midx + 1):
                tkey = "x" + str(i)
                if tkey in kwargs:
                    self._koef_list.append(kwargs[tkey])
                else:
                    self._koef_list.append(0)

    def __str__(self):
        """Convert the internal coefficent list into a printable polynomial.

        Resulting polynomial has a following format:
            x^4 - 5x^3 + 3x^2 + x - 4

        Returns:
            String containing the resulting polynomial.
        """
        res = ""
        i = len(self._koef_list)
        for x in reversed(self._koef_list):
            i -= 1
            if x == 0:
                continue

            sign = "-" if x < 0 else "+"
            koef = abs(x) if abs(x) != 1 else ""
            if i > 1:
                order = "x^" + str(i)
            elif i == 1:
                order = "x"
            else:
                order = ""
                koef = abs(x)

            if res:
                res += " {} ".format(sign)
            elif x < 0:
                res += "{} ".format(sign)

            res += "{}{}".format(koef, order)

        return res if res else "0"

    def __eq__(self, x):
        """Compare two polynomials

        Arguments:
            x -- an instance of Polynomial class or string representation of
                 a polynomial to compare with

        Returns:
            True if both polynomials are equal, False otherwise.
        """
        return str(self) == str(x)

    def __add__(self, x):
        """Add two polynomials and return the result.

        Arguments:
            x -- an instance of Polynomial class to add to

        Returns:
            A new instance of Polynomial class with sum of both polynomials.

        """
        maxlen = max(len(self._koef_list), len(x._koef_list))
        a = self._pad(self._koef_list, maxlen)
        b = self._pad(list(x._koef_list), maxlen)
        return Polynomial([sum(l) for l in zip(a, b)])

    def __pow__(self, order):
        """Raise polynomial to given power.

        Arguments:
            order -- an order to raise the polynomial to

        Returns:
            A new instance of Polynomial class raised to the given power.

        """
        max_order = (len(self._koef_list) - 1) * order
        last_p = self._koef_list
        aux_array = []

        for _ in range(0, order - 1):
            for x in range(0, len(self._koef_list)):
                aux = [0] * (max_order + 1)
                for y in range(0, len(last_p)):
                    aux[x + y] = self._koef_list[x] * last_p[y]

                aux_array.append(aux)

            last_p = list(map(sum, zip(*aux_array)))
            # Trim unnecessary fields from the new polynomial
            for i in range(len(last_p) - 1, -1, -1):
                if last_p[i] == 0:
                    last_p.pop()
                else:
                    break

            aux_array = []

        return Polynomial(last_p)

    def _pad(self, slist, maxlen, val = 0):
        """An auxilliary method to pad given list to given length.

        Arguments:
            slist -- source list
            maxlen -- maximum length of the resulting list
            val -- value to pad the list with

        Returns:
            A new padded list.

        """
        return slist + [val] * (maxlen - len(slist))

    def derivative(self):
        """Derivate the polynomial.

        Returns:
            A new instance of derived polynomial.

        """
        res = []
        for i in range(1, len(self._koef_list)):
            res.append(self._koef_list[i] * i)

        return Polynomial(res)

    def at_value(self, x, y = None):
        """Solve the polynomial for given x.

        If y is given as well, method returns a difference between
        at_value(y) and at_value(x).

        Arguments:
            x -- value of x to solve the polynomial for
            y -- second value of x to solve the polynomial for

        """
        if y is not None:
            return self.at_value(y) - self.at_value(x)

        res = 0
        for i in range(1, len(self._koef_list)):
            res += self._koef_list[i] * (x ** i)

        return res + self._koef_list[0]

def test():
    assert str(Polynomial(0,1,0,-1,4,-2,0,1,3,0)) == "3x^8 + x^7 - 2x^5 + 4x^4 - x^3 + x"
    assert str(Polynomial([-5,1,0,-1,4,-2,0,1,3,0])) == "3x^8 + x^7 - 2x^5 + 4x^4 - x^3 + x - 5"
    assert str(Polynomial(x7=1, x4=4, x8=3, x9=0, x0=0, x5=-2, x3= -1, x1=1)) == "3x^8 + x^7 - 2x^5 + 4x^4 - x^3 + x"
    assert str(Polynomial(x2=0)) == "0"
    assert str(Polynomial(x0=0)) == "0"
    assert Polynomial(x0=2, x1=0, x3=0, x2=3) == Polynomial(2,0,3)
    assert Polynomial(x2=0) == Polynomial(x0=0)
    assert str(Polynomial(x0=1)+Polynomial(x1=1)) == "x + 1"
    assert str(Polynomial([-1,1,1,0])+Polynomial(1,-1,1)) == "2x^2"
    pol1 = Polynomial(x2=3, x0=1)
    pol2 = Polynomial(x1=1, x3=0)
    assert str(pol1+pol2) == "3x^2 + x + 1"
    assert str(pol1+pol2) == "3x^2 + x + 1"
    assert str(Polynomial(x0=-1,x1=1)**1) == "x - 1"
    assert str(Polynomial(x0=-1,x1=1)**2) == "x^2 - 2x + 1"
    pol3 = Polynomial(x0=-1,x1=1)
    assert str(pol3**4) == "x^4 - 4x^3 + 6x^2 - 4x + 1"
    assert str(pol3**4) == "x^4 - 4x^3 + 6x^2 - 4x + 1"
    assert str(Polynomial(x0=2).derivative()) == "0"
    assert str(Polynomial(x3=2,x1=3,x0=2).derivative()) == "6x^2 + 3"
    assert str(Polynomial(x3=2,x1=3,x0=2).derivative().derivative()) == "12x"
    pol4 = Polynomial(x3=2,x1=3,x0=2)
    assert str(pol4.derivative()) == "6x^2 + 3"
    assert str(pol4.derivative()) == "6x^2 + 3"
    assert Polynomial(-2,3,4,-5).at_value(0) == -2
    assert Polynomial(x2=3, x0=-1, x1=-2).at_value(3) == 20
    assert Polynomial(x2=3, x0=-1, x1=-2).at_value(3,5) == 44
    pol5 = Polynomial([1,0,-2])
    assert pol5.at_value(-2.4) == -10.52
    assert pol5.at_value(-2.4) == -10.52
    assert pol5.at_value(-1,3.6) == -23.92
    assert pol5.at_value(-1,3.6) == -23.92

if __name__ == '__main__':
    test()
