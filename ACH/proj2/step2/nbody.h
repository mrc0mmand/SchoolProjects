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

enum {
    POS_X = 0,
    POS_Y,
    POS_Z,
    VEL_X,
    VEL_Y,
    VEL_Z,
    WEIGHT,
    P_EN_SIZE
};

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

__global__ void calculate_velocity(t_particles p_in, t_particles p_out, int N, float dt);

void particles_read(FILE *fp, t_particles &p, int N);

void particles_write(FILE *fp, t_particles &p, int N);

#endif /* __NBODY_H__ */
