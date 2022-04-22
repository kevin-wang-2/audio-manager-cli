#ifndef SAMPLETIMECODE_H
#define SAMPLETIMECODE_H


class SampleTimeCode
{
    SampleTimeCode();
public:
    static void init();
    static double get();
    static void set(double sample);
};

#endif // SAMPLETIMECODE_H
