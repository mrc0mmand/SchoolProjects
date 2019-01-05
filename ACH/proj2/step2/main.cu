/*
 * Architektura procesoru (ACH 2017)
 * Projekt c. 2 (cuda)
 * Login: xsumsa01
 */

#include <sys/time.h>
#include <cstdio>
#include <cmath>

#include "nbody.h"

#define NF(x) (N * sizeof(float))

/**
  * @brief Allocate memory on CPU
  *
  * @param t Data type of the allocated memory
  * @param x Destination pointer
  * @param s Size of the allocated memory
  */
#define CPU_ALLOC(t, x, s) \
    do { \
        if(cudaMallocHost(&x, sizeof(t) * (s)) != cudaSuccess) { \
            cudaError e = cudaGetLastError(); \
            fprintf(stderr, "cudaMallocHost() failed: %s\n", cudaGetErrorString(e)); \
            exit(EXIT_FAILURE); \
        } \
        cudaMemset(x, 0, N * sizeof(*x)); \
    } while(0)

/**
  * @brief Free memory allocated by CPU_ALLOC
  *
  * @param x Pointer to allocated memory
  */
#define CPU_FREE(x) \
    do { \
        cudaFreeHost(x); \
        x = NULL; \
    } while(0)

/**
  * @brief Allocate memory on GPU
  *
  * @param t Data type of the allocated memory
  * @param x Destination pointer
  * @param s Size of the allocated memory
  */
#define GPU_ALLOC(t, x, s) \
    do { \
        if(cudaMalloc(&x, sizeof(t) * (s)) != cudaSuccess) { \
            cudaError e = cudaGetLastError(); \
            fprintf(stderr, "cudaMalloc() failed: %s\n", cudaGetErrorString(e)); \
            exit(EXIT_FAILURE); \
        } \
        cudaMemset(x, 0, N * sizeof(*x)); \
    } while(0)

/**
  * @brief Free memory allocated by GPU_ALLOC
  *
  * @param x Pointer to allocated memory
  */
#define GPU_FREE(x) \
    do { \
        cudaFree(x); \
        x = NULL; \
    } while(0)

