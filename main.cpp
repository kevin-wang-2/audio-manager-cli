#include "asiodriver.h"
#include "Panner.h"
#include "fader.h"
#include "Imager.h"
#include "sine.h"
#include "waveformplayer.h"
#include "sampletimecode.h"
#include <iostream>

using namespace std;

void printParameterList(const AudioDevice& dev) {
    int cnt = dev.getParamCnt();
    cout << "---------" << endl;
    for(int i = 0; i < cnt; i++) {
        auto name = dev.getParamName(i);
        auto type = dev.getParamType(i);
        auto range = dev.getParameterRange(i);
        auto valueType = dev.getParameterValueType(i);
        auto value = dev.getParameterValue(i);

        cout << i << " " << name << endl;
        switch (type) {
            case DP_Variable:
                switch (valueType) {
                    case PV_Double_Bounded:
                        cout << "Value: " << value.val << endl;
                        cout << "Range: " << range.range[0] << " - " << range.range[1] << endl;
                        break;
                    case PV_Single_Bounded:
                        cout << "Value: " << value.val << endl;
                        cout << "Range: -Inf" << " - " << range.max << endl;;
                        break;
                    case PV_Enum:
                        cout << "Value: " << value.enu << endl;
                        cout << "Range: " << range.range[0] << " - " << range.range[1] << endl;;
                        break;
                }
                break;
            case DP_Switch:
                cout << (value.enu ? "On" : "Off") << endl;
                break;
            case DP_Button:
                cout << "Button" << endl;
                break;
        }
        cout << "---------" << endl;
    }
}

void setStringParam(AudioDevice &dev, int param, const string &val) {
    auto type = dev.getParamType(param);
    if (type == DP_Button) {
        dev.press(param);
    } else if (type == DP_Switch) {
        if (val == "Off" || val == "0") dev.setValue(param, {.enu = 0});
        else dev.setValue(param, {.enu =  1});
    } else {
        auto vType = dev.getParameterValueType(param);
        if (vType == PV_Enum) {
            try {
                int nval = stoi(val);
                dev.setValue(param, {.enu = nval});
            } catch(invalid_argument &e) {
            }
        } else {
            try {
                double nval = stod(val);
                dev.setValue(param, {nval});
            } catch(invalid_argument &e) {
            }
        }
    }
}

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
    StereoPanner panner(48000);
    Fader fader(48000, TRK_STEREO);
    Imager imager(48000);

    imager.connectGenerator(gen, 0);
    panner.connectGenerator(imager, 0);
    fader.connectGenerator(panner, 0);

    drv.connectGenerator(fader, l, {{0, 0}});
    drv.connectGenerator(fader, r, {{0, 1}});

    drv.setSynchronizationCallback(SampleTimeCode::set);

    cout << "Configuration Done." << endl;

    drv.startAudio();

    while (1) {
        std::string line;
        getline(cin, line);

        if (line.find("exit") == 0) break;

        size_t actionPos = line.find(' ');
        string action = line.substr(0, actionPos);
        string sub = line.substr(actionPos + 1, string::npos);

        if (action == "list") {
            actionPos = sub.find(' ');
            // TODO: Replace later for formal mixer setup
            if (actionPos == string::npos) {
                if (sub == "chain") {
                    cout << "0 imager, 1 panner, 2 fader" << endl;
                } else {
                    string::size_type sz = 0;
                    try {
                        int target = stoi(sub, &sz);
                        if (sz != 0) {
                            switch (target) {
                                case 0:
                                    printParameterList(imager);
                                    break;
                                case 1:
                                    printParameterList(panner);
                                    break;
                                case 2:
                                    printParameterList(fader);
                                    break;
                            }
                        }
                    } catch (invalid_argument &e) {
                    }
                }
            }
        } else if (action == "set") {
            size_t delimPos = sub.find('.');
            actionPos = sub.find(' ');

            if (delimPos != string::npos && actionPos != string::npos && delimPos < actionPos) {
                string efxId = sub.substr(0, delimPos);
                string paramId = sub.substr(delimPos + 1, actionPos - delimPos);
                string val = sub.substr(actionPos + 1, string::npos);

                try {
                    int efx = stoi(efxId);
                    int param = stoi(paramId);
                    switch (efx) {
                        case 0:
                            setStringParam(imager, param, val);
                            break;
                        case 1:
                            setStringParam(panner, param, val);
                            break;
                        case 2:
                            setStringParam(fader, param, val);
                            break;
                    }
                } catch (invalid_argument &e) {

                }
            }

        }

    }

    drv.stopAudio();

    return 0;
}
