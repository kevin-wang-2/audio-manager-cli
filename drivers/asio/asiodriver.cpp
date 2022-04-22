#include "asiodriver.h"
#include "asiodrivers.h"

// some external references
extern AsioDrivers* asioDrivers;
bool loadAsioDriver(const char *name);

ASIOError ASIOCreateBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
    long bufferSize, ASIOCallbacks *callbacks);

static AsioDriver *activeDriver;

long AsioDriver::initAsioStaticData (AsioDriver::DriverInfo *asioDriverInfo)
{	// collect the informational data of the driver
    // get the number of available channels
    if(ASIOGetChannels(&asioDriverInfo->inputChannels, &asioDriverInfo->outputChannels) == ASE_OK)
    {
        printf ("ASIOGetChannels (inputs: %d, outputs: %d);\n", asioDriverInfo->inputChannels, asioDriverInfo->outputChannels);

        // get the usable buffer sizes
        if(ASIOGetBufferSize(&asioDriverInfo->minSize, &asioDriverInfo->maxSize, &asioDriverInfo->preferredSize, &asioDriverInfo->granularity) == ASE_OK)
        {
            printf ("ASIOGetBufferSize (min: %d, max: %d, preferred: %d, granularity: %d);\n",
                     asioDriverInfo->minSize, asioDriverInfo->maxSize,
                     asioDriverInfo->preferredSize, asioDriverInfo->granularity);

            // get the currently selected sample rate
            if(ASIOGetSampleRate(&asioDriverInfo->sampleRate) == ASE_OK)
            {
                if(ASIOSetSampleRate(48000.0) == ASE_OK)
                {
                    printf ("ASIOGetSampleRate (sampleRate: %f);\n", asioDriverInfo->sampleRate);
                    if (asioDriverInfo->sampleRate <= 0.0 || asioDriverInfo->sampleRate > 96000.0) {
                        // Driver does not store it's internal sample rate, so set it to a know one.
                        // Usually you should check beforehand, that the selected sample rate is valid
                        // with ASIOCanSampleRate().
                        if (ASIOSetSampleRate(48000.0) == ASE_OK) {
                            if (ASIOGetSampleRate(&asioDriverInfo->sampleRate) == ASE_OK)
                                printf("ASIOGetSampleRate (sampleRate: %f);\n", asioDriverInfo->sampleRate);
                            else
                                return -6;
                        } else
                            return -5;
                    }
                } else return -5;

                // check wether the driver requires the ASIOOutputReady() optimization
                // (can be used by the driver to reduce output latency by one block)
                if(ASIOOutputReady() == ASE_OK)
                    asioDriverInfo->postOutput = true;
                else
                    asioDriverInfo->postOutput = false;
                printf ("ASIOOutputReady(); - %s\n", asioDriverInfo->postOutput ? "Supported" : "Not supported");

                return 0;
            }
            return -3;
        }
        return -2;
    }
    return -1;
}

