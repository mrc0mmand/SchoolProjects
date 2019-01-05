/*
 * Architektura procesoru (ACH 2016)
 * Projekt c. 1 (nbody)
 * Login: xsumsa01
 */

#ifndef __VELOCITY_H__
#define __VELOCITY_H__

#include <cstdlib>
#include <cstdio>

/* gravitacni konstanta */
constexpr float G = 6.67384e-11f;

/* collision threshold */
constexpr float COLLISION_DISTANCE = 0.01f;

/* struktura castice (hmotneho bodu) */
struct t_particle
{
    float pos_x;
    float pos_y;
    float pos_z;
    float vel_x;
    float vel_y;
    float vel_z;
    float weight;
};

/* vektor zmeny rychlosti */
struct t_velocity
{
    float x;
    float y;
    float z;
};

// Create SIMD versions of the function
#pragma omp declare simd
void calculate_velocity(
  float pos1_x, float pos1_y, float pos1_z, float vel1_x, float vel1_y,
  float vel1_z, float weight1,
  float pos2_x, float pos2_y, float pos2_z, float vel2_x, float vel2_y,
  float vel2_z, float weight2,
  float &v_x, float &v_y, float &v_z
);

#endif /* __VELOCITY_H__ */
