#ifndef AUDIO_MANAGER_CLI_HIGHPASSFILTER_H
#define AUDIO_MANAGER_CLI_HIGHPASSFILTER_H

#include "audiodevice.h"
#include "audiogenerator.h"
#include "audioreciever.h"

class HighPassFilter : public AudioDevice, public AudioGenerator, public AudioReceiver{
    double a0 = 1, a1 = 0, a2 = 0, b0 = 1, b1 = 0, b2 = 0;
    int lastBufferSize = 0;
    double *prevInput = nullptr, *prevOutput = nullptr;
    int swap = 1;
public:
    HighPassFilter(double _sampleRate);
    ~HighPassFilter() {
        delete prevInput;
        delete prevOutput;
    }

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;

};

class MultiHighPassFilter : public AudioDevice, public AudioGenerator, public AudioReceiver{
    double a0 = 1, a1 = 0, a2 = 0, b0 = 1, b1 = 0, b2 = 0;
    int lastBufferSize = 0;
    double **prevInput = nullptr, **prevOutput = nullptr;
    int swap = 1;
public:
    MultiHighPassFilter(double _sampleRate, TrackType type);
    ~MultiHighPassFilter() {
        for (int channel = 0; channel < trackCnt[itracks[0]]; channel++) {
            delete[] prevInput[channel];
            delete[] prevOutput[channel];
        }
        delete prevInput;
        delete prevOutput;
    }

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;

};


#endif //AUDIO_MANAGER_CLI_HIGHPASSFILTER_H
