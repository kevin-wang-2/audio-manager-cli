#include "asiodriver.h"
#include "fader.h"
#include "Imager.h"
#include "sine.h"
#include "waveformplayer.h"
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

select:
    cout << "Select an ASIO Driver:";
    cin >> i;

    if(drv.loadDriver(i)) {
        cout << "Cannot Start ASIO." << endl;
        goto select;
    }

    drv.setSampleRate(48000.0);

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

    string filename;
    cout << "File to play: ";
    cin >> filename;

    WaveformPlayer gen(48000, filename);
    Sine sine(48000, TRK_STEREO, INT_MAX, 0, 440);
    Imager imager(48000);
    Fader fader(48000, TRK_STEREO);

    fader.connectGenerator(gen, 0);
    imager.connectGenerator(fader, 0);

    drv.connectGenerator(imager, l, {{0, 0}});
    drv.connectGenerator(imager, r, {{0, 1}});

    drv.setSynchronizationCallback(SampleTimeCode::set);

    cout << "Configuration Done." << endl;

    drv.startAudio();

    double db = 0.0;
    while(db <= 200.0) {
        cin >> db;

        imager.setValue(0, {db});
    }

    drv.stopAudio();
}
