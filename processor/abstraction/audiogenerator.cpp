#include "audiogenerator.h"
#include <cstring>

void AudioGenerator::fillBuffer(int track, double *buffer[], int bufferSize) {
    for (int i = 0; i < trackCnt[otracks[0]]; i++) memset(buffer[i], 0, bufferSize * sizeof(double));
}
