#!/bin/sh

#clean files
rm -rf ~test-outputs
mkdir ~test-outputs
THR_BLC=128

#Test: Two particles on circle
echo "Two particles on circular trajectory..."
../nbody 2 0.00001f 543847 $THR_BLC ../../test-data/circle.dat ~test-outputs/circle.out
./test-difference.py ~test-outputs/circle.out ../../test-data/circle-ref.dat

#Test:
echo "Points on line without collision"
../nbody 32 0.001f 10000 $THR_BLC ../../test-data/two-lines.dat ~test-outputs/two-lines-v.out
./test-difference.py ~test-outputs/two-lines-v.out ../../test-data/two-lines-ref.dat

#Test:
echo "Points on line with one collision"
../nbody 32 0.001f 45000 $THR_BLC ../../test-data/two-lines.dat ~test-outputs/two-lines-one-v.out
./test-difference.py ~test-outputs/two-lines-one-v.out ../../test-data/two-lines-collided-45k.dat

#Test:
echo "Points on line with several collision"
../nbody 32 0.001f 50000 $THR_BLC ../../test-data/two-lines.dat ~test-outputs/two-lines-several.out
./test-difference.py ~test-outputs/two-lines-several.out ../../test-data/two-lines-collided-50k.dat


#Test
echo "Symetry globe test"
../nbody 932 0.1f 1 $THR_BLC ../../test-data/thompson_points_932.dat ~test-outputs/thompson-v.out
./test-thompson.py ../../test-data/thompson_points_932.dat ~test-outputs/thompson-v.out


#Test:
echo "Stability globe test"
../nbody 932 0.00001f 15000 $THR_BLC ../../test-data/thompson_points_932.dat ~test-outputs/thompson.out
./test-thompson.py ../../test-data/thompson_points_932.dat ~test-outputs/thompson.out
