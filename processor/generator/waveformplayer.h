#ifndef WAVEFORMPLAYER_H
#define WAVEFORMPLAYER_H

#include "audiogenerator.h"
#include <string>
#include <vector>

class WaveformPlayer : public AudioGenerator
{
    typedef struct RiffFmt {
       int size;

       short type;
       short numChannels;

       int sampleRate;
       int byteRate;

       short blockAlign;
       short bitDepth;
    } RiffFmt;

    typedef struct RiffHeader {
        RiffFmt fmt;
    } RiffHeader;

    RiffHeader header;

    std::vector<std::pair<double, double>> samples;

    void parseRIFF(const std::string &fn);

public:
    WaveformPlayer(double _sampleRate, const std::string &fn) :
        AudioGenerator(_sampleRate, 1, {TRK_STEREO}) {
        parseRIFF(fn);
    }

    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};

#endif // WAVEFORMPLAYER_H
