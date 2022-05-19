#include "HighPassFilter.h"
#include "sampletimecode.h"
#include <cmath>
#include <cstring>

HighPassFilter::HighPassFilter(double _sampleRate) : AudioGenerator(_sampleRate, TRK_MONO),
                                                     AudioReceiver(_sampleRate, 1, {TRK_MONO}) {
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
    parameters[1].v.val = 1;

    name = "HighPassFilter";
}

void HighPassFilter::setValue(int id, ParameterValue value) {
    /**
     * For Low Pass Filter
     * w_0 = 2 * pi * f_c / f_s
     * alpha = sin(w_0) / (2 * Q)
     *
     * b0 = (1 + cos(w_0)) / 2
     * b1 = -(1 + cos(w_0))
     * b2 = (1 + cos(w_0)) / 2
     * a0 = 1 + alpha
     * a1 = -2 * cos(w_0)
     * a2 = 1 - alpha
     */

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

    } else if (id == 1) {
        // Need not update b0, b1, b2, a1
        parameters[1].v.val = value.val;

        double w_0 = 2 * M_PI * parameters[0].v.val / AudioGenerator::sampleRate;
        double alpha = sin(w_0) / (2 * value.val);

        a0 = 1 + alpha;
        a2 = 1 - alpha;
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
        memset(buffer[0], 0, bufferSize * sizeof(int));
    } else {
        if (lastBufferSize != bufferSize || !prevInput || !prevOutput) {
            // Buffer Size may have changed, reset buffer
            delete[] prevInput;
            delete[] prevOutput;
            prevInput = new double[bufferSize * 2];
            prevOutput = new double[bufferSize * 2];
            memset(prevInput, 0, (bufferSize * 2) * sizeof(double));
            memset(prevOutput, 0, (bufferSize * 2) * sizeof(double));
        }

        lastBufferSize = bufferSize;


        if (swap) swap = 0;
        else swap = 1;

        double xd, xdd, yd, ydd;
        // fetch from buffer
        if (swap) { // Last buffer stores in the first slot
            xd = prevInput[bufferSize - 1];
            yd = prevOutput[bufferSize - 1];
            if (bufferSize > 1) {
                xdd = prevInput[bufferSize - 2];
                ydd = prevOutput[bufferSize - 2];
            } else {
                xdd = prevInput[bufferSize * 2 - 1];
                ydd = prevOutput[bufferSize * 2 - 1];
            }
        } else {
            xd = prevInput[bufferSize * 2 - 1];
            xdd = prevInput[bufferSize * 2 - 2];
            yd = prevOutput[bufferSize * 2 - 1];
            ydd = prevOutput[bufferSize * 2 - 2];
        }

        receiveBuffer(0, buffer, bufferSize);

        double *bufferStart, *outputStart;

        if (swap) {
            bufferStart = prevInput + bufferSize;
            outputStart = prevOutput + bufferSize;
        } else {
            bufferStart = prevInput;
            outputStart = prevOutput;
        }

        for (int offset = 0; offset < bufferSize; offset++) {
            double y = (b0 / a0) * buffer[0][offset] +  (b1 / a0) * xd + (b2 / a0) * xdd - (a1 / a0) * yd - (a2 / a0) * ydd;
            *(bufferStart++) = buffer[0][offset];
            *(outputStart++) = y;
            xdd = xd;
            ydd = yd;
            xd = buffer[0][offset];
            yd = y;
            buffer[0][offset] = y;
        }
    }
}

MultiHighPassFilter::MultiHighPassFilter(double _sampleRate, TrackType type) : AudioGenerator(_sampleRate, type),
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
    parameters[1].v.val = 1;

    prevInput = new double*[trackCnt[type]];
    prevOutput = new double*[trackCnt[type]];

    for (int channel = 0; channel < trackCnt[itracks[0]]; channel++) {
        prevInput[channel] = nullptr;
        prevOutput[channel] = nullptr;
    }

    name = "MultiHighPassFilter";
}


void MultiHighPassFilter::setValue(int id, ParameterValue value) {
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

    } else if (id == 1) {
        // Need not update b0, b1, b2, a1
        parameters[1].v.val = value.val;

        double w_0 = 2 * M_PI * parameters[0].v.val / AudioGenerator::sampleRate;
        double alpha = sin(w_0) / (2 * value.val);

        a0 = 1 + alpha;
        a2 = 1 - alpha;
    }
}

void MultiHighPassFilter::press(int id) {
}

void MultiHighPassFilter::fillBuffer(int track, double **buffer, int bufferSize) {
    /**
     * Transfer function: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (a0 + a1*z^-1 + a2*z^-2)
     * Time-domain rep: y[n] = (b0 / a0) * x[n] + (b1 / a0) * x[n-1] + (b2 / a0) * x[n-2] - (a1 / a0) * y[n-1] - (a2 / a0) * y[n-2]
     */

    if (noconnection(track)) {
        for (int channel = 0; channel < trackCnt[itracks[0]]; channel++)
            memset(buffer[channel], 0, bufferSize * sizeof(int));
    } else {
        receiveBuffer(0, buffer, bufferSize);

        if (swap) swap = 0;
        else swap = 1;

        for (int channel = 0; channel < trackCnt[itracks[0]]; channel++) {
            if (lastBufferSize != bufferSize || !prevInput[channel] || !prevOutput[channel]) {
                // Buffer Size may have changed, reset buffer
                delete[] prevInput[channel];
                delete[] prevOutput[channel];
                prevInput[channel] = new double[bufferSize * 2];
                prevOutput[channel] = new double[bufferSize * 2];
                memset(prevInput[channel], 0, (bufferSize * 2) * sizeof(double));
                memset(prevOutput[channel], 0, (bufferSize * 2) * sizeof(double));
            }

            double xd, xdd, yd, ydd;
            // fetch from buffer
            if (swap) { // Last buffer stores in the first slot
                xd = prevInput[channel][bufferSize - 1];
                yd = prevOutput[channel][bufferSize - 1];
                if (bufferSize > 1) {
                    xdd = prevInput[channel][bufferSize - 2];
                    ydd = prevOutput[channel][bufferSize - 2];
                } else {
                    xdd = prevInput[channel][bufferSize * 2 - 1];
                    ydd = prevOutput[channel][bufferSize * 2 - 1];
                }
            } else {
                xd = prevInput[channel][bufferSize * 2 - 1];
                xdd = prevInput[channel][bufferSize * 2 - 2];
                yd = prevOutput[channel][bufferSize * 2 - 1];
                ydd = prevOutput[channel][bufferSize * 2 - 2];
            }

            double *bufferStart, *outputStart;

            if (swap) {
                bufferStart = prevInput[channel] + bufferSize;
                outputStart = prevOutput[channel] + bufferSize;
            } else {
                bufferStart = prevInput[channel];
                outputStart = prevOutput[channel];
            }

            for (int offset = 0; offset < bufferSize; offset++) {
                double y = (b0 / a0) * buffer[channel][offset] + (b1 / a0) * xd + (b2 / a0) * xdd - (a1 / a0) * yd -
                           (a2 / a0) * ydd;
                *(bufferStart++) = buffer[channel][offset];
                *(outputStart++) = y;
                xdd = xd;
                ydd = yd;
                xd = buffer[channel][offset];
                yd = y;
                buffer[channel][offset] = y;
            }
        }

        lastBufferSize = bufferSize;
    }
}