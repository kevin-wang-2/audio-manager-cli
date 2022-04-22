#ifndef SAMPLETIMECODE_H
#define SAMPLETIMECODE_H


class SampleTimeCode
{
    SampleTimeCode();
public:
    static void init();
    static int get();
    static void tick();
};

#endif // SAMPLETIMECODE_H
