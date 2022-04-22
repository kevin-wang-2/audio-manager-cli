#include "audiochannel.h"
#include "audiodriver.h"


template <int destSize>
void copyInt32BufInt(void *dest, const double *buf, int size) {
    constexpr auto overflow = 32 - destSize;
    constexpr auto blockCnt = destSize / 8; // Block count of character
    char *destMem = (char *)dest;
    for (int i = 0; i < size; i++) {
        int tmp;
        tmp = buf[i];
        if (buf[i] > INT_MAX) {
            tmp = INT_MAX;
        }
        if (buf[i] < INT_MIN) {
            tmp = INT_MIN;
        }
        tmp >>= overflow;
        for (int offset = 0; offset < blockCnt; offset++) *(destMem++) = *((char *)(&tmp) + offset);
    }
}

/**
 * @brief copyInt32BufFloat
 * @param dest
 * @param buf
 * @param size
 * TODO: Implement Function
 */
template <int destSize>
void copyInt32BufFloat(void *dest, const double *buf, int size) {
    float *destMem = (float *)dest;
    for (int i = 0; i < size; i++) destMem[i] = 0;
}

void AudioChannel::fillBuffer(const double *buffer, int index) {
    BitDepthType bd = pdrv->getBitDepth();
    int bufferSize = pdrv->getBufferSize();

    switch (bd) {
        case BD_INT16LSB:
            copyInt32BufInt<16>(this->buffer[index], buffer, bufferSize);
            break;
        case BD_INT24LSB:
            copyInt32BufInt<24>(this->buffer[index], buffer, bufferSize);
            break;
        break;
        case BD_INT32LSB:
            copyInt32BufInt<32>(this->buffer[index], buffer, bufferSize);
            break;
        break;
        case BD_FLOAT32LSB:
            copyInt32BufFloat<32>(this->buffer[index], buffer, bufferSize);
            break;
        case BD_FLOAT64LSB:
            copyInt32BufFloat<64>(this->buffer[index], buffer, bufferSize);
            break;
        case BD_INT16MSB:
            copyInt32BufInt<32>(this->buffer[index], buffer, bufferSize);
            break;
        break;
        case BD_INT24MSB:
            copyInt32BufInt<32>(this->buffer[index], buffer, bufferSize);
            break;
        break;
        case BD_INT32MSB:
            copyInt32BufInt<32>(this->buffer[index], buffer, bufferSize);
            break;
        break;
        case BD_FLOAT32MSB:
            copyInt32BufFloat<32>(this->buffer[index], buffer, bufferSize);
            break;
        case BD_FLOAT64MSB:
            copyInt32BufFloat<64>(this->buffer[index], buffer, bufferSize);
            break;
        case BD_UNSUPPORTED:
        default:
        break;
    }
}