ASIOError AsioDriver::createAsioBuffers (DriverInfo *asioDriverInfo)
{	// create buffers for all inputs and outputs of the card with the
    // preferredSize from ASIOGetBufferSize() as buffer size
    long i;
    ASIOError result;

    // fill the bufferInfos from the start without a gap
    std::vector<ASIOBufferInfo> &info = asioDriverInfo->bufferInfos;

    info.resize(asioDriverInfo->inputChannels + asioDriverInfo->outputChannels);

    // prepare inputs (Though this is not necessaily required, no opened inputs will work, too
    asioDriverInfo->inputBuffers = asioDriverInfo->inputChannels;
    for(i = 0; i < asioDriverInfo->inputBuffers; i++)
    {
        info[i].isInput = ASIOTrue;
        info[i].channelNum = i;
        info[i].buffers[0] = info[i].buffers[1] = 0;
    }

    // prepare outputs
    asioDriverInfo->outputBuffers = asioDriverInfo->outputChannels;
    for(i = 0; i < asioDriverInfo->outputBuffers; i++)
    {
        info[asioDriverInfo->inputChannels + i].isInput = ASIOFalse;
        info[asioDriverInfo->inputChannels + i].channelNum = i;
        info[asioDriverInfo->inputChannels + i].buffers[0] = info[i].buffers[1] = 0;
    }

    // create and activate buffers

    result = ASIOCreateBuffers(asioDriverInfo->bufferInfos.data(),
        asioDriverInfo->inputBuffers + asioDriverInfo->outputBuffers,
        asioDriverInfo->preferredSize, &asioCallbacks);

    channels.reserve(asioDriverInfo->inputChannels + asioDriverInfo->outputChannels);
    resizeTrack(asioDriverInfo->outputChannels);

    if (result == ASE_OK)
    {
        asioDriverInfo->channelInfos.resize(asioDriverInfo->inputBuffers + asioDriverInfo->outputBuffers);

        // now get all the buffer details, sample word length, name, word clock group and activation
        for (i = 0; i < asioDriverInfo->inputBuffers + asioDriverInfo->outputBuffers; i++)
        {
            asioDriverInfo->channelInfos[i].channel = asioDriverInfo->bufferInfos[i].channelNum;
            asioDriverInfo->channelInfos[i].isInput = asioDriverInfo->bufferInfos[i].isInput;
            result = ASIOGetChannelInfo(&asioDriverInfo->channelInfos[i]);
            channels.push_back(AudioChannel(
                                   asioDriverInfo->channelInfos[i].channel,
                                   asioDriverInfo->channelInfos[i].name,
                                   asioDriverInfo->channelInfos[i].isInput ? AC_INPUT : AC_OUTPUT,
                                   *this,
                                   asioDriverInfo->bufferInfos[i].buffers[0],
                                   asioDriverInfo->bufferInfos[i].buffers[1]
                               ));
            if (!asioDriverInfo->channelInfos[i].isInput) {
                itracks[i - asioDriverInfo->inputBuffers] = TRK_MONO;
            }
            if (result != ASE_OK)
                break;
        }

        if (result == ASE_OK)
        {
            // get the input and output latencies
            // Latencies often are only valid after ASIOCreateBuffers()
            // (input latency is the age of the first sample in the currently returned audio block)
            // (output latency is the time the first sample in the currently returned audio block requires to get to the output)
            result = ASIOGetLatencies(&asioDriverInfo->inputLatency, &asioDriverInfo->outputLatency);
            if (result == ASE_OK)
                printf ("ASIOGetLatencies (input: %d, output: %d);\n", asioDriverInfo->inputLatency, asioDriverInfo->outputLatency);
        }
    }
    return result;
}

unsigned long getSysReferenceTime()
{	// get the system reference time
#if WINDOWS
    return timeGetTime();
#elif MAC
static const double twoRaisedTo32 = 4294967296.;
    UnsignedWide ys;
    Microseconds(&ys);
    double r = ((double)ys.hi * twoRaisedTo32 + (double)ys.lo);
    return (unsigned long)(r / 1000.);
#endif
}

#if NATIVE_INT64
    #define ASIO64toDouble(a)  (a)
#else
    const double twoRaisedTo32 = 4294967296.;
    #define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif

ASIOTime *AsioDriver::bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow)
{	// the actual processing callback.
    // Beware that this is normally in a seperate thread, hence be sure that you take care
    // about thread synchronization. This is omitted here for simplicity.
    static long processedSamples = 0;

    auto &asioDriverInfo = activeDriver->asioDriverInfo;

    // store the timeInfo for later use
    asioDriverInfo.tInfo = *timeInfo;

    // get the time stamp of the buffer, not necessary if no
    // synchronization to other media is required
    if (timeInfo->timeInfo.flags & kSystemTimeValid)
        asioDriverInfo.nanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
    else
        asioDriverInfo.nanoSeconds = 0;

    if (timeInfo->timeInfo.flags & kSamplePositionValid)
        asioDriverInfo.samples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
    else
        asioDriverInfo.samples = 0;

    if (timeInfo->timeCode.flags & kTcValid)
        asioDriverInfo.tcSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
    else
        asioDriverInfo.tcSamples = 0;

    // get the system reference time
    asioDriverInfo.sysRefTime = getSysReferenceTime();

    int bufferSize = asioDriverInfo.preferredSize;
    double buffer[bufferSize];
    double *pbuffer[1];
    pbuffer[0] = buffer;

    for (int track = 0; track < activeDriver->itrackNum; track++) {
        if (!activeDriver->noconnection(track)) {
            activeDriver->recieveBuffer(track, pbuffer, bufferSize);
            activeDriver->channels[track + asioDriverInfo.inputChannels].fillBuffer(buffer, index);
        }
    }

    activeDriver->b_cb(index);

    /*

    if (processedSamples >= asioDriverInfo.sampleRate * TEST_RUN_TIME)	// roughly measured
        asioDriverInfo.stopped = true;
    else
        processedSamples += buffSize;
    */

    // finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
    if (asioDriverInfo.postOutput)
        ASIOOutputReady();
    return 0L;
}

