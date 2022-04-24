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



WaveformPlayer::RiffHeaderChunk WaveformPlayer::findByID(char ID[]) {
    switch(ID[0]) {
    case 'd':
        if (ID[1] == 'a' && ID[2] == 't' && ID[3] == 'a') {
            return RHC_DATA;
        } else {
            return RHC_UNSUPPORTED;
        }
    case 'f':
        switch(ID[1]) {
        case 'a':
            if (ID[2] == 'c' && ID[3] == 't') {
                return RHC_FACT;
            } else {
                return RHC_UNSUPPORTED;
            }
        case 'm':
            if (ID[2] == 't') {
                return RHC_FMT;
            } else {
                return RHC_UNSUPPORTED;
            }
        default:
            return RHC_UNSUPPORTED;
        }
    default:
        return RHC_UNSUPPORTED;
    }
}

void WaveformPlayer::parseRIFFHeader(FILE *fp) {
    fseek(fp, 12, SEEK_SET);

    while (1) {
        char temp[4] = {0};
        unsigned int size;
        fread(temp, 1, 4, fp);
        switch(findByID(temp)) {
        case RHC_FMT:
            size = 16;
            fread(&header.fmt, 20, 1, fp);

            if (header.fmt.cksize > size) {
                fread(&header.fmtExt.cbSize, 2, 1, fp);

                fread((char *)&header.fmtExt + 2, header.fmtExt.cbSize, 1, fp);
                size += 2 + header.fmtExt.cbSize;
            }

            if (header.fmt.cksize > size) {
                fseek(fp, header.fmt.cksize - size, SEEK_CUR);
            }

            break;
        case RHC_DATA:
            fread(&header.audioSize, 4, 1, fp);
            return;
        case RHC_FACT:
        default:
            fread(&size, 4, 1, fp);

            fseek(fp, size, SEEK_CUR);
        }
    }
}

void WaveformPlayer::parseRIFFContent(FILE *fp) {
    int sampleByteCnt = header.fmt.nBitsPerSample / 8;
    int sampleSize = sampleByteCnt * header.fmt.nChannels;


    if (header.fmt.wFormatTag == WF_PCM) {
        for (int i = 0; i < header.audioSize / sampleSize; i++) {
            unsigned char sample[sampleByteCnt * header.fmt.nChannels];
            fread(sample, sampleByteCnt, header.fmt.nChannels, fp);

            std::vector<double> vsample;
            vsample.resize(header.fmt.nChannels);
            for (int j = 0; j < header.fmt.nChannels; j++) {
                unsigned int currentSample = 0;
                for (unsigned int offset = 0; offset < sampleByteCnt; offset++) {
                    currentSample += sample[j * sampleByteCnt + offset] << (offset * 8);
                }

                if (sample[j * sampleByteCnt + sampleByteCnt - 1] & 0x80) {
                    for (unsigned int offset = sampleByteCnt; offset < 4; offset++) {
                        currentSample += 0xff << (offset * 8);
                    }
                }

                vsample[j] = intClip(*(int *)(&currentSample), header.fmt.nBitsPerSample);
            }

            samples.emplace_back(std::move(vsample));
        }
    }
}

void WaveformPlayer::parseRIFF(const std::string &fn, TrackType type) {

    FILE *fp = fopen(fn.c_str(), "rb");


    // 1. Parse wave header
    parseRIFFHeader(fp);

    // 2. Check & set channel type
    if (trackCnt[type] < header.fmt.nChannels) {
        // Means that this audio is multi-channel audio
        // 1. Figure out the channel numbers and channel counts
        int trackNum = header.fmt.nChannels / trackCnt[type];
        int monoNum = header.fmt.nChannels % trackCnt[type];

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
