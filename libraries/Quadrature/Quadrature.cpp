/******************************************************************************
 *  Quadrature.cpp - Arduino library for reading quadrature encoders.
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

#include "Quadrature.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h" 
#else
#include "WConstants.h"
#endif

#include <avr/interrupt.h>


const int _half [4][4] = {
    { 0, -1, 1, 0 },		//  00 -> 10 is CW
    { 1, 0, 0, -1 },		//  01 -> 00 is CW
    { -1, 0, 0, 1 },		//  10 -> 11 is CW
    { 0, 1, -1, 0 }		//  11 -> 01 is CW
};

const int _full [4][4] = {
    { 0, 0, 0, 0 },		//  00 -> 10 is silent CW
    { 1, 0, 0, -1 },		//  01 -> 00 is CW
    { -1, 0, 0, 1 },		//  10 -> 11 is CW
    { 0, 0, 0, 0 }		//  11 -> 01 is silent CW
};


Quadrature * Quadrature::_registry[DIGITAL_PINS];
//vector<Quadrature *> Quadrature::_registry;


Quadrature::Quadrature(int pin1, int pin2):
    _pin1(pin1), _pin2(pin2), _position(0),
    _min(0), _max(0), _usingmin(0), _usingmax(0) {

    pinMode(pin1, INPUT);
    pinMode(pin2, INPUT);
    digitalWrite(pin1, HIGH);		//  activate internal pullups
    digitalWrite(pin2, HIGH);

    _previous = _readpins();		//  read initial position

    TIMSK2 |= (1 << TOIE2);		//  enable timer 2 overflow interrupt

    _registry[pin1] = this;
    //_registry.push_back(this);
}


inline int Quadrature::_readpins(void) {
    return digitalRead(_pin2) << 1 | digitalRead(_pin1);
}

/*
  ((*portInputRegister(digitalPinToPort(_pin2))
    & digitalPinToBitMask(_pin2)) ? HIGH : LOW) << 1
  | ((*portInputRegister(digitalPinToPort(_pin1))
    & digitalPinToBitMask(_pin1)) ? HIGH : LOW)
*/


inline void Quadrature::isr(void) {
    int q;

    for (q = 0; q < DIGITAL_PINS; ++ q) {
	Quadrature *quad;

	if (_registry[q]) {
	    quad = _registry[q];
	    int quadbits;
	    quadbits = quad->_readpins();

	    if (quadbits != quad->_previous) {
		int position = quad->_position +
			_full[quad->_previous][quadbits];

		//  limit to minimum if assigned
		position = quad->_usingmin
			? max(quad->_min, position)
			: position;

		//  limit to maximum if assigned
		quad->_position = quad->_usingmax
			? min(quad->_max, position)
			: position;

		quad->_previous = quadbits;
	    }
	}
    }
}


ISR(TIMER2_OVF_vect) {
    Quadrature::isr();
}
