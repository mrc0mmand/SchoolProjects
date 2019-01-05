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

#define CPU_FREE(x) \
    do { \
        cudaFreeHost(x); \
        x = NULL; \
    } while(0)

#define GPU_ALLOC(t, x, s) \
    do { \
        if(cudaMalloc(&x, sizeof(t) * (s)) != cudaSuccess) { \
            cudaError e = cudaGetLastError(); \
            fprintf(stderr, "cudaMalloc() failed: %s\n", cudaGetErrorString(e)); \
            exit(EXIT_FAILURE); \
        } \
        cudaMemset(x, 0, N * sizeof(*x)); \
    } while(0)

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

    t_particles particles_gpu;
    t_velocities tmp_velocities_gpu;

    /* DOPLNTE: alokaci pameti na GPU */
    GPU_ALLOC(float, particles_gpu.pos_x, N);
    GPU_ALLOC(float, particles_gpu.pos_y, N);
    GPU_ALLOC(float, particles_gpu.pos_z, N);
    GPU_ALLOC(float, particles_gpu.vel_x, N);
    GPU_ALLOC(float, particles_gpu.vel_y, N);
    GPU_ALLOC(float, particles_gpu.vel_z, N);
    GPU_ALLOC(float, particles_gpu.weight, N);

    GPU_ALLOC(float, tmp_velocities_gpu.x, N);
    GPU_ALLOC(float, tmp_velocities_gpu.y, N);
    GPU_ALLOC(float, tmp_velocities_gpu.z, N);

    cudaMemcpy(particles_gpu.pos_x, particles_cpu.pos_x, NF(N), cudaMemcpyHostToDevice);
    cudaMemcpy(particles_gpu.pos_y, particles_cpu.pos_y, NF(N), cudaMemcpyHostToDevice);
    cudaMemcpy(particles_gpu.pos_z, particles_cpu.pos_z, NF(N), cudaMemcpyHostToDevice);
    cudaMemcpy(particles_gpu.vel_x, particles_cpu.vel_x, NF(N), cudaMemcpyHostToDevice);
    cudaMemcpy(particles_gpu.vel_y, particles_cpu.vel_y, NF(N), cudaMemcpyHostToDevice);
    cudaMemcpy(particles_gpu.vel_z, particles_cpu.vel_z, NF(N), cudaMemcpyHostToDevice);
    cudaMemcpy(particles_gpu.weight, particles_cpu.weight, NF(N), cudaMemcpyHostToDevice);

    cudaError err = cudaGetLastError();
    if(err != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy(): %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    size_t grid = (N / thr_blc) + 1; //(N + thr_blc- 1) / thr_blc;

    // vypocet
    gettimeofday(&t1, 0);

    for (int s = 0; s < steps; ++s)
    {
        calculate_gravitation_velocity<<<grid, thr_blc>>>(particles_gpu, tmp_velocities_gpu, N, dt);
        calculate_collision_velocity<<<grid, thr_blc>>>(particles_gpu, tmp_velocities_gpu, N, dt);
        update_particle<<<grid, thr_blc>>>(particles_gpu, tmp_velocities_gpu, N, dt);
        cudaMemset(tmp_velocities_gpu.x, 0, NF(N));
        cudaMemset(tmp_velocities_gpu.y, 0, NF(N));
        cudaMemset(tmp_velocities_gpu.z, 0, NF(N));
    }
    cudaDeviceSynchronize();
    gettimeofday(&t2, 0);

    // cas
    double t = (1000000.0 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec) / 1000000.0;
    printf("Time: %f s\n", t);

    cudaMemcpy(particles_cpu.pos_x, particles_gpu.pos_x, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.pos_y, particles_gpu.pos_y, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.pos_z, particles_gpu.pos_z, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.vel_x, particles_gpu.vel_x, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.vel_y, particles_gpu.vel_y, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.vel_z, particles_gpu.vel_z, NF(N), cudaMemcpyDeviceToHost);
    cudaMemcpy(particles_cpu.weight, particles_gpu.weight, NF(N), cudaMemcpyDeviceToHost);

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

    GPU_FREE(particles_gpu.pos_x);
    GPU_FREE(particles_gpu.pos_y);
    GPU_FREE(particles_gpu.pos_z);
    GPU_FREE(particles_gpu.vel_x);
    GPU_FREE(particles_gpu.vel_y);
    GPU_FREE(particles_gpu.vel_z);
    GPU_FREE(particles_gpu.weight);

    GPU_FREE(tmp_velocities_gpu.x);
    GPU_FREE(tmp_velocities_gpu.y);
    GPU_FREE(tmp_velocities_gpu.z);

    return 0;
}
