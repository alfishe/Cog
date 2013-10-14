#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "lanczos_resampler.h"

enum { LANCZOS_RESOLUTION = 8192 };
enum { LANCZOS_WIDTH = 8 };
enum { LANCZOS_SAMPLES = LANCZOS_RESOLUTION * LANCZOS_WIDTH };

static double lanczos_lut[LANCZOS_SAMPLES + 1];

enum { lanczos_buffer_size = LANCZOS_WIDTH * 4 };

int fEqual(const double b, const double a)
{
    return fabs(a - b) < 1.0e-6;
}

static double sinc(double x)
{
    return fEqual(x, 0.0) ? 1.0 : sin(x * M_PI) / (x * M_PI);
}

void lanczos_init()
{
    unsigned i;
    double dx = (double)(LANCZOS_WIDTH) / LANCZOS_SAMPLES, x = 0.0;
    for (i = 0; i < LANCZOS_SAMPLES + 1; ++i, x += dx)
        lanczos_lut[i] = fabs(x) < LANCZOS_WIDTH ? sinc(x) * sinc(x / LANCZOS_WIDTH) : 0.0;
}

typedef struct lanczos_resampler
{
    long write_pos, write_filled;
    long read_pos, read_filled;
    unsigned short phase;
    unsigned long phase_inc;
    float buffer_in[lanczos_buffer_size * 2];
    int buffer_out[lanczos_buffer_size];
} lanczos_resampler;

void * lanczos_resampler_create()
{
    lanczos_resampler * r = ( lanczos_resampler * ) malloc( sizeof(lanczos_resampler) );
    if ( !r ) return 0;

    r->write_pos = 0;
    r->write_filled = 0;
    r->read_pos = 0;
    r->read_filled = 0;
    r->phase = 0;
    r->phase_inc = 0;
    memset( r->buffer_in, 0, sizeof(r->buffer_in) );
    memset( r->buffer_out, 0, sizeof(r->buffer_out) );

    return r;
}

void lanczos_resampler_delete(void * _r)
{
    free( _r );
}

long lanczos_resampler_get_free_count(void *_r)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;
    return lanczos_buffer_size - r->write_filled;
}

long lanczos_resampler_ready(void *_r)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;
    return r->write_filled > (LANCZOS_WIDTH * 2);
}

void lanczos_resampler_clear(void *_r)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;
    r->write_pos = 0;
    r->write_filled = 0;
    r->read_pos = 0;
    r->read_filled = 0;
    r->phase = 0;
}

void lanczos_resampler_set_rate(void *_r, double new_factor)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;
    r->phase_inc = (long)( new_factor * LANCZOS_RESOLUTION );
}

void lanczos_resampler_write_sample(void *_r, int s)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;

    if ( r->write_filled < lanczos_buffer_size )
    {
        float s32 = (float)s;

        r->buffer_in[ r->write_pos ] = s32;
        r->buffer_in[ r->write_pos + lanczos_buffer_size ] = s32;

        ++r->write_filled;

        r->write_pos = ( r->write_pos + 1 ) % lanczos_buffer_size;
    }
}

static long lanczos_resampler_run(lanczos_resampler * r, int ** out_, int * out_end)
{
    long in_size = r->write_filled;
    float const* in_ = r->buffer_in + lanczos_buffer_size + r->write_pos - r->write_filled;
    long used = 0;
    in_size -= LANCZOS_WIDTH * 2;
    if ( in_size > 0 )
    {
        int* out = *out_;
        float const* in = in_;
        float const* const in_end = in + in_size;
        long phase = r->phase;
        long phase_inc = r->phase_inc;

        long step = phase_inc > LANCZOS_RESOLUTION ? LANCZOS_RESOLUTION * LANCZOS_RESOLUTION / phase_inc : LANCZOS_RESOLUTION;

        do
        {
            // accumulate in extended precision
            double kernel[LANCZOS_WIDTH * 2], kernel_sum = 0.0;
            long i = LANCZOS_WIDTH;
            long phase_adj = phase * step / LANCZOS_RESOLUTION;
            double sample;

            if ( out >= out_end )
                break;

            for (; i >= -LANCZOS_WIDTH + 1; --i)
            {
                long pos = i * step;
                kernel_sum += kernel[i + LANCZOS_WIDTH - 1] = lanczos_lut[labs(phase_adj - pos)];
            }
            for (sample = 0, i = 0; i < LANCZOS_WIDTH * 2; ++i)
                sample += in[i] * kernel[i];
            *out++ = (int) (sample / kernel_sum);

            phase += phase_inc;

            in += phase >> 13;

            phase &= 8191;
        }
        while ( in < in_end );

        r->phase = phase;
        *out_ = out;

        used = in - in_;

        r->write_filled -= used;
    }

    return used;
}

static void lanczos_resampler_fill(lanczos_resampler * r)
{
    while ( r->write_filled > (LANCZOS_WIDTH * 2) &&
            r->read_filled < lanczos_buffer_size )
    {
        long write_pos = ( r->read_pos + r->read_filled ) % lanczos_buffer_size;
        long write_size = lanczos_buffer_size - write_pos;
        int * out = r->buffer_out + write_pos;
        if ( write_size > ( lanczos_buffer_size - r->read_filled ) )
            write_size = lanczos_buffer_size - r->read_filled;
        lanczos_resampler_run( r, &out, out + write_size );
        r->read_filled += out - r->buffer_out - write_pos;
    }
}

long lanczos_resampler_get_sample_count(void *_r)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;
    if ( r->read_filled < 1 )
        lanczos_resampler_fill( r );
    return r->read_filled;
}

int lanczos_resampler_get_sample(void *_r)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;
    if ( r->read_filled < 1 )
        lanczos_resampler_fill( r );
    if ( r->read_filled < 1 )
        return 0;
    return r->buffer_out[ r->read_pos ];
}

void lanczos_resampler_remove_sample(void *_r)
{
    lanczos_resampler * r = ( lanczos_resampler * ) _r;
    if ( r->read_filled > 0 )
    {
        --r->read_filled;
        r->read_pos = ( r->read_pos + 1 ) % lanczos_buffer_size;
    }
}