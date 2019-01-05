/*
 * Architektura procesoru (ACH 2016)
 * Projekt c. 1 (nbody)
 * Login: xsumsa01
 */

#include "nbody.h"

void particles_simulate(particles_t &p)
{
    int i, j, k;

    t_velocities velocities = {};

    __assume_aligned((float*)(p.pos_x), 64);
    __assume_aligned((float*)(p.pos_y), 64);
    __assume_aligned((float*)(p.pos_z), 64);
    __assume_aligned((float*)(p.vel_x), 64);
    __assume_aligned((float*)(p.vel_y), 64);
    __assume_aligned((float*)(p.vel_z), 64);
    __assume_aligned((float*)(p.weight), 64);

    for (k = 0; k < STEPS; k++)
    {
        //vynulovani mezisouctu
        for (i = 0; i < N; i++)
        {
            velocities[i].x = 0.0f;
            velocities[i].y = 0.0f;
            velocities[i].z = 0.0f;
        }
        //vypocet nove rychlosti
        for (i = 0; i < N; i++)
        {
            const t_particle p1 = {p.pos_x[i], p.pos_y[i], p.pos_z[i], p.vel_x[i],
                                   p.vel_y[i], p.vel_z[i], p.weight[i]};

            // Force loop vectorization
            #pragma omp simd
            for (j = 0; j < N; j++)
            {
                const t_particle p2 = {p.pos_x[j], p.pos_y[j], p.pos_z[j], p.vel_x[j],
                                   p.vel_y[j], p.vel_z[j], p.weight[j]};
                calculate_gravitation_velocity(p2, p1, velocities[j]);
                calculate_collision_velocity(p2, p1, velocities[j]);
            }
        }
            //ulozeni rychlosti a posun castic
        for (i = 0; i < N; i++)
        {
            p.vel_x[i] += velocities[i].x;
            p.vel_y[i] += velocities[i].y;
            p.vel_z[i] += velocities[i].z;

            p.pos_x[i] += p.vel_x[i] * DT;
            p.pos_y[i] += p.vel_y[i] * DT;
            p.pos_z[i] += p.vel_z[i] * DT;
        }
    }
}


void particles_read(FILE *fp, particles_t &p)
{
    for (int i = 0; i < N; i++)
    {
        fscanf(fp, "%f %f %f %f %f %f %f \n",
            &p.pos_x[i], &p.pos_y[i], &p.pos_z[i],
            &p.vel_x[i], &p.vel_y[i], &p.vel_z[i],
            &p.weight[i]);
    }
}

void particles_write(FILE *fp, particles_t &p)
{
    for (int i = 0; i < N; i++)
    {
        fprintf(fp, "%10.10f %10.10f %10.10f %10.10f %10.10f %10.10f %10.10f \n",
            p.pos_x[i], p.pos_y[i], p.pos_z[i],
            p.vel_x[i], p.vel_y[i], p.vel_z[i],
            p.weight[i]);
    }
}
