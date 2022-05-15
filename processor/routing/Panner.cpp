#include "Panner.h"
#include <cstring>
#include <cmath>

Panner::Panner(double _sampleRate, PanLaw _pl) : AudioGenerator(_sampleRate, TRK_MONO),
                                     AudioReciever(_sampleRate, 1, {TRK_STEREO}),
                                     pl(_pl){
    parameters.resize(1);

    // Parameter0 Mute (ON/OFF)
    parameters[0].type = DP_Variable;
    parameters[0].id = 0;
    parameters[0].name = "Pan";
    parameters[0].vtype = PV_Double_Bounded;
    parameters[0].range.range[0] = -45;
    parameters[0].range.range[1] = 45;
    parameters[0].v.val = 0;

    lmult = 1;
    rmult = 1;

    // Name
    name = "Mono Panner";
}

void Panner::setValue(int id, ParameterValue value) {
    if (id == 0) {
        parameters[0].v.val = value.val;

        switch (pl) {
            case PL_3_db:
                lmult = cos((value.val + 45) * M_PI / 180);
                rmult = sin((value.val + 45) * M_PI / 180);
                break;
            case PL_4_5_db:
                lmult = sqrt(cos((value.val + 45) * M_PI / 180) * (45 - value.val) / 90);
                rmult = sqrt(sin((value.val + 45) * M_PI / 180) * (value.val + 45) / 90);
                break;
            case PL_6_db:
                lmult = (45 - value.val) / 90;
                rmult = (value.val + 45) / 90;
                break;
        }
    }
}

void Panner::press(int id) {
}

void Panner::fillBuffer(int track, double **buffer, int bufferSize) {
    if (noconnection(0)) {
        for (int i = 0; i < trackCnt[otracks[0]]; i++)
            memset(buffer[i], 0, bufferSize * sizeof(int));
    } else {
        // Directly use the lmult and rmult to calculate the desired L and R response
        // 1. Receive Buffer
        recieveBuffer(0, buffer, bufferSize);

        // 2. Calculate multiplications
        for (int i = 0; i < bufferSize; i++) {
            buffer[0][i] *= lmult;
            buffer[1][i] *= rmult;
        }
    }
}
