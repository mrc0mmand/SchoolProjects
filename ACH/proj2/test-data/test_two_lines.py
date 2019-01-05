import sys
import math

pos = []
vel = []
COUNT = 32
with open(sys.argv[1], 'r') as f:
    for line in f:
        vals = line.split()
        pos.append([ float(vals[0]), float(vals[1]), float(vals[2]) ])
        vel.append([ float(vals[3]), float(vals[4]), float(vals[5]) ])


if not len(pos) == COUNT:
    print "*ERROR* File expects output of simulation using two-lines.dat as input and two-lines-setup.dat as constants in makefile"
    exit(-1)

fail = False
positions = []
#tests if the positions are still simetrical
for i in range(0,COUNT/2):
	#should cancel each other
    diff_pos = pos[i][0] + pos[COUNT-i-1][0]
    diff_vel = vel[i][0] + vel[COUNT-i-1][0]

    if (abs(diff_pos) > 0.001) or (abs(diff_vel) > 0.001):
        fail = True
        positions = [i, COUNT-i-1]

if fail:
    print("Pairs of particles on " + str(positions[0]) + " and " + str(positions[1]) +" positions do not cancel each other - ERROR")
    print("*ERROR* Points have NOT move uniformly!")
    exit(-1)
else:
    print("Particles cancel each others: - OK")
    exit(0)
