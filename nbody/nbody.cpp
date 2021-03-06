/**
* @file nbody.cpp
*
* N-Body Simuluation
* Kernel of particles simulation program
*
* @author: Dominik Simek <xsimek23@stud.fit.vutbr.cz
*
*/

#include <cfloat>
#include <cmath>
#include <omp.h>
#include <algorithm>
#include "nbody.h"

const float MIN_FLT = 1e-37f;
const float SML_FLT = 1e-9f;

/**
* Allocation of aligned arrays
* Align = 64B
*/
t_particles_DA *particles_alloc(size_t size)
{
    // pointer to structure
    t_particles_DA *p_ret = (t_particles_DA*) _mm_malloc(sizeof(t_particles_DA), 64);
    if(p_ret == NULL)
    {
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // allocate for pre pos_x
    p_ret->pos_x = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->pos_x == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for pos_y
    p_ret->pos_y = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->pos_y == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for pos_z
    p_ret->pos_z = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->pos_z == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for vel_x
    p_ret->vel_x = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->vel_x == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for vel_y
    p_ret->vel_y = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->vel_y == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for vel_z
    p_ret->vel_z = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->vel_z == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for weight
    p_ret->weight = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->weight == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // fx
    p_ret->fx = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->fx == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for fy
    p_ret->fy = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->fy == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for fz
    p_ret->fz = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->fz == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for ax
    p_ret->ax = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->ax == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for ay
    p_ret->ay = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->ay == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    // alocate for az
    p_ret->az = (float*) _mm_malloc(size * sizeof(float), 64);
    if(p_ret->az == NULL)
    {
        _mm_free(p_ret);
        fprintf(stderr, "_mm_malloc() error !!!");
        return NULL;
    }

    return p_ret;
}

/**
* Free particles from memory
*/
void particles_free(t_particles_DA *p)
{
    if(p != NULL)
    {
        // free arrays of each pieces
        _mm_free(p->pos_x);
        _mm_free(p->pos_y);
        _mm_free(p->pos_z);
        _mm_free(p->vel_x);
        _mm_free(p->vel_y);
        _mm_free(p->vel_z);
        _mm_free(p->weight);
        _mm_free(p->fx);
        _mm_free(p->fy);
        _mm_free(p->fz);
        _mm_free(p->ax);
        _mm_free(p->ay);
        _mm_free(p->az);

        // free structure pointer
        _mm_free(p);
    }

    return;
}
/**
* Initialize particle's structure 
* Numa First Touch Policy
*/
void particles_init(t_particles_DA p)
{
    int i;

    #pragma omp parallel for
    for(i = 0; i < N; i++)
    {
        p.pos_x[i] = 0.0f;
        p.pos_y[i] = 0.0f;
        p.pos_z[i] = 0.0f;
        p.vel_x[i] = 0.0f;
        p.vel_y[i] = 0.0f;
        p.vel_z[i] = 0.0f;
        p.weight[i] = 0.0f;
    }

    return;
}

/**
* Simulation kernel
*/
void particles_simulate(t_particles_DA p)
{
    #pragma omp parallel
    {
        // each steps of simulation
        for(uint step = 0; step < STEPS; step++)
        {
            #pragma omp for
            //#pragma vector aligned
            #pragma simd
            for(uint j = 0; j < N; j++)
            {
                p.fx[j] = 0.0f;
                p.fy[j] = 0.0f;
                p.fz[j] = 0.0f;
            }

            // iterate over all bodies
            #pragma omp for
            #pragma vector aligned
            for(uint i = 0; i < N; i++)
            {
                float dx, dy, dz;   // distance between bodies X, Y, Z
                float dist_sqr;     // distance^2
                float inv_dist;     // inverted distance
                float inv_dist3;    // inverted distance^3

                #pragma vector aligned
                #pragma simd
                for(uint j = 0; j < N; j++)
                {
                    // distance between bodies
                    dx = p.pos_x[j] - p.pos_x[i]; // x
                    dy = p.pos_y[j] - p.pos_y[i]; // y
                    dz = p.pos_z[j] - p.pos_z[i]; // z

                    // distance between bodies^2
                    dist_sqr = dx*dx + dy*dy + dz*dz + SML_FLT;

                    // inverted distance
                    inv_dist = 1.0f / sqrt(dist_sqr);

                    // inverted distance^3
                    inv_dist3 = inv_dist * inv_dist * inv_dist;

                    // compute increment of force
                    p.fx[i] += dx * ((p.weight[i] * p.weight[j] * G) * inv_dist3);
                    p.fy[i] += dy * ((p.weight[i] * p.weight[j] * G) * inv_dist3);
                    p.fz[i] += dz * ((p.weight[i] * p.weight[j] * G) * inv_dist3);

                }

            }

            // new acceleration
            #pragma omp for
            //#pragma vector aligned
            #pragma simd
            for(uint i = 0; i < N; i++)
            {
                p.ax[i] = p.fx[i] / p.weight[i];
                p.ay[i] = p.fy[i] / p.weight[i];
                p.az[i] = p.fz[i] / p.weight[i];
            }

            // new velocity
            #pragma omp for
            //#pragma vector aligned
            #pragma simd
            for(uint i = 0; i < N; i++)
            {
                p.vel_x[i] += DT * p.ax[i];
                p.vel_y[i] += DT * p.ay[i];
                p.vel_z[i] += DT * p.az[i];

            }

            // new position
            #pragma omp for
            //#pragma vector aligned
            #pragma simd
            for(uint i = 0; i < N; i++)
            {
               p.pos_x[i] += p.vel_x[i] * DT;
               p.pos_y[i] += p.vel_y[i] * DT;
               p.pos_z[i] += p.vel_z[i] * DT;
            }

        }
    }

    return;
}

/**
* Read particles from file
*/
void particles_read(FILE *fp, t_particles_DA p)
{
    int i;
    for (i = 0; i < N; i++)
    {
        fscanf(fp, "%f %f %f %f %f %f %f \n",
            &p.pos_x[i], &p.pos_y[i], &p.pos_z[i],
            &p.vel_x[i], &p.vel_y[i], &p.vel_z[i],
            &p.weight[i]);
    }

    return;
}

/**
* Write particles to file
*/
void particles_write(FILE *fp, t_particles_DA p)
{
    int i;
    for (i = 0; i < N; i++)
    {
        fprintf(fp, "%10.4f %10.4f %10.4f %10.4f %10.4f %10.4f %10.4f \n",
            p.pos_x[i], p.pos_y[i], p.pos_z[i],
            p.vel_x[i], p.vel_y[i], p.vel_z[i],
            p.weight[i]);
    }

    return;
}
