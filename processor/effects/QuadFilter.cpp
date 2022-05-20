#include "QuadFilter.h"

double BiQuad::calculateSample(double sample) {
    double y;
    y = b0 * sample + b1 * xd + b2 * xdd - a1 * yd -
        a2 * ydd;
    xdd = xd;
    ydd = yd;
    xd = sample;
    yd = y;
    return y;
}

void BiQuad::reset() {
    xd = xdd = yd = ydd = 0;
}