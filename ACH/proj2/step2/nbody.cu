/*
 * Architektura procesoru (ACH 2017)
 * Projekt c. 2 (cuda)
 * Login: xsumsa01
 */

#include <cmath>
#include <cfloat>
#include "nbody.h"

/**
  * @brief Calculate index to the shared memory
  * @details Calculation: thread_id * #t_particles + offset,
             where thread_idx is ID of the active thread,
             #t_particles is number of items of the t_particles structure,
             and offset is index of each structure member defined by
             enum in nbody.h file
  *
  * @param id Thread ID
  * @param x Offset (see enum in nbody.h)
  */
#define SH_IDX(id, x) (id * P_EN_SIZE + (x))

__global__ void calculate_velocity(t_particles p_in, t_particles p_out, int N, float dt)
{
    int idx = (blockIdx.x * blockDim.x) + threadIdx.x;
    int tid = threadIdx.x;
    extern __shared__ float p_sh[];

    if(idx >= N)
        return;

    p_sh[SH_IDX(tid, POS_X)] = p_in.pos_x[idx];
    p_sh[SH_IDX(tid, POS_Y)] = p_in.pos_y[idx];
    p_sh[SH_IDX(tid, POS_Z)] = p_in.pos_z[idx];
    p_sh[SH_IDX(tid, VEL_X)] = p_in.vel_x[idx];
    p_sh[SH_IDX(tid, VEL_Y)] = p_in.vel_y[idx];
    p_sh[SH_IDX(tid, VEL_Z)] = p_in.vel_z[idx];
    p_sh[SH_IDX(tid, WEIGHT)] = p_in.weight[idx];
    __syncthreads();

    float vel_x = 0;
    float vel_y = 0;
    float vel_z = 0;

    for(int i = 0; i < N; i++) {
        float r, dx, dy, dz;

        dx = p_in.pos_x[i] - p_sh[SH_IDX(tid, POS_X)];
        dy = p_in.pos_y[i] - p_sh[SH_IDX(tid, POS_Y)];
        dz = p_in.pos_z[i] - p_sh[SH_IDX(tid, POS_Z)];

        r = sqrt(dx*dx + dy*dy + dz*dz);

        if(r > COLLISION_DISTANCE) {
            /* Newton's law of universal gravitation:
             *      F = G * ((m1 * m2) / r^2) * u
             * where G is the gravitational constant, m1 and m2 are masses of particles,
             * r is distance, and u is a unit vector defined as:
             *      u = (r2 - r1) / r
             *
             * Gravitational velocity:
             *      v_g = F / m * d_t
             */

            float f = (G * p_sh[SH_IDX(tid, WEIGHT)] * p_in.weight[i]) / (r * r);

            vel_x += r != 0.0f ? (((f * (dx/r)) / p_sh[SH_IDX(tid, WEIGHT)]) * dt) : 0.0f;
            vel_y += r != 0.0f ? (((f * (dy/r)) / p_sh[SH_IDX(tid, WEIGHT)]) * dt) : 0.0f;
            vel_z += r != 0.0f ? (((f * (dz/r)) / p_sh[SH_IDX(tid, WEIGHT)]) * dt) : 0.0f;
        } else if(r > 0.0f && r < COLLISION_DISTANCE) {
            /* Collision velocities:
             *      w1 = (m1 - m2) * v1 / M + 2 * m2 * v2 / M
             *  where m1 and m2 are masses of particles, v1 and v2 are velocities, and
             *  M is the center of mass calculated as m1 + m2
             */

            float mtot = p_sh[SH_IDX(tid, WEIGHT)] + p_in.weight[i];
            float mdif = p_sh[SH_IDX(tid, WEIGHT)] - p_in.weight[i];

            vel_x += ((mdif * p_sh[SH_IDX(tid, VEL_X)] / mtot) + 2 * (p_in.weight[i] * p_in.vel_x[i]) / mtot) - p_sh[SH_IDX(tid, VEL_X)];
            vel_y += ((mdif * p_sh[SH_IDX(tid, VEL_Y)] / mtot) + 2 * (p_in.weight[i] * p_in.vel_y[i]) / mtot) - p_sh[SH_IDX(tid, VEL_Y)];
            vel_z += ((mdif * p_sh[SH_IDX(tid, VEL_Z)] / mtot) + 2 * (p_in.weight[i] * p_in.vel_z[i]) / mtot) - p_sh[SH_IDX(tid, VEL_Z)];
        }
    }

    p_out.vel_x[idx] = p_sh[SH_IDX(tid, VEL_X)] + vel_x;
    p_out.vel_y[idx] = p_sh[SH_IDX(tid, VEL_Y)] + vel_y;
    p_out.vel_z[idx] = p_sh[SH_IDX(tid, VEL_Z)] + vel_z;

    p_out.pos_x[idx] = p_sh[SH_IDX(tid, POS_X)] + p_out.vel_x[idx] * dt;
    p_out.pos_y[idx] = p_sh[SH_IDX(tid, POS_Y)] + p_out.vel_y[idx] * dt;
    p_out.pos_z[idx] = p_sh[SH_IDX(tid, POS_Z)] + p_out.vel_z[idx] * dt;
}

__host__ void particles_read(FILE *fp, t_particles &p, int N)
{
    for(int i = 0; i < N; i++) {
        fscanf(fp, "%f %f %f %f %f %f %f \n",
                &p.pos_x[i], &p.pos_y[i], &p.pos_z[i],
                &p.vel_x[i], &p.vel_y[i], &p.vel_z[i],
                &p.weight[i]);
    }
}

__host__  void particles_write(FILE *fp, t_particles &p, int N)
{
    for (int i = 0; i < N; i++)
    {
        fprintf(fp, "%10.10f %10.10f %10.10f %10.10f %10.10f %10.10f %10.10f \n",
                p.pos_x[i], p.pos_y[i], p.pos_z[i],
                p.vel_x[i], p.vel_y[i], p.vel_z[i],
                p.weight[i]);
    }
}
