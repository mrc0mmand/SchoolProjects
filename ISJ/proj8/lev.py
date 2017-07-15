#!/usr/bin/env python3

def lev(a, b):
    """Recursively calculate the Levenshtein edit distance between two strings,
    a and b.
    Returns the edit distance.
    """
    if("" == a):
        return len(b)   # returns if a is an empty string
    if("" == b):
        return len(a)   # returns if b is an empty string
    return min(lev(a[:-1], b[:-1])+(a[-1] != b[-1]), lev(a[:-1], b)+1, lev(a, b[:-1])+1)

print(lev('abracadabra', 'abba'))

# Editační vzdáleností můžeme měřit vzdálenost, odlišnost, dvou řetězců.
# V případě editační vzdálenosti je vzdálenost definována jako počet operací
# smazání znaku, vložení znaku a záměny znaku tak, aby se jeden řetězec
# transformoval na druhý.
# Výpočet můžeme popsat následujícím způsobem:
# Máme dány dva řetězce a = a_1 a_2... a_m a b = b_1 b_2... b_n.
# Editační vzdálenost d_i,j předpony délky i řetězce a 
# a předpony délky j řetězce b můžeme vypočítat jako
# 1: d_i 1,j + 1 d d i,j = min i,j 1 + 1 (1) d i 1,j 1 + 1 pokud a i b j d i 1,j 1
# pokud a i = b j pro 1 i m a 1 j n.
# První hodnota odpovídá vymazání j-tého znaku z prvního řetězce, druhá hodnota
# odpovídá vložení znaku na (j+1)-ní pozici do prvního řetězce, třetí hodnota
# odpovídá výměně j-tého a i-tého znaku. Čtvrtá hodnota se uplatní jen 
# v případě, že jsou znaky v obou řetězcích shodné.
# Na konci hodnota d_mn udává editační vzdálenost řetězců a a b. Dále je
# pochopitelně definováno: d 0,0 = 0 d i,0 = i, pro 1 i m d 0,j = j, pro 1 j n Z
# předchozího textu je patrné, že celý výpočet můžeme implementovat jako
# rekurzivní funkci, kde výpočet hodnoty d mn se rozpadne na výpočet hodnot d m
# 1,n, d m,n 1, případně d m 1,n 1 a tak dále, až k definovaným koncovým hodnotám
# d 0,0 atd. Příklad Hodnota d ij udává editační vzdálenost mezi prvními i znaky
# řetězce a a prvními j znaky řetězce b. Tuto hodnotu však neumíme vypočítat
# přímo, ale vypočteme ji na základě znalostí vzdáleností mezi prvními i 1 znaky
# řetězce a a prvními j 1 znaky řetězce b. Tímto způsobem postupně redukujeme
# problém až na úroveň případů, kdy je hodnota známa přímo, například pro d 0,0 =
# 0. Předpokládejme, že a = abcabba a b = cbabac, odtud m = 7 a n = 6. Editační
# vzdálenost těchto dvou řetězců se bude počítat jako hodnota d 7,6 d 7,6 = min d
# 6,6 + 1 d 7,5 + 1 d 6,5 + 1 protože a 7 b 6 tj. a c Editační vzdálenost celých
# řetězců a a b je jinak řečeno rovna editační vzdálenosti prvních 7 znaků
# řetězce a a prvních 6 znaků řetězce b. Hodnotu d 6,6 1 Předponou délky i
# řetězce a se myslí prvních i znaků od začátku řetězce. 9


