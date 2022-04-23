#include "waveformplayer.h"
#include "sampletimecode.h"
#include <cstdio>
#include <cstring>

double intClip(int val, int depth) {
    if (depth == 32) return val;

    int offset = 32 - depth;
    if(val > ((2 << depth) - 1)) return INT_MAX;
    else if (val < (-(2 << depth) + 1)) return -INT_MAX;
    return val << offset;
}

void WaveformPlayer::parseRIFFHeader(FILE *fp) {
    fseek(fp, 12, SEEK_SET);

    while (1) {
        char temp[4] = {0};
        fread(temp, 1, 4, fp);
        if (temp[0] == 'f' && temp[1] == 'm' && temp[2] == 't') {
            fread(&header.fmt, sizeof(RiffFmt), 1, fp);
            fseek(fp, header.fmt.size - sizeof(RiffFmt) + 4, SEEK_CUR);
        } else if (temp[0] == 'd' && temp[1] == 'a' && temp[2] == 't' && temp[3] == 'a') {
            fread(&header.audioSize, 4, 1, fp);
            return;
        }else {
            int size;
            fread(&size, 4, 1, fp);

            fseek(fp, size, SEEK_CUR);
        }
    }
}

void WaveformPlayer::parseRIFFContent(FILE *fp) {
    int sampleSize = header.fmt.bitDepth / 8 * header.fmt.numChannels;

    for (int i = 0; i < header.audioSize / sampleSize; i++) {
        unsigned char sample[header.fmt.bitDepth / 8 * header.fmt.numChannels];

        fread(sample, header.fmt.bitDepth / 8, header.fmt.numChannels, fp);

        std::vector<double> vsample;
        vsample.resize(header.fmt.numChannels);

        for (int j = 0; j < header.fmt.numChannels; j++) {
            vsample[j] = intClip(sample[j * header.fmt.bitDepth / 8]
                    + ((unsigned int)sample[j * header.fmt.bitDepth / 8 + 1] << 8)
                    + ((unsigned int)sample[j * header.fmt.bitDepth / 8 + 2] << 16)
                    + ((sample[j * header.fmt.bitDepth / 8 + 2] & 0x80) ? 0xff000000 : 0), header.fmt.bitDepth);
        }

        samples.emplace_back(std::move(vsample));
    }
}

void WaveformPlayer::parseRIFF(const std::string &fn, TrackType type) {

    FILE *fp = fopen(fn.c_str(), "rb");


    // 1. Parse wave header
    parseRIFFHeader(fp);

    // 2. Check & set channel type
    if (trackCnt[type] < header.fmt.numChannels) {
        // Means that this audio is multi-channel audio
        // 1. Figure out the channel numbers and channel counts
        int trackNum = header.fmt.numChannels / trackCnt[type];
        int monoNum = header.fmt.numChannels % trackCnt[type];

        // 2. Set up output channel information
        std::vector<TrackType> tracks;
        tracks.resize(trackNum, type);
        tracks.reserve(trackNum + monoNum);
        for (int i = 0; i < monoNum; i++) tracks.push_back(TRK_MONO);

        setTracks(tracks);
    }

    //3. Parse Content
    parseRIFFContent(fp);

    fclose(fp);
}

void WaveformPlayer::parseRIFF(const std::string &fn, const std::vector<TrackType> &type) {

    FILE *fp = fopen(fn.c_str(), "rb");


    // 1. Parse wave header
    parseRIFFHeader(fp);

    // 2. Believe in the given type now

    // 3. Parse Content
    parseRIFFContent(fp);

    fclose(fp);
}

void WaveformPlayer::fillBuffer(int track, double *buffer[], int bufferSize) {
    int samplePos = SampleTimeCode::get();

    int start = 0;

    for (int prev = 0; prev < track; prev++) {
        start += trackCnt[otracks[prev]];
    }

    for (int channel = start; channel < start + trackCnt[otracks[track]]; channel++) {
        if (samples[0].size() > channel) {
            for (int offset = 0; offset < bufferSize; offset++) {
                    buffer[channel - start][offset] = samples[samplePos + offset][channel];
            }
        } else {
            memset(buffer[channel - start], 0, bufferSize);
        }
    }
}
