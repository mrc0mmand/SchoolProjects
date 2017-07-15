#!/usr/bin/env python3

import fileinput

words = []
rev = []
for line in fileinput.input():
    w = line.rstrip()
    words.append(w)
    rev.append(w[::-1])

words = set(words).intersection(rev)

palindroms = [w for w in words if w == w[::-1]]

result = [w for w in words if w not in palindroms]

print(sorted(result))
