/*
 * Architektura procesoru (ACH 2017)
 * Projekt c. 2 (cuda)
 * Login: xsumsa01
 */

#include <cmath>
#include <cfloat>
#include "nbody.h"

__global__ void calculate_velocity(t_particles p_in, t_particles p_out, int N, float dt)
{
    int idx = (blockIdx.x * blockDim.x) + threadIdx.x;

    if(idx >= N)
        return;

    float vel_x = 0;
    float vel_y = 0;
    float vel_z = 0;

    for(int i = 0; i < N; i++) {
        float r, dx, dy, dz;

        dx = p_in.pos_x[i] - p_in.pos_x[idx];
        dy = p_in.pos_y[i] - p_in.pos_y[idx];
        dz = p_in.pos_z[i] - p_in.pos_z[idx];

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

            float f = (G * p_in.weight[idx] * p_in.weight[i]) / (r * r);

            vel_x += r != 0.0f ? (((f * (dx/r)) / p_in.weight[idx]) * dt) : 0.0f;
            vel_y += r != 0.0f ? (((f * (dy/r)) / p_in.weight[idx]) * dt) : 0.0f;
            vel_z += r != 0.0f ? (((f * (dz/r)) / p_in.weight[idx]) * dt) : 0.0f;
        } else if(r > 0.0f && r < COLLISION_DISTANCE) {
            /* Collision velocities:
             *      w1 = (m1 - m2) * v1 / M + 2 * m2 * v2 / M
             *  where m1 and m2 are masses of particles, v1 and v2 are velocities, and
             *  M is the center of mass calculated as m1 + m2
             */

            float mtot = p_in.weight[idx] + p_in.weight[i];
            float mdif = p_in.weight[idx] - p_in.weight[i];

            vel_x += ((mdif * p_in.vel_x[idx] / mtot) + 2 * (p_in.weight[i] * p_in.vel_x[i]) / mtot) - p_in.vel_x[idx];
            vel_y += ((mdif * p_in.vel_y[idx] / mtot) + 2 * (p_in.weight[i] * p_in.vel_y[i]) / mtot) - p_in.vel_y[idx];
            vel_z += ((mdif * p_in.vel_z[idx] / mtot) + 2 * (p_in.weight[i] * p_in.vel_z[i]) / mtot) - p_in.vel_z[idx];
        }
    }

    p_out.vel_x[idx] = p_in.vel_x[idx] + vel_x;
    p_out.vel_y[idx] = p_in.vel_y[idx] + vel_y;
    p_out.vel_z[idx] = p_in.vel_z[idx] + vel_z;

    p_out.pos_x[idx] = p_in.pos_x[idx] + p_out.vel_x[idx] * dt;
    p_out.pos_y[idx] = p_in.pos_y[idx] + p_out.vel_y[idx] * dt;
    p_out.pos_z[idx] = p_in.pos_z[idx] + p_out.vel_z[idx] * dt;
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
