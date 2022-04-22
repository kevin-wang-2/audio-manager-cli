#include "asiodriver.h"
#include "fader.h"
#include "sine.h"
#include "sampletimecode.h"
#include <iostream>
#include <cmath>
#include <ctime>

using namespace std;

int main() {
    AsioDriver drv;

    auto names = drv.getDriverNames();

    int i = 0;

    for (auto &name : names) {
        cout << i++ << ":" << name << endl;
    }

    cout << "Select an ASIO Driver:";
    cin >> i;

    drv.loadDriver(i);

    drv.setSampleRate(48000.0);
    drv.setSampleRate(512);

    int outputChannelCnt = drv.getOutputChannelCnt();
    int inputChannelCnt = drv.getInputChannelCnt();
    for (int i = inputChannelCnt; i < inputChannelCnt + outputChannelCnt; i++) {
        cout << i - inputChannelCnt << ":" << drv.getChannel(i).getName() << endl;
    }

    int l, r;

    cout << "Select Left Channel:";
    cin >> l;

    cout << "Select Right Channel:";
    cin >> r;

    SampleTimeCode::init();

    Sine sineGen(48000, TRK_STEREO, INT_MAX, 0, 880);
    Fader fader(48000, TRK_STEREO);

    fader.connectGenerator(sineGen, 0);

    drv.connectGenerator(fader, l, {{0, 0}});
    drv.connectGenerator(fader, r, {{0, 1}});

    drv.setSynchronizationCallback(SampleTimeCode::set);

    cout << "Configuration Done." << endl;

    drv.startAudio();

    double db = 0.0;
    while(db <= 6.0) {
        cin >> db;

        fader.setValue(1, {db});
    }

    drv.stopAudio();
}
