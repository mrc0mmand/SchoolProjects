/*
 * Architektura procesoru (ACH 2016)
 * Projekt c. 1 (nbody)
 * Login: xsumsa01
 */

#include <cmath>
#include "nbody.h"

void particles_simulate(particles_t &p)
{
    int i, j, k;

    t_velocities velocities = {};

    __assume_aligned(&p, 64);
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
            // Force loop vectorization
            #pragma omp simd private(velocities)
            for (j = i; j < N; j++)
            {
                //calculate_velocity(p2, p1, velocities[j]);
                //calculate_velocity(p.pos_x[j], p.pos_y[j], p.pos_z[j], p.vel_x[j],
                //        p.vel_y[j], p.vel_z[j], p.weight[j],
                //        p.pos_x[i], p.pos_y[i], p.pos_z[i], p.vel_x[i],
                //        p.vel_y[i], p.vel_z[i], p.weight[i],
                //        velocities[j].x, velocities[j].y, velocities[j].z);
                //
                float r, dx, dy, dz;
                float vx, vy, vz, vx2, vy2, vz2;

                dx = p.pos_x[i] - p.pos_x[j];
                dy = p.pos_y[i] - p.pos_y[j];
                dz = p.pos_z[i] - p.pos_z[j];

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

                    float f = (G * p.weight[j] * p.weight[i]) / (r * r);

                    vx = r != 0.0f ? (((f * (dx/r)) / p.weight[j]) * DT) : 0.0f;
                    vy = r != 0.0f ? (((f * (dy/r)) / p.weight[j]) * DT) : 0.0f;
                    vz = r != 0.0f ? (((f * (dz/r)) / p.weight[j]) * DT) : 0.0f;

                    vx2 = r != 0.0f ? (((f * (-dx/r)) / p.weight[i]) * DT) : 0.0f;
                    vy2 = r != 0.0f ? (((f * (-dy/r)) / p.weight[i]) * DT) : 0.0f;
                    vz2 = r != 0.0f ? (((f * (-dz/r)) / p.weight[i]) * DT) : 0.0f;

                    velocities[j].x += vx;
                    velocities[j].y += vy;
                    velocities[j].z += vz;

                    velocities[i].x += vx2;
                    velocities[i].y += vy2;
                    velocities[i].z += vz2;
                } else if(r > 0.0f && r < COLLISION_DISTANCE) {
                    /* Collision velocities:
                     *      w1 = (m1 - m2) * v1 / M + 2 * m2 * v2 / M
                     *  where m1 and m2 are masses of particles, v1 and v2 are velocities, and
                     *  M is the center of mass calculated as m1 + m2
                     */

                    float mtot = p.weight[j] + p.weight[i];
                    float wdif = p.weight[j] - p.weight[i];

                    vx = ((wdif * p.vel_x[j] / mtot) + 2 * (p.weight[i] * p.vel_x[i]) / mtot) - p.vel_x[j];
                    vy = ((wdif * p.vel_y[j] / mtot) + 2 * (p.weight[i] * p.vel_y[i]) / mtot) - p.vel_y[j];
                    vz = ((wdif * p.vel_z[j] / mtot) + 2 * (p.weight[i] * p.vel_z[i]) / mtot) - p.vel_z[j];

                    vx2 = ((-wdif * p.vel_x[i] / mtot) + 2 * (p.weight[j] * p.vel_x[j]) / mtot) - p.vel_x[i];
                    vy2 = ((-wdif * p.vel_y[i] / mtot) + 2 * (p.weight[j] * p.vel_y[j]) / mtot) - p.vel_y[i];
                    vz2 = ((-wdif * p.vel_z[i] / mtot) + 2 * (p.weight[j] * p.vel_z[j]) / mtot) - p.vel_z[i];

                    velocities[j].x += vx;
                    velocities[j].y += vy;
                    velocities[j].z += vz;

                    velocities[i].x += vx2;
                    velocities[i].y += vy2;
                    velocities[i].z += vz2;
                }
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
