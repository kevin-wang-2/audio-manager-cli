#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

/**
 * Created by WKB in 2022/4/21
 * Abstraction of a general Audio Device (Virtual) including plugins, mixers, etc.
 */

#include <string>
#include <vector>

typedef enum {
    DP_Variable,
    DP_Switch,
    DP_Button
} DeviceParameterType;

typedef enum {
    PV_Double_Bounded,
    PV_Single_Bounded,
    PV_Enum
} ParameterValueType;

typedef union {
    double range[2];
    double max;
} ParameterValueRange;

typedef union {
    double val;
    int enu;
} ParameterValue;

class AudioDevice
{
protected:
    struct AudioDeviceParameter {
        int id;
        std::string name;
        DeviceParameterType type;
        ParameterValueType vtype;
        ParameterValue v;
        ParameterValueRange range;
        std::vector<std::string> list;
    };

    using AudioDeviceParameter = struct AudioDeviceParameter;

    std::vector<AudioDeviceParameter> parameters;
    std::string name;
public:
    AudioDevice();

// Getters
    std::string getName() const { return name; };
    std::string getParamName(int id) const {return parameters[id].name; }
    int getParamCnt() const { return parameters.size(); }
    DeviceParameterType getParamType(int id) const { return parameters[id].type; }
    ParameterValueType getParameterValueType(int id) const { return parameters[id].vtype; }
    const ParameterValueRange &getParameterRange(int id) const { return parameters[id].range; };
    const std::vector<std::string> &getParameterRangeList(int id) const { return parameters[id].list; };
    ParameterValue getParameterValue(int id) const { return parameters[id].v; }

// Adjustment
    virtual void setValue(int id, ParameterValue value) = 0;
    virtual void press(int id) = 0;
    void set(int id, ParameterValue value = {100.0}) {
        if(getParamType(id) == DP_Variable || getParamType(id) == DP_Switch) {
            setValue(id, value);
        } else {
            press(id);
        }
    }
};

#endif // AUDIODEVICE_H
