/*
 * Architektura procesoru (ACH 2017)
 * Projekt c. 2 (cuda)
 * Login: xsumsa01
 */

#ifndef __NBODY_H__
#define __NBODY_H__

#include <cstdlib>
#include <cstdio>

/* gravitacni konstanta */
#define G 6.67384e-11f
#define COLLISION_DISTANCE 0.01f

/* struktura castic */
typedef struct
{
    float *pos_x;
    float *pos_y;
    float *pos_z;
    float *vel_x;
    float *vel_y;
    float *vel_z;
    float *weight;
} t_particles;

/* pomocna struktura rychlosti */
typedef struct
{
    float *x;
    float *y;
    float *z;
} t_velocities;

__global__ void calculate_gravitation_velocity(t_particles p, t_velocities tmp_vel, int N, float dt);

__global__ void calculate_collision_velocity(t_particles p, t_velocities tmp_vel, int N, float dt);

__global__ void update_particle(t_particles p, t_velocities tmp_vel, int N, float dt);

void particles_read(FILE *fp, t_particles &p, int N);

void particles_write(FILE *fp, t_particles &p, int N);

#endif /* __NBODY_H__ */
