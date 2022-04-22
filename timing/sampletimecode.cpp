#include "sampletimecode.h"

static int sample;

void SampleTimeCode::init() { sample = 0; }
int SampleTimeCode::get() { return sample; }
void SampleTimeCode::tick() { sample++; }
