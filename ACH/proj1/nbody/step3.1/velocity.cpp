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
  const t_particle &p1,
  const t_particle &p2,
  t_velocity &vel
)
{
    float r, dx, dy, dz;
    float vx, vy, vz;

    dx = p2.pos_x - p1.pos_x;
    dy = p2.pos_y - p1.pos_y;
    dz = p2.pos_z - p1.pos_z;

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

        float f = (G * p1.weight * p2.weight) / (r * r);

        vx = r != 0.0f ? (((f * (dx/r)) / p1.weight) * DT) : 0.0f;
        vy = r != 0.0f ? (((f * (dy/r)) / p1.weight) * DT) : 0.0f;
        vz = r != 0.0f ? (((f * (dz/r)) / p1.weight) * DT) : 0.0f;

        vel.x += vx;
        vel.y += vy;
        vel.z += vz;
    } else if(r > 0.0f && r < COLLISION_DISTANCE) {
        /* Collision velocity:
         *      w1 = (m1 - m2) * v1 / M + 2 * m2 * v2 / M
         *  where m1 and m2 are masses of particles, v1 and v2 are velocities, and
         *  M is the center of mass calculated as m1 + m2
         */

        float mtot = p1.weight + p2.weight;
        float wdif = p1.weight - p2.weight;

        vx = ((wdif * p1.vel_x / mtot) + 2 * (p2.weight * p2.vel_x) / mtot) - p1.vel_x;
        vy = ((wdif * p1.vel_y / mtot) + 2 * (p2.weight * p2.vel_y) / mtot) - p1.vel_y;
        vz = ((wdif * p1.vel_z / mtot) + 2 * (p2.weight * p2.vel_z) / mtot) - p1.vel_z;

        vel.x += vx;
        vel.y += vy;
        vel.z += vz;
    }
}
