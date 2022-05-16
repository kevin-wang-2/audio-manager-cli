#include "audioreciever.h"
#include <cstring>
#include <stdio.h>

AudioReceiver::AudioReceiver(double _sampleRate, int _trackNum, std::vector<TrackType> _tracks) :
    sampleRate(_sampleRate), itrackNum(_trackNum), itracks(std::move(_tracks))
{
    connections.resize(itrackNum);
}


/**
 * Connections format:
 * i: input, o: output
 * T: track, B: Buffer, D: Device
 *
 * -> []: map, =>: vector index
 * {}: vector,
 *
 * iTx => {oDx -> [oTx -> [oBx -> {iBx}]]}
 */

void AudioReceiver::connectGenerator(AudioGenerator &gen, int track, std::vector<std::pair<int, int>> _bufferSelection) {
    if (_bufferSelection.empty()) {
        auto &connectionMap = connections[track][&gen];
        int bufferNumReq = trackCnt[itracks[track]];
        int otrackNum = gen.getTrackNum();
        int ibuffer = 0;

        // 1. Assign Suitable Tracks and Buffers
        for (int otrack = 0; otrack < otrackNum; otrack++) {
            int obufferNum = trackCnt[gen.getType(otrack)];
            for (int obuffer = 0; obuffer < obufferNum && ibuffer < bufferNumReq; obuffer++) {
                connectionMap[otrack][obuffer].push_back(ibuffer++);
            }
            if (ibuffer == bufferNumReq) break; // Early Exit
        }

        // 2. Not enough buffer, use the last
        // TODO: Use more generic processor (e.g. for spacial audio)
        if (ibuffer < bufferNumReq) {
            int lastOTrack = otrackNum - 1;
            int lastOBuffer = trackCnt[gen.getType(lastOTrack)] - 1;
            for(;ibuffer < bufferNumReq; ibuffer++) {
                connectionMap[lastOTrack][lastOBuffer].push_back(ibuffer);
            }
        }

    } else {
        int ibuffer = 0;
        for (auto [otrack, obuffer]: _bufferSelection) {
            connections[track][&gen][otrack][obuffer].push_back(ibuffer++);
        }
    }
}

void AudioReceiver::disconnectGenerator(int track) {
    connections[track] = {};
}

void AudioReceiver::receiveBuffer(int track, double *buffer[], int bufferSize) {
    double dummy[bufferSize];

    // 1. Walk through all possible devices
    for (auto& [pgen, connectionMap]: connections[track]) {
        // 2. Walk through all possible otracks
        for (auto& [otrack, trackMap]: connectionMap) {
            // 3. Arrange Buffer for This Call
            int bufferCnt = trackCnt[pgen->getType(otrack)];
            double *pbuf[bufferCnt]; // Buffer Pointer

            // 3.1 Point all buffer to dummy
            for (int i = 0; i < bufferCnt; i++) pbuf[i] = dummy;

            // 3.2 Point buffer to the first ibuffer
            for (auto& [obuffer, ibuffers]: trackMap) {
                if (ibuffers.size() > 0) {
                    pbuf[obuffer] = buffer[ibuffers[0]];
                }
            }

            // 4. Fill this buffer
            pgen->fillBuffer(otrack, pbuf, bufferSize);

            // 5. Copy buffer if necessary
            for (auto& [obuffer, ibuffers]: trackMap) {
                if(ibuffers.size() > 1) {
                    for (auto it = ibuffers.begin() + 1; it < ibuffers.end(); it++) {
                        memcpy(buffer[*it], pbuf[obuffer], bufferSize * sizeof(double));
                    }
                }
            }

        }
    }
}
