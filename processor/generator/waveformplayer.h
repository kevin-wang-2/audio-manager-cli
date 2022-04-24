#ifndef WAVEFORMPLAYER_H
#define WAVEFORMPLAYER_H

#include "audiogenerator.h"
#include <string>
#include <vector>

class WaveformPlayer : public AudioGenerator
{
    typedef enum {
        RHC_FMT,
        RHC_DATA,
        RHC_FACT,
        RHC_UNSUPPORTED
    } RiffHeaderChunk;

    typedef struct RiffFmt {
       unsigned cksize:4;

       unsigned wFormatTag:2;
       unsigned nChannels:2;

       unsigned nSamplesPerSec:4;
       unsigned nAvgBytesPerSec:4;

       unsigned nBlockAlign:2;
       unsigned nBitsPerSample:2;

       unsigned cbSize;
    } RiffFmt;

    typedef struct RiffFmtExt {
        unsigned wValidBitsPerSample:2;
        unsigned dwChannelMask:2;

        unsigned char SubFormat[8];
    } RiffFmtExt;

    typedef struct RiffFact {
        unsigned ckSize:4;
        unsigned dwSampleLength:4;
    } RiffFact;

    typedef struct RiffHeader {
        RiffFmt fmt;
        RiffFmtExt fmtExt;
        RiffFact fact;
        int audioSize;
    } RiffHeader;

    RiffHeader header;

    std::vector<std::vector<double>> samples;

    static RiffHeaderChunk findByID(char ID[]);

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
