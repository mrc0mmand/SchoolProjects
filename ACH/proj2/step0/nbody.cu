/*
 * Architektura procesoru (ACH 2017)
 * Projekt c. 2 (cuda)
 * Login: xsumsa01
 */

#include <cmath>
#include <cfloat>
#include "nbody.h"

__global__ void calculate_gravitation_velocity(t_particles p, t_velocities tmp_vel, int N, float dt)
{
    int idx = (blockIdx.x * blockDim.x) + threadIdx.x;

    if(idx >= N)
        return;

    for(int i = 0; i < N; i++) {
        float r, dx, dy, dz;
        float vx, vy, vz;

        dx = p.pos_x[i] - p.pos_x[idx];
        dy = p.pos_y[i] - p.pos_y[idx];
        dz = p.pos_z[i] - p.pos_z[idx];

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

            float f = (G * p.weight[idx] * p.weight[i]) / (r * r);

            vx = r != 0.0f ? (((f * (dx/r)) / p.weight[idx]) * dt) : 0.0f;
            vy = r != 0.0f ? (((f * (dy/r)) / p.weight[idx]) * dt) : 0.0f;
            vz = r != 0.0f ? (((f * (dz/r)) / p.weight[idx]) * dt) : 0.0f;

            tmp_vel.x[idx] += vx;
            tmp_vel.y[idx] += vy;
            tmp_vel.z[idx] += vz;
        }
    }
}

__global__ void calculate_collision_velocity(t_particles p, t_velocities tmp_vel, int N, float dt)
{
    int idx = (blockIdx.x * blockDim.x) + threadIdx.x;

    if(idx >= N)
        return;

    for(int i = 0; i < N; i++) {
        float r, dx, dy, dz;
        float vx, vy, vz;

        dx = p.pos_x[i] - p.pos_x[idx];
        dy = p.pos_y[i] - p.pos_y[idx];
        dz = p.pos_z[i] - p.pos_z[idx];

        r = sqrt(dx*dx + dy*dy + dz*dz);

        if(r > 0.0f && r < COLLISION_DISTANCE) {
            /* Collision velocities:
             *      w1 = (m1 - m2) * v1 / M + 2 * m2 * v2 / M
             *  where m1 and m2 are masses of particles, v1 and v2 are velocities, and
             *  M is the center of mass calculated as m1 + m2
             */

            float mtot = p.weight[idx] + p.weight[i];
            float mdif = p.weight[idx] - p.weight[i];

            vx = ((mdif * p.vel_x[idx] / mtot) + 2 * (p.weight[i] * p.vel_x[i]) / mtot) - p.vel_x[idx];
            vy = ((mdif * p.vel_y[idx] / mtot) + 2 * (p.weight[i] * p.vel_y[i]) / mtot) - p.vel_y[idx];
            vz = ((mdif * p.vel_z[idx] / mtot) + 2 * (p.weight[i] * p.vel_z[i]) / mtot) - p.vel_z[idx];

            tmp_vel.x[idx] += vx;
            tmp_vel.y[idx] += vy;
            tmp_vel.z[idx] += vz;
        }
    }
}

__global__ void update_particle(t_particles p, t_velocities tmp_vel, int N, float dt)
{
    int idx = (blockIdx.x * blockDim.x) + threadIdx.x;

    if(idx >= N)
        return;

    p.vel_x[idx] += tmp_vel.x[idx];
    p.vel_y[idx] += tmp_vel.y[idx];
    p.vel_z[idx] += tmp_vel.z[idx];

    p.pos_x[idx] += p.vel_x[idx] * dt;
    p.pos_y[idx] += p.vel_y[idx] * dt;
    p.pos_z[idx] += p.vel_z[idx] * dt;
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
