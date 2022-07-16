#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <string>
#include "bitboards.h"

using namespace std;

#define FENTOSQ(i) (8*(7-(i/8))+(i%8))

tBoard* ParseFEN(string fen);
void setCounterFrequency();
double getPCFrequency();
void startCounter(int counterId);
void updateCounter(int counterId, double *countVar);

#endif