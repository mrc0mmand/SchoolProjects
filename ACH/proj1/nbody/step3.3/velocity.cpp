/*
 * Architektura procesoru (ACH 2016)
 * Projekt c. 1 (nbody)
 * Login: xsumsa01
 */

#include <cmath>
#include <cfloat>
#include <cstdlib>
#include "velocity.h"

/**
 * @brief Calculate gravitational velocity and collision velocity difference
 *        for particles p1 and p2
 *
 * @details This function is a result of merge of calculate_gravitation_velocity
 *          and calculate_collision_velocity from previous steps
 */
void calculate_velocity(
  float pos1_x, float pos1_y, float pos1_z, float vel1_x, float vel1_y,
  float vel1_z, float weight1,
  float pos2_x, float pos2_y, float pos2_z, float vel2_x, float vel2_y,
  float vel2_z, float weight2,
  float &v_x, float &v_y, float &v_z
)
{
    float r, dx, dy, dz;
    float vx, vy, vz;

    dx = pos2_x - pos1_x;
    dy = pos2_y - pos1_y;
    dz = pos2_z - pos1_z;

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

        float f = (G * weight1 * weight2) / (r * r);

        vx = r != 0.0f ? (((f * (dx/r)) / weight1) * DT) : 0.0f;
        vy = r != 0.0f ? (((f * (dy/r)) / weight1) * DT) : 0.0f;
        vz = r != 0.0f ? (((f * (dz/r)) / weight1) * DT) : 0.0f;

        v_x += vx;
        v_y += vy;
        v_z += vz;
    } else if(r > 0.0f && r < COLLISION_DISTANCE) {
        /* Collision velocity:
         *      w1 = (m1 - m2) * v1 / M + 2 * m2 * v2 / M
         *  where m1 and m2 are masses of particles, v1 and v2 are velocities, and
         *  M is the center of mass calculated as m1 + m2
         */

        float mtot = weight1 + weight2;
        float wdif = weight1 - weight2;

        vx = ((wdif * vel1_x / mtot) + 2 * (weight2 * vel2_x) / mtot) - vel1_x;
        vy = ((wdif * vel1_y / mtot) + 2 * (weight2 * vel2_y) / mtot) - vel1_y;
        vz = ((wdif * vel1_z / mtot) + 2 * (weight2 * vel2_z) / mtot) - vel1_z;

        v_x += vx;
        v_y += vy;
        v_z += vz;
    }
}