int main(int argc, char **argv)
{
    FILE *fp;
    struct timeval t1, t2;
    int N;
    float dt;
    int steps;
    int thr_blc;

    // parametry
    if (argc != 7)
    {
        printf("Usage: nbody <N> <dt> <steps> <thr/blc> <input> <output>\n");
        exit(1);
    }
    N = atoi(argv[1]);
    dt = atof(argv[2]);
    steps = atoi(argv[3]);
    thr_blc = atoi(argv[4]);

    printf("N: %d\n", N);
    printf("dt: %f\n", dt);
    printf("steps: %d\n", steps);
    printf("threads/block: %d\n", thr_blc);

    // alokace pameti na CPU
    t_particles particles_cpu;

    CPU_ALLOC(float, particles_cpu.pos_x, N);
    CPU_ALLOC(float, particles_cpu.pos_y, N);
    CPU_ALLOC(float, particles_cpu.pos_z, N);
    CPU_ALLOC(float, particles_cpu.vel_x, N);
    CPU_ALLOC(float, particles_cpu.vel_y, N);
    CPU_ALLOC(float, particles_cpu.vel_z, N);
    CPU_ALLOC(float, particles_cpu.weight, N);

    // nacteni castic ze souboru
    fp = fopen(argv[5], "r");
    if (fp == NULL)
    {
        printf("Can't open file %s!\n", argv[5]);
        exit(1);
    }
    particles_read(fp, particles_cpu, N);
    fclose(fp);

    t_particles particles_gpu[2];

    /* DOPLNTE: alokaci pameti na GPU */
    for(size_t i = 0; i < 2; i++) {
        GPU_ALLOC(float, particles_gpu[i].pos_x, N);
        GPU_ALLOC(float, particles_gpu[i].pos_y, N);
        GPU_ALLOC(float, particles_gpu[i].pos_z, N);
        GPU_ALLOC(float, particles_gpu[i].vel_x, N);
        GPU_ALLOC(float, particles_gpu[i].vel_y, N);
        GPU_ALLOC(float, particles_gpu[i].vel_z, N);
        GPU_ALLOC(float, particles_gpu[i].weight, N);

        cudaMemcpy(particles_gpu[i].pos_x, particles_cpu.pos_x, NF(N), cudaMemcpyHostToDevice);
        cudaMemcpy(particles_gpu[i].pos_y, particles_cpu.pos_y, NF(N), cudaMemcpyHostToDevice);
        cudaMemcpy(particles_gpu[i].pos_z, particles_cpu.pos_z, NF(N), cudaMemcpyHostToDevice);
        cudaMemcpy(particles_gpu[i].vel_x, particles_cpu.vel_x, NF(N), cudaMemcpyHostToDevice);
        cudaMemcpy(particles_gpu[i].vel_y, particles_cpu.vel_y, NF(N), cudaMemcpyHostToDevice);
        cudaMemcpy(particles_gpu[i].vel_z, particles_cpu.vel_z, NF(N), cudaMemcpyHostToDevice);
        cudaMemcpy(particles_gpu[i].weight, particles_cpu.weight, NF(N), cudaMemcpyHostToDevice);
    }

    cudaError err = cudaGetLastError();
    if(err != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy(): %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    size_t grid = (N / thr_blc) + 1; //(N + thr_blc- 1) / thr_blc;

    // vypocet
    gettimeofday(&t1, 0);

    size_t p_in_idx = 0;
    size_t p_out_idx = 0;
    size_t sh_mem = thr_blc * sizeof(float) * P_EN_SIZE;
    for (int s = 0; s < steps; ++s)
    {
        // Swap p_in and p_out in each step
        p_in_idx = s % 2;
        p_out_idx = (s + 1) % 2;
        calculate_velocity<<<grid, thr_blc, sh_mem>>>(particles_gpu[p_in_idx], particles_gpu[p_out_idx], N, dt);
    }
    cudaDeviceSynchronize();
    gettimeofday(&t2, 0);

    // cas
    double t = (1000000.0 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec) / 1000000.0;
    printf("Time: %f s\n", t);

    cudaMemcpy(particles_cpu.pos_x, particles_gpu[p_out_idx].pos_x, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.pos_y, particles_gpu[p_out_idx].pos_y, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.pos_z, particles_gpu[p_out_idx].pos_z, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.vel_x, particles_gpu[p_out_idx].vel_x, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.vel_y, particles_gpu[p_out_idx].vel_y, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.vel_z, particles_gpu[p_out_idx].vel_z, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.weight, particles_gpu[p_out_idx].weight, NF(N), cudaMemcpyDeviceToHost);

    // ulozeni castic do souboru
    fp = fopen(argv[6], "w");
    if (fp == NULL)
    {
        printf("Can't open file %s!\n", argv[6]);
        exit(1);
    }
    particles_write(fp, particles_cpu, N);
    fclose(fp);

    // Cleanup
    CPU_FREE(particles_cpu.pos_x);
    CPU_FREE(particles_cpu.pos_y);
    CPU_FREE(particles_cpu.pos_z);
    CPU_FREE(particles_cpu.vel_x);
    CPU_FREE(particles_cpu.vel_y);
    CPU_FREE(particles_cpu.vel_z);
    CPU_FREE(particles_cpu.weight);

    for(size_t i = 0; i < 2; i++) {
        GPU_FREE(particles_gpu[0].pos_x);
        GPU_FREE(particles_gpu[0].pos_y);
        GPU_FREE(particles_gpu[0].pos_z);
        GPU_FREE(particles_gpu[0].vel_x);
        GPU_FREE(particles_gpu[0].vel_y);
        GPU_FREE(particles_gpu[0].vel_z);
        GPU_FREE(particles_gpu[0].weight);
    }

    return 0;
}
