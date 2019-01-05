/*
 * Architektura procesoru (ACH 2016)
 * Projekt c. 1 (nbody)
 * Login: xsumsa01
 */

#ifndef __NBODY_H__
#define __NBODY_H__

#include <cstdlib>
#include <cstdio>
#include "velocity.h"

using t_particles   = t_particle[N];
using t_velocities  = t_velocity[N];

/* SoA (StructureOfArrays) version of AoS (ArrayOfStructures) data structure
 * t_particles. This change allows easier data manipulation with SIMD
 * instructions (single SIMD register can now handle homogenous data).
 *
 * Also, all information is aligned to 64 bytes, which together with
 * __asssume_aligned() call in nbody.cpp removes the need for unaligned access
 * and thus for scatter/gather emulation.
 */
typedef struct {
    float pos_x[N] __attribute__((aligned(64)));
    float pos_y[N] __attribute__((aligned(64)));
    float pos_z[N] __attribute__((aligned(64)));
    float vel_x[N] __attribute__((aligned(64)));
    float vel_y[N] __attribute__((aligned(64)));
    float vel_z[N] __attribute__((aligned(64)));
    float weight[N] __attribute__((aligned(64)));
} particles_t;

void particles_simulate(particles_t &p);

void particles_read(FILE *fp, particles_t &p);

void particles_write(FILE *fp, particles_t &p);

#endif /* __NBODY_H__ */
