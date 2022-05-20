#ifndef AUDIO_MANAGER_CLI_QUADFILTER_H
#define AUDIO_MANAGER_CLI_QUADFILTER_H


class BiQuad {
    double xd = 0, xdd = 0, yd = 0, ydd = 0;
public:
    double a1, a2, b0, b1, b2; // Assume a0 = 1

    double calculateSample(double sample);
    void reset();
};


#endif //AUDIO_MANAGER_CLI_QUADFILTER_H
