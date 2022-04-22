#ifndef FADER_H
#define FADER_H

/**
 * Created by WKB in 2022/4/22
 * Simple Fader
 */

#include "audiodevice.h"
#include "audiogenerator.h"
#include "audioreciever.h"

class Fader : public AudioDevice, public AudioGenerator, public AudioReciever
{
public:
    Fader(double _sampleRate, TrackType type);

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};

#endif // FADER_H
