#ifndef AUDIOCHANNEL_H
#define AUDIOCHANNEL_H

#include <string>

/**
 * Created by WKB in 2022/4/21
 * Abstraction Layer of Any Specific Audio Channel
 */

class AudioDriver;

typedef enum {
    AC_INPUT,
    AC_OUTPUT
} AudioChannelType;

/**
 * @brief The AudioChannel Abstraction
 * Basic Wrapper of an Audio Channel
 */

class AudioChannel
{
    std::string name;
    AudioChannelType type;
    AudioDriver *pdrv;

    void *buffer[2];

    int id;

public:
    AudioChannel(int _id, std::string _name, AudioChannelType _type, AudioDriver &_pdrv, void *buf1, void *buf2) :
        id(_id), name(std::move(_name)), type(_type), pdrv(&_pdrv) {
        buffer[0] = buf1;
        buffer[1] = buf2;
    }

// Core Functions
    /**
     * @brief fillBuffer
     * @param buffer
     * Notice that internal calculation should be at least 32-bit INT precision, more should be better, currently hardcoded in the function
     * TODO: change the buffer type into generic typing
     */
    void fillBuffer(const double *buffer, int index);

// Generic Data Acquirement
    virtual const std::string &getName() const { return name; }
    virtual AudioChannelType getType() const { return type; }

// Assignment Operator
    virtual AudioChannel& operator=(const AudioChannel &another) {
        name = another.name;
        type = another.type;
        pdrv = another.pdrv;
        buffer[0] = another.buffer[0];
        buffer[1] = another.buffer[1];

        return *this;
    }

    friend class AudioDriver;
};

#endif // AUDIOCHANNEL_H
