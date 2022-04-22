#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

/**
 * Created by WKB in 2022/4/21
 * Basic anstraction of any specific audio driver. Notice, this only contains pure abstract function
 */

#include <string>
#include <vector>
#include <functional>
#include "audiochannel.h"
#include "audioreciever.h"
#include "AudioError.h"

typedef enum {
    BD_INT16LSB,
    BD_INT24LSB,
    BD_INT32LSB,
    BD_FLOAT32LSB,
    BD_FLOAT64LSB,
    BG_INT16LSB32,
    BG_INT24LSB32,
    BG_INT32LSB32,
    BD_INT16MSB,
    BD_INT24MSB,
    BD_INT32MSB,
    BD_FLOAT32MSB,
    BD_FLOAT64MSB,
    BG_INT16MSB32,
    BG_INT24MSB32,
    BG_INT32MSB32,
    BD_UNSUPPORTED
} BitDepthType;

class AudioDriver : public AudioReciever
{
public:

// Constructors
    AudioDriver(double _sampleRate, int _trackNum, std::vector<TrackType> _tracks)
        : AudioReciever(_sampleRate, _trackNum, std::move(_tracks)) {}

// Driver Selection
    virtual std::vector<std::string> getDriverNames() const = 0;
    virtual AudioDriverError loadDriver(int id) = 0;

// Audio Core Function
    virtual void startAudio() = 0;
    virtual void stopAudio() = 0;

// Setters
    virtual AudioDriverError setSampleRate(double sampleRate) = 0;
    virtual void setBufferSize(int bufferSize) = 0;

// Generic Getters
    virtual double getSampleRate() const = 0;
    virtual int getBufferSize() const = 0;
    virtual int getInputLatency() const = 0;
    virtual int getOutputLatency() const = 0;
    virtual BitDepthType getBitDepth() const = 0;
    virtual std::string getName() const = 0;

// Channel
    virtual int getInputChannelCnt() const = 0;
    virtual int getOutputChannelCnt() const = 0;
    virtual AudioChannel &getChannel(int id) = 0;

// Callbacks
    using GenericCallback = std::function<void(int, void *)>;
    using SynchronizationCallback = std::function<void(double)>;

    virtual void setGenericCallback(const GenericCallback &cb) = 0;
    virtual void setSynchronizationCallback(const SynchronizationCallback &cb) = 0;
};

#endif // AUDIODRIVER_H
