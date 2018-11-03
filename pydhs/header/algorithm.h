/*
 * Algorithm.h
 *
 *  Created on: May 24, 2014
 *      Author: tonny
 */

#ifndef ALGORITHM_H
#define ALGORITHM_H
#include <sys/timeb.h>

class Algorithm {
private:
    
    int start_ms;
    
    int get_now_ms() const;
    
public:
    
    Algorithm();
    
    ~Algorithm();
    
    int get_elapesd_ms() const;
};

#endif /* ALGORITHM_H_ */
