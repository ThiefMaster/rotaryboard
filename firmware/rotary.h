#ifndef ROTARY_H
#define ROTARY_H

#include <stdint.h>

// maximum supported number of rotary encoders.
// with 4 8-bit ports and 5 lines (2x data, 2x led, 1x button) 6 is the maximum
// it can be lowered to save some memory
// note that all elements in the struct passed to rotary_init are counted, i.e.
// buttons/leds handled by this library must be considered, too!
#ifndef MAX_ROTARY_ENCODERS
#define MAX_ROTARY_ENCODERS 6
#endif


// helper macros
#define _MAKE_DDR(LETTER) DDR ##LETTER
#define _MAKE_PORT(LETTER) PORT ##LETTER
#define _MAKE_PIN(LETTER) PIN ##LETTER
#define _MAKE_P(LETTER, NUM) P ##LETTER ##NUM


#define _MAKE_IO_FIELDS(FIELD) \
	volatile uint8_t *ddr_ ##FIELD; \
	volatile uint8_t *port_ ##FIELD; \
	volatile uint8_t *pullup_ ##FIELD; \
	uint8_t pin_ ##FIELD;

#define _MAKE_IO_ARRAY(FIELD, SIZE) \
	volatile uint8_t *ddr_ ##FIELD [SIZE]; \
	volatile uint8_t *port_ ##FIELD [SIZE]; \
	uint8_t pin_ ##FIELD [SIZE];


#define FEAT_ROTARY 0x1
#define FEAT_BUTTON 0x2
#define FEAT_LED 0x4
#define FEAT_LED2 0x8


struct rotary_encoder {
	uint8_t features;
	_MAKE_IO_ARRAY(phase, 2)
	_MAKE_IO_FIELDS(button)
	_MAKE_IO_ARRAY(leds, 2)
};



// public api

// defines a rotary encoder with two leds and a button
#define ROTARY_ENCODER(PORT_PHASE_0, PIN_PHASE_0, PORT_PHASE_1, PIN_PHASE_1, PORT_BUTTON, PIN_BUTTON, PORT_LED_0, PIN_LED_0, PORT_LED_1, PIN_LED_1) { \
	FEAT_ROTARY | FEAT_BUTTON | FEAT_LED | FEAT_LED2, \
	{ & _MAKE_DDR(PORT_PHASE_0), & _MAKE_DDR(PORT_PHASE_1) }, \
	{ & _MAKE_PIN(PORT_PHASE_0), & _MAKE_PIN(PORT_PHASE_1) }, \
	{ _MAKE_P(PORT_PHASE_0, PIN_PHASE_0), _MAKE_P(PORT_PHASE_1, PIN_PHASE_1) }, \
	& _MAKE_DDR(PORT_BUTTON), & _MAKE_PIN(PORT_BUTTON), & _MAKE_PORT(PORT_BUTTON), _MAKE_P(PORT_BUTTON, PIN_BUTTON), \
	{ & _MAKE_DDR(PORT_LED_0), & _MAKE_DDR(PORT_LED_1) }, \
	{ & _MAKE_PORT(PORT_LED_0), & _MAKE_PORT(PORT_LED_1) }, \
	{ _MAKE_P(PORT_LED_0, PIN_LED_0), _MAKE_P(PORT_LED_1, PIN_LED_1) } \
}

// when using buttons make sure to place them AFTER the actual rotary encoders
// the first item that does not have FEAT_ROTARY breaks rotary-related loops
#define LED_BUTTON(PORT_BUTTON, PIN_BUTTON, PORT_LED, PIN_LED) { \
	FEAT_BUTTON | FEAT_LED, \
	{ NULL, NULL }, \
	{ NULL, NULL }, \
	{ 0, 0 }, \
	& _MAKE_DDR(PORT_BUTTON), & _MAKE_PIN(PORT_BUTTON), & _MAKE_PORT(PORT_BUTTON), _MAKE_P(PORT_BUTTON, PIN_BUTTON), \
	{ & _MAKE_DDR(PORT_LED), NULL }, \
	{ & _MAKE_PORT(PORT_LED), NULL }, \
	{ _MAKE_P(PORT_LED, PIN_LED), 0 } \
}


void rotary_init(struct rotary_encoder *encoders, uint8_t count);
int16_t* rotary_read_all();
int16_t rotary_read(uint8_t num);
uint8_t rotary_read_button(uint8_t num, uint8_t clear);
uint8_t* rotary_read_button_all(uint8_t clear);
void rotary_set_leds(uint8_t num, uint8_t leds);

#endif
