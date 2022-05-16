#ifndef AUDIO_MANAGER_CLI_IMAGER_H
#define AUDIO_MANAGER_CLI_IMAGER_H

#include "audiodevice.h"
#include "audiogenerator.h"
#include "audioreciever.h"

class Imager : public AudioDevice, public AudioGenerator, public AudioReceiver {
public:
    Imager(double _sampleRate);

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};


#endif //AUDIO_MANAGER_CLI_IMAGER_H
