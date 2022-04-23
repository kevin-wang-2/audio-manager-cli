#include "waveformplayer.h"
#include "sampletimecode.h"
#include <cstdio>

double int24Clip32(int val) {
    if(val > 8388607) return INT_MAX;
    else if (val < -8388608) return -INT_MAX;
    return val * 256;
}

void WaveformPlayer::parseRIFF(const std::string &fn) {

    FILE *fp = fopen(fn.c_str(), "rb");

    fseek(fp, 12, SEEK_SET);

    while (1) {
        char temp[4] = {0};
        fread(temp, 1, 4, fp);
        if (temp[0] == 'f' && temp[1] == 'm' && temp[2] == 't') {
            fread(&header.fmt, sizeof(RiffFmt), 1, fp);
            fseek(fp, header.fmt.size - sizeof(RiffFmt) + 4, SEEK_CUR);
        } else if (temp[0] == 'd' && temp[1] == 'a' && temp[2] == 't' && temp[3] == 'a') {
            int size;
            fread(&size, 4, 1, fp);

            break;
        }else {
            int size;
            fread(&size, 4, 1, fp);

            fseek(fp, size, SEEK_CUR);
        }
    }

    while (!feof(fp)) {
        unsigned char sample[header.fmt.bitDepth / 8 * 2];

        fread(sample, header.fmt.bitDepth / 8, 2, fp);

        unsigned int L, R;
        L = sample[0] + ((unsigned int)sample[1] << 8) + ((unsigned int)sample[2] << 16) + ((sample[2] & 0x80) ? 0xff000000 : 0);
        R = sample[3] + ((unsigned int)sample[4] << 8) + ((unsigned int)sample[5] << 16) + ((sample[5] & 0x80) ? 0xff000000 : 0);

        samples.push_back({int24Clip32(*((int *)(&L))), int24Clip32(*((int *)(&R)))});
    }
}

void WaveformPlayer::fillBuffer(int track, double *buffer[], int bufferSize) {
    int samplePos = SampleTimeCode::get();

    for (int offset = 0; offset < bufferSize; offset++) {
        buffer[0][offset] = samples[samplePos + offset].first;
        buffer[1][offset] = samples[samplePos + offset].second;
    }
}
