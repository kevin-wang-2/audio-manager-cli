#include "HighPassFilter.h"
#include <cmath>
#include <cstring>

HighPassFilter::HighPassFilter(double _sampleRate, TrackType type) : AudioGenerator(_sampleRate, type),
                                                                     AudioReceiver(_sampleRate, 1, {type})  {
    parameters.resize(2);

    parameters[0].type = DP_Variable;
    parameters[0].id = 0;
    parameters[0].name = "Cutoff";
    parameters[0].vtype = PV_Double_Bounded;
    parameters[0].range.range[0] = 0;
    parameters[0].range.range[1] = 20000;
    parameters[0].v.val = 0;

    parameters[1].type = DP_Variable;
    parameters[1].id = 0;
    parameters[1].name = "Q";
    parameters[1].vtype = PV_Double_Bounded;
    parameters[1].range.range[0] = 0;
    parameters[1].range.range[1] = 12;
    parameters[1].v.val = 0.707;

    filters = new BiQuad[trackCnt[type]];

    for (int channel = 0; channel < trackCnt[type]; channel++) {
        filters[channel].b0 = 1;
        filters[channel].b1 = 0;
        filters[channel].b2 = 0;
        filters[channel].a1 = 0;
        filters[channel].a2 = 0;
    }

    name = "HighPassFilter";
}


void HighPassFilter::setValue(int id, ParameterValue value) {
    if (id == 0) {
        parameters[0].v.val = value.val;

        double w_0 = 2 * M_PI * value.val / AudioGenerator::sampleRate;
        double alpha = sin(w_0) / (2 * parameters[1].v.val);

        b0 = (1 + cos(w_0)) / 2;
        b1 = -(1 + cos(w_0));
        b2 = (1 + cos(w_0)) / 2;
        a0 = 1 + alpha;
        a1 = -2 * cos(w_0);
        a2 = 1 - alpha;

        for (int channel = 0; channel < trackCnt[itracks[0]]; channel++) {
            filters[channel].b0 = b0 / a0;
            filters[channel].b1 = b1 / a0;
            filters[channel].b2 = b2 / a0;
            filters[channel].a1 = a1 / a0;
            filters[channel].a2 = a2 / a0;
        }

    } else if (id == 1) {
        // Need not update b0, b1, b2, a1
        parameters[1].v.val = value.val;

        double w_0 = 2 * M_PI * parameters[0].v.val / AudioGenerator::sampleRate;
        double alpha = sin(w_0) / (2 * value.val);

        a0 = 1 + alpha;
        a2 = 1 - alpha;

        for (int channel = 0; channel < trackCnt[itracks[0]]; channel++) {
            filters[channel].b0 = b0 / a0;
            filters[channel].b1 = b1 / a0;
            filters[channel].b2 = b2 / a0;
            filters[channel].a1 = a1 / a0;
            filters[channel].a2 = a2 / a0;
        }
    }
}

void HighPassFilter::press(int id) {
}

void HighPassFilter::fillBuffer(int track, double **buffer, int bufferSize) {
    /**
     * Transfer function: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (a0 + a1*z^-1 + a2*z^-2)
     * Time-domain rep: y[n] = (b0 / a0) * x[n] + (b1 / a0) * x[n-1] + (b2 / a0) * x[n-2] - (a1 / a0) * y[n-1] - (a2 / a0) * y[n-2]
     */

    if (noconnection(track)) {
        for (int channel = 0; channel < trackCnt[itracks[0]]; channel++)
            memset(buffer[channel], 0, bufferSize * sizeof(int));
    } else {
        receiveBuffer(0, buffer, bufferSize);

        for (int channel = 0; channel < trackCnt[itracks[0]]; channel++) {
            if (lastBufferSize != bufferSize) {
                filters[channel].reset();
            }

            for (int offset = 0; offset < bufferSize; offset++) {
                buffer[channel][offset] = filters[channel].calculateSample(buffer[channel][offset]);
            }
        }

        lastBufferSize = bufferSize;
    }
}