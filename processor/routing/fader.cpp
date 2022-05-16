#include "fader.h"
#include <cstring>
#include <cmath>

Fader::Fader(double _sampleRate, TrackType type) : AudioGenerator(_sampleRate, type), AudioReciever(_sampleRate, 1, {type}) {
    parameters.resize(2);

    // Parameter0 Mute (ON/OFF)
    parameters[0].type = DP_Switch;
    parameters[0].id = 0;
    parameters[0].name = "Mute";
    parameters[0].vtype = PV_Enum;
    parameters[0].list = {"Off", "On"};
    parameters[0].v.enu = 0;

    // Parameter1 Fader (-Inf-12.0
    parameters[1].type = DP_Variable;
    parameters[1].id = 1;
    parameters[1].name = "Fader";
    parameters[1].vtype = PV_Single_Bounded;
    parameters[1].range.max = 12.0;
    parameters[1].v.val = 0.0;

    // Name
    name = "Fader";
}

#include <iostream>

// Device Methods
void Fader::setValue(int id, ParameterValue value) {
    switch(id) {
    case 0:
        if(value.val == 0.0 || value.enu == 0) {
            parameters[0].v.enu = 0;
        } else {
            parameters[0].v.enu = 1;
        }
        break;
    case 1:
        parameters[1].v.val = value.val;
        break;
    }
}
void Fader::press(int id) {
    if (id == 0) {
        parameters[0].v.enu = !parameters[0].v.enu;
    }
};

// Output Audio Calculation
void Fader::fillBuffer(int, double *buffer[], int bufferSize) {
    if (parameters[0].v.enu == 1 || noconnection(0)) {
        for (int i = 0; i < trackCnt[otracks[0]]; i++)
            memset(buffer[i], 0, bufferSize * sizeof(int));
    } else {
        // 1. Calculate Amplitude Multiplier
        double mult = pow(10, parameters[1].v.val / 20);

        // 2. Get upstream buffer
        recieveBuffer(0, buffer, bufferSize);

        // 3. Calculate and fill buffer
        for (int i = 0; i < trackCnt[otracks[0]]; i++)
            for (int j = 0; j < bufferSize; j++) buffer[i][j] *= mult;
    }
};
