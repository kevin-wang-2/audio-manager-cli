#include "sine.h"
#include "sampletimecode.h"
#include <cmath>
#include <cstring>

void Sine::fillBuffer(int, double *buffer[], int bufferSize)
{
    double temp[bufferSize];

    double omega = freq * 2 * 3.1415926535897932384626;
    double sample = 1000000.0 / sampleRate;


    double microSecs = SampleTimeCode::get() * sample;

    for(int i = 0; i < bufferSize; i++)
    {
        temp[i] = amplitude * sin((microSecs + sample * i) * omega / 1000000.0 + phase);
    }

    for (int i = 0; i < trackCnt[otracks[0]]; i++) {
        memcpy(buffer[i], temp, bufferSize * sizeof(double));
    }
}
