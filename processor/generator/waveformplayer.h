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

    const unsigned int WF_PCM = 1;
    const unsigned int WF_IEEE_FLOAT = 3;
    const unsigned int WF_ALAW = 6;
    const unsigned int WF_MULAW = 7;
    const unsigned int WF_EXTENSIBLE = 0xFFFE;

    typedef struct RiffFmt {
       unsigned cksize;

       unsigned short wFormatTag;
       unsigned short nChannels;

       unsigned nSamplesPerSec;
       unsigned nAvgBytesPerSec;

       unsigned short nBlockAlign;
       unsigned short nBitsPerSample;
    } RiffFmt;

    typedef struct RiffFmtExt {
        unsigned short cbSize;

        unsigned short wValidBitsPerSample;
        unsigned int dwChannelMask;

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
