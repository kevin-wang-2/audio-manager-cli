#ifndef SINE_H
#define SINE_H

/**
 * Created by WKB in 2022/4/21
 * Simple Sine-wave generator, basic testing unit
 */


#include "audiogenerator.h"
#include <limits.h>

class Sine : public AudioGenerator
{
    int amplitude;
    int phase;
    int freq;
public:
    Sine(double _sampleRate, TrackType _type = TRK_STEREO, int _amplitude = INT_MAX, int _phase = 0, int _freq = 440) :
        AudioGenerator(_sampleRate, 1, {_type}), amplitude(_amplitude), phase(_phase), freq(_freq) {}

    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};

#endif // SINE_H
