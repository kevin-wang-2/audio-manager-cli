#ifndef AUDIORECIEVER_H
#define AUDIORECIEVER_H


/**
 * Created by WKB in 2022/4/21
 * Abstraction of a general Audio Reciever, i.e. any device that would have some audio inputs
 */

#include "audiometa.h"
#include "audiogenerator.h"
#include <vector>
#include <unordered_map>

class AudioReciever
{
    /**
     * @brief connections
     * i: input, o: output
     * T: track, B: Buffer, D: Device
     *
     * -> []: map, =>: vector index
     * {}: vector
     *
     * iTx => {oDx -> [oTx -> [oBx -> {iBx}]]}
     */
    std::vector<std::unordered_map<AudioGenerator *, std::unordered_map <int, std::unordered_map<int, std::vector<int>>>>> connections;

protected:
    double sampleRate;
    int itrackNum;
    std::vector<TrackType> itracks;

    void receiveBuffer(int track, double *buffer[], int bufferSize);
    virtual bool noconnection(int track) { return connections[track].empty(); }

    virtual void resizeTrack(int trackNum) {
        itrackNum = trackNum;
        itracks.resize(trackNum);
        connections.resize(trackNum, {});
    }

public:
    AudioReciever(double _sampleRate, int _trackNum, std::vector<TrackType> _tracks);

// Core Functions
    virtual void connectGenerator(AudioGenerator &gen, int track, std::vector<std::pair<int, int>> bufferSelection = {});
    // virtual void connectGenerator(int track, std::unordered_map<AudioGenerator &, std::tuple<int, int>> bufferSelection);
    virtual void disconnectGenerator(int track);

// Getters
    virtual int getTrackNum() const { return itrackNum; }
    virtual const std::vector<TrackType> &getTrackTypes() const { return itracks; }
    virtual TrackType getTrackType(int track) const { return itracks[track]; }
};

#endif // AUDIORECIEVER_H
