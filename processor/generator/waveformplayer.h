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
        int audioSize;
    } RiffHeader;

    RiffHeader header;

    std::vector<std::vector<double>> samples;

    void parseRIFFHeader(FILE *fp);

    void parseRIFFContent(FILE *fp);

    void parseRIFF(const std::string &fn, TrackType type);

    void parseRIFF(const std::string &fn, const std::vector<TrackType> &type);

public:
    WaveformPlayer(double _sampleRate, const std::string &fn, TrackType type = TRK_STEREO) :
        AudioGenerator(_sampleRate, 1, {type}) {
        parseRIFF(fn, type);
    }
    WaveformPlayer(double _sampleRate, const std::string &fn, const std::vector<TrackType> &type) :
        AudioGenerator(_sampleRate, type.size(), type) {
        parseRIFF(fn, type);
    }

    virtual void fillBuffer(int track, double *buffer[], int bufferSize) override;
};

#endif // WAVEFORMPLAYER_H
