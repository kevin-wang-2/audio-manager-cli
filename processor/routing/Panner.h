#ifndef AUDIO_MANAGER_CLI_PANNER_H
#define AUDIO_MANAGER_CLI_PANNER_H


#include "audioreciever.h"
#include "audiodevice.h"

typedef enum {
    PL_3_db,
    PL_4_5_db,
    PL_6_db
} PanLaw;

class Panner : public AudioDevice, public AudioGenerator, public AudioReciever {
    double lmult, rmult;
    PanLaw pl;
public:
    Panner(double _sampleRate, PanLaw _pl = PL_3_db);

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};

class StereoPanner : public AudioDevice, public AudioGenerator, public AudioReciever {
    double lmultl, lmultr, rmultl, rmultr;
    PanLaw pl;
public:
    StereoPanner(double _sampleRate, PanLaw _pl = PL_3_db);

    // Device Methods
    virtual void setValue(int id, ParameterValue value) override;
    virtual void press(int id) override;

    // Output Audio Calculation
    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};


#endif //AUDIO_MANAGER_CLI_PANNER_H
