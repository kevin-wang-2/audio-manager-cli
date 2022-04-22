#ifndef ASIODRIVER_H
#define ASIODRIVER_H

/**
 * Created by WKB in 2022/4/21
 * Abstraction Layer of ASIO Driver
 * Trying to hide the ugly implementations by Steinberg
 */

// Abstraction
#include "audiodriver.h"

// ASIO-Related Headers
#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h"

class AsioDriver : public AudioDriver
{
    // Copied from ASIO SDK Example
    typedef struct DriverInfo
    {
        // ASIOInit()
        ASIODriverInfo driverInfo;

        // ASIOGetChannels()
        long           inputChannels;
        long           outputChannels;

        // ASIOGetBufferSize()
        long           minSize;
        long           maxSize;
        long           preferredSize;
        long           granularity;

        // ASIOGetSampleRate()
        ASIOSampleRate sampleRate;

        // ASIOOutputReady()
        bool           postOutput;

        // ASIOGetLatencies ()
        long           inputLatency;
        long           outputLatency;

        // ASIOCreateBuffers ()
        long inputBuffers;	// becomes number of actual created input buffers
        long outputBuffers;	// becomes number of actual created output buffers
        std::vector<ASIOBufferInfo> bufferInfos; // buffer info's

        // ASIOGetChannelInfo()
        std::vector<ASIOChannelInfo> channelInfos; // channel info's
        // The above two arrays share the same indexing, as the data in them are linked together

        // Information from ASIOGetSamplePosition()
        // data is converted to double floats for easier use, however 64 bit integer can be used, too
        double         nanoSeconds;
        double         samples;
        double         tcSamples;	// time code samples

        // bufferSwitchTimeInfo()
        ASIOTime       tInfo;			// time info state
        unsigned long  sysRefTime;      // system reference time, when bufferSwitch() was called

        // Signal the end of processing in this example
        bool           stopped;
    } DriverInfo;

    // Dirty ASIO-Related Functions and variables
    DriverInfo asioDriverInfo;
    ASIOCallbacks asioCallbacks;
    static long initAsioStaticData (DriverInfo *asioDriverInfo);
    ASIOError createAsioBuffers (DriverInfo *asioDriverInfo);

    // Raw Callbacks
    static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
    static void bufferSwitch(long index, ASIOBool processNow);
    static void sampleRateChanged(ASIOSampleRate sRate);
    static long asioMessages(long selector, long value, void* message, double* opt);

    // User Callbacks
    GenericCallback g_cb;
    SynchronizationCallback s_cb;

    // Channel Abstraction Layer
    std::vector<AudioChannel> channels;
public:
    AsioDriver();

    // Driver Selection
    virtual std::vector<std::string> getDriverNames() const final;
    virtual void loadDriver(int id) final;

    // Audio Core Function
    virtual void startAudio();
    virtual void stopAudio();

    // Setters
    virtual void setSampleRate(double sampleRate);
    virtual void setBufferSize(int bufferSize);

    // Generic Getters
    virtual double getSampleRate() const final { return asioDriverInfo.sampleRate; }
    virtual int getBufferSize() const final { return asioDriverInfo.preferredSize; }
    virtual int getInputLatency() const final { return asioDriverInfo.inputLatency; }
    virtual int getOutputLatency() const final { return asioDriverInfo.outputLatency; }
    virtual BitDepthType getBitDepth() const final {
            // OK do processing for the outputs only
            switch (asioDriverInfo.channelInfos[0].type)
            {
            case ASIOSTInt16LSB:
                return BD_INT16LSB;
            case ASIOSTInt24LSB:
                return BD_INT24LSB;
            case ASIOSTInt32LSB:
                return BD_INT32LSB;
            case ASIOSTFloat32LSB:
                return BD_FLOAT32LSB;
            case ASIOSTFloat64LSB:
                return BD_FLOAT64LSB;
            case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
            case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
            case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
            case ASIOSTInt32LSB24:
                return BD_UNSUPPORTED;
            case ASIOSTInt16MSB:
                return BD_INT16MSB;
            case ASIOSTInt24MSB:
                return BD_INT24MSB;
            case ASIOSTInt32MSB:
                return BD_INT32MSB;
            case ASIOSTFloat32MSB:
                return BD_FLOAT32MSB;
            case ASIOSTFloat64MSB:
                return BD_FLOAT64MSB;
            case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
            case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
            case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
            case ASIOSTInt32MSB24:
                return BD_UNSUPPORTED;
            default:
                return BD_UNSUPPORTED;
            }
    }

    virtual std::string getName() const final { return asioDriverInfo.driverInfo.name; };

    // Channel
    virtual int getInputChannelCnt() const final { return asioDriverInfo.inputChannels; }
    virtual int getOutputChannelCnt() const final { return asioDriverInfo.outputChannels; }
    virtual AudioChannel &getChannel(int id) final { return channels[id]; }

    // Callbacks
    virtual void setGenericCallback(const GenericCallback &cb) final { g_cb = cb; };
    virtual void setSynchronizationCallback(const SynchronizationCallback &cb) final { s_cb = cb; };
};

#endif // ASIODRIVER_H
