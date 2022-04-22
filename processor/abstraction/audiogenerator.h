#ifndef AUDIOGENERATOR_H
#define AUDIOGENERATOR_H

/**
 * Created by WKB in 2022/4/21
 * Abstraction of a general Audio Generator, i.e. any device that would output some audio
 */

#include "audiometa.h"
#include <vector>

class AudioGenerator
{
protected:
    double sampleRate;
    int otrackNum;
    std::vector<TrackType> otracks;

public:
    AudioGenerator(double _sampleRate, int _trackNum = 1, std::vector<TrackType> _tracks = {TRK_STEREO})
        : sampleRate(_sampleRate), otrackNum(_trackNum), otracks(std::move(_tracks)) {};

// Core Functions
    virtual void fillBuffer(int track, double *buffer[], int bufferSize);

// Getters
    virtual int getTrackNum() { return otrackNum; }
    virtual TrackType getType(int track = 0) { return otracks[0]; }
};

#endif // AUDIOGENERATOR_H
