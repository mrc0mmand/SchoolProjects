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
 * @breef   Funkce vypocte rychlost kterou teleso p1 ziska vlivem gravitace p2.
 * @details Viz zadani.pdf
 */
void calculate_gravitation_velocity(
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

    /* MISTO PRO VAS KOD GRAVITACE */

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

    /* KONEC */

    vel.x += (r > COLLISION_DISTANCE) ? vx : 0.0f;
    vel.y += (r > COLLISION_DISTANCE) ? vy : 0.0f;
    vel.z += (r > COLLISION_DISTANCE) ? vz : 0.0f;
}

/**
 * @breef   Funkce vypocte rozdil mezi rychlostmi pred a po kolizi telesa p1 do telesa p2.
 */
void calculate_collision_velocity(
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

    /* MISTO PRO VAS KOD KOLIZE */

    /* Collision velocity:
     *      w1 = (m1 - m2) * v1 / M + 2 * m2 * v2 / M
     *  where m1 and m2 are masses of particles, v1 and v2 are velocities, and
     *  M is the center of mass calculated as m1 + m2
     */

    vx = (((p1.weight - p2.weight) * p1.vel_x / (p1.weight + p2.weight)) + 2 * (p2.weight * p2.vel_x) / (p1.weight + p2.weight)) - p1.vel_x;
    vy = (((p1.weight - p2.weight) * p1.vel_y / (p1.weight + p2.weight)) + 2 * (p2.weight * p2.vel_y) / (p1.weight + p2.weight)) - p1.vel_y;
    vz = (((p1.weight - p2.weight) * p1.vel_z / (p1.weight + p2.weight)) + 2 * (p2.weight * p2.vel_z) / (p1.weight + p2.weight)) - p1.vel_z;

    /* KONEC */

    // jedna se o rozdilne ale blizke prvky
    if (r > 0.0f && r < COLLISION_DISTANCE) {
        vel.x += vx;
        vel.y += vy;
        vel.z += vz;
    }
}
