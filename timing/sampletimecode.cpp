#include "sampletimecode.h"

static double sample;

void SampleTimeCode::init() { sample = 0; }
double SampleTimeCode::get() { return sample; }
void SampleTimeCode::set(double _sample) { sample = _sample; }
