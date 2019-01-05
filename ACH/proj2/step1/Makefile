# Architektura procesoru (ACH 2017)
# Projekt c. 2 (cuda)
# Login: xsumsa01

N=4096
DT=0.001f
STEPS=10
THREADS_PER_BLOCK=128

INPUT=../input.dat
OUTPUT=../step0.dat

flags=-Xptxas="-v"  


all:
	nvcc $(flags) nbody.cu main.cu -o nbody
	icpc gen.cpp -o gen

clean:
	rm -f *.o nbody

run:
	./nbody $(N) $(DT) $(STEPS) $(THREADS_PER_BLOCK) $(INPUT) $(OUTPUT)

profile:
	nvprof \
		--devices 0 \
		--metrics flops_sp \
		--metrics flops_sp_add \
		--metrics flops_sp_mul \
		--metrics flops_sp_fma \
		--metrics flops_sp_special \
		--metrics flop_sp_efficiency \
		--metrics sm_efficiency \
		--metrics alu_fu_utilization \
		--metrics gld_transactions \
		--metrics gld_efficiency \
		--metrics achieved_occupancy \
		--metrics shared_efficiency \
		./nbody $(N) $(DT) 1 $(THREADS_PER_BLOCK) $(INPUT) $(OUTPUT)