//----------------------------------------------------------------------------------
void AsioDriver::bufferSwitch(long index, ASIOBool processNow)
{	// the actual processing callback.
    // Beware that this is normally in a seperate thread, hence be sure that you take care
    // about thread synchronization. This is omitted here for simplicity.

    // as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
    // though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
    ASIOTime  timeInfo;
    memset (&timeInfo, 0, sizeof (timeInfo));

    // get the time stamp of the buffer, not necessary if no
    // synchronization to other media is required
    if(ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
        timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

    bufferSwitchTimeInfo (&timeInfo, index, processNow);
}


//----------------------------------------------------------------------------------
void AsioDriver::sampleRateChanged(ASIOSampleRate sRate)
{
    // do whatever you need to do if the sample rate changed
    // usually this only happens during external sync.
    // Audio processing is not stopped by the driver, actual sample rate
    // might not have even changed, maybe only the sample rate status of an
    // AES/EBU or S/PDIF digital input at the audio device.
    // You might have to update time/sample related conversion routines, etc.

    activeDriver->g_cb(0, &sRate);
}

//----------------------------------------------------------------------------------
long AsioDriver::asioMessages(long selector, long value, void* message, double* opt)
{
    // currently the parameters "value", "message" and "opt" are not used.
    long ret = 0;
    switch(selector)
    {
        case kAsioSelectorSupported:
            if(value == kAsioResetRequest
            || value == kAsioEngineVersion
            || value == kAsioResyncRequest
            || value == kAsioLatenciesChanged
            // the following three were added for ASIO 2.0, you don't necessarily have to support them
            || value == kAsioSupportsTimeInfo
            || value == kAsioSupportsTimeCode
            || value == kAsioSupportsInputMonitor)
                ret = 1L;
            break;
        case kAsioResetRequest:
            // defer the task and perform the reset of the driver during the next "safe" situation
            // You cannot reset the driver right now, as this code is called from the driver.
            // Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
            // Afterwards you initialize the driver again.
            ret = 1L;
            break;
        case kAsioResyncRequest:
            // This informs the application, that the driver encountered some non fatal data loss.
            // It is used for synchronization purposes of different media.
            // Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
            // Windows Multimedia system, which could loose data because the Mutex was hold too long
            // by another thread.
            // However a driver can issue it in other situations, too.
            ret = 1L;
            break;
        case kAsioLatenciesChanged:
            // This will inform the host application that the drivers were latencies changed.
            // Beware, it this does not mean that the buffer sizes have changed!
            // You might need to update internal delay data.
            ret = 1L;
            break;
        case kAsioEngineVersion:
            // return the supported ASIO version of the host application
            // If a host applications does not implement this selector, ASIO 1.0 is assumed
            // by the driver
            ret = 2L;
            break;
        case kAsioSupportsTimeInfo:
            // informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
            // is supported.
            // For compatibility with ASIO 1.0 drivers the host application should always support
            // the "old" bufferSwitch method, too.
            ret = 1;
            break;
        case kAsioSupportsTimeCode:
            // informs the driver wether application is interested in time code info.
            // If an application does not need to know about time code, the driver has less work
            // to do.
            ret = 0;
            break;
    }
    return ret;
}

AsioDriver::AsioDriver() : AudioDriver(48000, 0, {})
{
    activeDriver = this;
}

std::vector<std::string> AsioDriver::getDriverNames() const {
    AsioDrivers drvGrp;

    // 1. Get Driver Names
    int numDrv = drvGrp.asioGetNumDev();
    char drivers[numDrv][1024];
    char *p_drivers[numDrv];
    for (int i = 0; i < numDrv; i++) p_drivers[i] = drivers[i];

    drvGrp.getDriverNames(p_drivers, numDrv);

    // 2. Wrap
    std::vector<std::string> ret;
    for (int i = 0; i < numDrv; i++) ret.push_back(p_drivers[i]);

    return ret;
}

void AsioDriver::loadDriver(int id) {
    AsioDrivers drvGrp;

    // 1. Get Driver Names
    int numDrv = drvGrp.asioGetNumDev();
    char drivers[numDrv][1024];
    char *p_drivers[numDrv];
    for (int i = 0; i < numDrv; i++) p_drivers[i] = drivers[i];

    drvGrp.getDriverNames(p_drivers, numDrv);

    // 2. Load Driver
    if(loadAsioDriver(drivers[id])) {
        if (ASIOInit (&asioDriverInfo.driverInfo) == ASE_OK)
        {
            if (initAsioStaticData (&asioDriverInfo) == 0)
            {
                asioCallbacks.bufferSwitch = &bufferSwitch;
                asioCallbacks.sampleRateDidChange = &sampleRateChanged;
                asioCallbacks.asioMessage = &asioMessages;
                asioCallbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo;
                if (createAsioBuffers (&asioDriverInfo) == ASE_OK)
                {

                }
            }
        }
    }

err:
    // TODO: Handle Errors
    return;
}

void AsioDriver::setSampleRate(double sampleRate) {
    if(ASIOGetSampleRate(&asioDriverInfo.sampleRate) == ASE_OK)
    {
        sampleRate = asioDriverInfo.sampleRate;
        if(ASIOSetSampleRate(sampleRate) == ASE_OK)
        {
            if (asioDriverInfo.sampleRate <= 0.0 || asioDriverInfo.sampleRate > 96000.0) {
                if (ASIOSetSampleRate(sampleRate) == ASE_OK) {
                    ASIOGetSampleRate(&asioDriverInfo.sampleRate);
                    sampleRate = asioDriverInfo.sampleRate;
                }
            }
        }
    }
err:
    // TODO: Handle Errors
    return;
}

void AsioDriver::setBufferSize(int bufferSize) {
    long i;
    ASIOError result;

    // fill the bufferInfos from the start without a gap
    std::vector<ASIOBufferInfo> info = asioDriverInfo.bufferInfos;

    // prepare inputs (Though this is not necessaily required, no opened inputs will work, too
    asioDriverInfo.inputBuffers = asioDriverInfo.inputChannels;
    for(i = 0; i < asioDriverInfo.inputBuffers; i++)
    {
        info[i].isInput = ASIOTrue;
        info[i].channelNum = i;
        info[i].buffers[0] = info[i].buffers[1] = 0;
    }

    // prepare outputs
    asioDriverInfo.outputBuffers = asioDriverInfo.outputChannels;
    for(i = 0; i < asioDriverInfo.outputBuffers; i++)
    {
        info[i].isInput = ASIOFalse;
        info[i].channelNum = i;
        info[i].buffers[0] = info[i].buffers[1] = 0;
    }

    // create and activate buffers

    result = ASIOCreateBuffers(asioDriverInfo.bufferInfos.data(),
        asioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers,
        bufferSize, &asioCallbacks);


    if (result == ASE_OK)
    {
        // now get all the buffer details, sample word length, name, word clock group and activation
        for (i = 0; i < asioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers; i++)
        {
            asioDriverInfo.channelInfos[i].channel = asioDriverInfo.bufferInfos[i].channelNum;
            asioDriverInfo.channelInfos[i].isInput = asioDriverInfo.bufferInfos[i].isInput;
            result = ASIOGetChannelInfo(&asioDriverInfo.channelInfos[i]);
            AudioChannel rep(
                                    i,
                                    asioDriverInfo.channelInfos[i].name,
                                    asioDriverInfo.channelInfos[i].isInput ? AC_INPUT : AC_OUTPUT,
                                    *this,
                                    asioDriverInfo.bufferInfos[i].buffers[0],
                                    asioDriverInfo.bufferInfos[i].buffers[1]
                                );
            channels.at(i) = rep;
            if (result != ASE_OK)
                break;
        }

        if (result == ASE_OK)
        {
            // get the input and output latencies
            // Latencies often are only valid after ASIOCreateBuffers()
            // (input latency is the age of the first sample in the currently returned audio block)
            // (output latency is the time the first sample in the currently returned audio block requires to get to the output)
            ASIOGetLatencies(&asioDriverInfo.inputLatency, &asioDriverInfo.outputLatency);
        }
    }
}

void AsioDriver::startAudio() {
    ASIOStart();
}

void AsioDriver::stopAudio() {
    ASIOStop();
    ASIODisposeBuffers();
    ASIOExit();

    AsioDrivers drvGrp;
    drvGrp.removeCurrentDriver();
}
