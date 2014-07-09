/******************************************************************************
 *  Quadrature.h - Arduino library for reading quadrature encoders.
 *  Version 0.90
 *  Created by Keith Neufeld, June 29, 2008.
 *
 *  Modified to work with Arduino IDE version1 by Bill Holland
 *  Further modified to work on Version 1 and earlier by Jim Cairns
 *
 *  This work is licensed under the Creative Commons Attribution-Share Alike 
 *  3.0 Unported License. To view a copy of this license, visit
 *  http://creativecommons.org/licenses/by-sa/3.0/ or send a letter to
 *  Creative Commons, 171 Second Street, Suite 300, San Francisco, California,
 *  94105, USA.
 ******************************************************************************/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#include "WConstants.h"
#endif



#ifndef Quadrature_h
#define Quadrature_h

//#include <vector.h>



#define DIGITAL_PINS (14)


class Quadrature
{
    public:
	Quadrature(int pin1, int pin2);

	void minimum(int min);
	int minimum(void);
	void maximum(int max);
	int maximum(void);
	void nominimum(void);
	void nomaximum(void);

	int position(void);
	void position(int pos);

	static void isr(void);

    private:
	int _readpins(void);
	int _pin1, _pin2;

	volatile int _position;

	int _min, _max;
	bool _usingmin, _usingmax;

	volatile int _previous;

	static Quadrature* _registry[DIGITAL_PINS];
	//static vector<Quadrature *> _registry;
};


//  Inline functions

inline void Quadrature::minimum(int min) {
    _min = min;
    _usingmin = 1;

    //  adjust position if lower than new minimum
    _position = max(_position, min);
}

inline int Quadrature::minimum() {
    return _min;
}

inline void Quadrature::maximum(int max) {
    _max = max;
    _usingmax = 1;

    //  adjust position if higher than new maximum
    _position = min(_position, max);
}

inline int Quadrature::maximum() {
    return _max;
}

inline void Quadrature::nominimum(void) {
    _usingmin = 0;
}

inline void Quadrature::nomaximum(void) {
    _usingmax = 0;
}


inline int Quadrature::position(void) {
    return _position;
}

inline void Quadrature::position(int pos) {
    _position = pos;
}


#endif // Quadrature_h
