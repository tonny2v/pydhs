/*
 * Algorithm.cpp
 *
 *  Created on: May 24, 2014
 *      Author: tonny
 */

#include "algorithm.h"

Algorithm::Algorithm() {
    start_ms = get_now_ms();
}

Algorithm::~Algorithm() {
    //TODO Auto-generated destructor stub
}

int Algorithm::get_now_ms() const
{
    timeb tb;
    ftime(&tb);
    int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
    return nCount;
}

int Algorithm::get_elapesd_ms() const{
    int nSpan = get_now_ms() - start_ms;
    if (nSpan < 0)
        nSpan += 0x100000 * 1000;
    return nSpan;
}




