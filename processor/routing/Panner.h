#ifndef AUDIO_MANAGER_CLI_PANNER_H
#define AUDIO_MANAGER_CLI_PANNER_H


#include "audioreciever.h"
#include "audiodevice.h"

class Panner : public AudioDevice, public AudioGenerator, public AudioReciever {
    double lmult, rmult;
public:
    Panner(double _sampleRate);

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};


#endif //AUDIO_MANAGER_CLI_PANNER_H
