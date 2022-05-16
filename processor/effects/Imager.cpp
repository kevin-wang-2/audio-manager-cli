#include "Imager.h"
#include <cstring>
#include <cmath>

Imager::Imager(double _sampleRate) : AudioGenerator(_sampleRate, TRK_STEREO),
                                     AudioReceiver(_sampleRate, 1, {TRK_STEREO}) {
    parameters.resize(1);

    // Parameter0 Mute (ON/OFF)
    parameters[0].type = DP_Variable;
    parameters[0].id = 0;
    parameters[0].name = "Width";
    parameters[0].vtype = PV_Double_Bounded;
    parameters[0].range.range[0] = 0;
    parameters[0].range.range[1] = 200;
    parameters[0].v.val = 100;

    // Name
    name = "Stereo Imager";
}


// Device Methods
void Imager::setValue(int id, ParameterValue value) {
    if (id == 0) {
        if (value.val) {
            parameters[0].v.val = value.val;
        }
    }
}

void Imager::press(int id) {
};

// Output Audio Calculation
void Imager::fillBuffer(int, double *buffer[], int bufferSize) {
    if (noconnection(0)) {
        for (int i = 0; i < trackCnt[otracks[0]]; i++)
            memset(buffer[i], 0, bufferSize * sizeof(int));
    } else {
        /**
         * We use M/S processing to make this imager work.
         * M = L + R;
         * S = L - R;
         *
         * Then, according to the requirement,
         * S = S * width;
         *
         * Then, recover the L and R from the M and S
         */
        // 1. Get upstream buffer
        receiveBuffer(0, buffer, bufferSize);

        // 2. Calculate each buffer
        for (int i = 0; i < bufferSize; i++) {
            double M, S;
            M = buffer[0][i] + buffer[1][i];
            S = (buffer[0][i] - buffer[1][i]) * parameters[0].v.val / 100;

            buffer[0][i] = (M + S) / 2;
            buffer[1][i] = (M - S) / 2;
        }
    }
};