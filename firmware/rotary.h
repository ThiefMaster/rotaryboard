#ifndef ROTARY_H
#define ROTARY_H

#include <stdint.h>

// maximum supported number of rotary encoders.
// with 4 8-bit ports and 5 lines (2x data, 2x led, 1x button) 6 is the maximum
// it can be lowered to save some memory
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
	uint8_t pin_ ##FIELD;

#define _MAKE_IO_ARRAY(FIELD, SIZE) \
	volatile uint8_t *ddr_ ##FIELD [SIZE]; \
	volatile uint8_t *port_ ##FIELD [SIZE]; \
	uint8_t pin_ ##FIELD [SIZE];

struct rotary_encoder {
	_MAKE_IO_ARRAY(phase, 2)
	_MAKE_IO_FIELDS(button)
	_MAKE_IO_ARRAY(leds, 2)
};



// public api
#define ROTARY_ENCODER(PORT_PHASE_0, PIN_PHASE_0, PORT_PHASE_1, PIN_PHASE_1, PORT_BUTTON, PIN_BUTTON, PORT_LED_0, PIN_LED_0, PORT_LED_1, PIN_LED_1) { \
	{ & _MAKE_DDR(PORT_PHASE_0), & _MAKE_DDR(PORT_PHASE_1) }, \
	{ & _MAKE_PIN(PORT_PHASE_0), & _MAKE_PIN(PORT_PHASE_1) }, \
	{ _MAKE_P(PORT_PHASE_0, PIN_PHASE_0), _MAKE_P(PORT_PHASE_1, PIN_PHASE_1) }, \
	& _MAKE_DDR(PORT_BUTTON), & _MAKE_PIN(PORT_BUTTON), _MAKE_P(PORT_BUTTON, PIN_BUTTON), \
	{ & _MAKE_DDR(PORT_LED_0), & _MAKE_DDR(PORT_LED_1) }, \
	{ & _MAKE_PORT(PORT_LED_0), & _MAKE_PORT(PORT_LED_1) }, \
	{ _MAKE_P(PORT_LED_0, PIN_LED_0), _MAKE_P(PORT_LED_1, PIN_LED_1) } \
}


void rotary_init(struct rotary_encoder *encoders, uint8_t count);
int16_t* rotary_read_all();
int16_t rotary_read(uint8_t num);
uint8_t rotary_read_button(uint8_t num, uint8_t clear);
uint8_t* rotary_read_button_all(uint8_t clear);
void rotary_set_leds(uint8_t num, uint8_t leds);

#endif
