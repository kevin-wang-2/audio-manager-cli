#ifndef AUDIO_MANAGER_CLI_HIGHPASSFILTER_H
#define AUDIO_MANAGER_CLI_HIGHPASSFILTER_H

#include "audiodevice.h"
#include "audiogenerator.h"
#include "audioreciever.h"
#include "QuadFilter.h"

class HighPassFilter : public AudioDevice, public AudioGenerator, public AudioReceiver{
    double a0 = 1, a1 = 0, a2 = 0, b0 = 1, b1 = 0, b2 = 0;

    int lastBufferSize;

    BiQuad *filters;
public:
    HighPassFilter(double _sampleRate, TrackType type);
    ~HighPassFilter() {
        delete[] filters;
    }

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;

};


#endif //AUDIO_MANAGER_CLI_HIGHPASSFILTER_H
