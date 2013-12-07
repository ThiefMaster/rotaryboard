#include <avr/io.h>
#include <avr/interrupt.h>
#include "rotary.h"

#define READ_FIELD(NUM, FIELD) (*encoder_list[(NUM)].port_ ##FIELD & (1 << encoder_list[(NUM)].pin_ ##FIELD))
#define SET_FIELD(NUM, FIELD) *encoder_list[(NUM)].port_ ##FIELD |= (1 << encoder_list[(NUM)].pin_ ##FIELD)
#define CLR_FIELD(NUM, FIELD) *encoder_list[(NUM)].port_ ##FIELD &= ~(1 << encoder_list[(NUM)].pin_ ##FIELD)
#define MAKE_OUTPUT(NUM, FIELD)	*encoder_list[(NUM)].ddr_ ##FIELD |= (1 << encoder_list[(NUM)].pin_ ##FIELD)
#define MAKE_INPUT(NUM, FIELD) *encoder_list[(NUM)].ddr_ ##FIELD &= ~(1 << encoder_list[(NUM)].pin_ ##FIELD)
#define ENABLE_PULLUP(NUM, FIELD) SET_FIELD(NUM, FIELD)

// value is in 10ms units
#define BUTTON_DEOUNCE_DELAY 3


static struct rotary_encoder *encoder_list = 0;
static uint8_t num_encoders = 0;

static volatile int16_t enc_delta[MAX_ROTARY_ENCODERS];
static int8_t last[MAX_ROTARY_ENCODERS];

static uint8_t button_state[MAX_ROTARY_ENCODERS];
static uint8_t button_counter[MAX_ROTARY_ENCODERS];
static volatile uint8_t button_press[MAX_ROTARY_ENCODERS];
static uint8_t button_timer;


void rotary_init(struct rotary_encoder *encoders, uint8_t count)
{
	encoder_list = encoders;
	num_encoders = count;

	// configure ports
	for (int8_t i = 0; i < num_encoders; i++) {
		MAKE_OUTPUT(i, leds[0]);
		MAKE_OUTPUT(i, leds[1]);
		MAKE_INPUT(i, phase[0]);
		MAKE_INPUT(i, phase[1]);
		MAKE_INPUT(i, button);
		ENABLE_PULLUP(i, button);
	}

	// prepare data
	for (int8_t i = 0; i < num_encoders; i++) {
		int8_t new = 0;

		if (READ_FIELD(i, phase[0])) {
			new = 3;
		}
		if (READ_FIELD(i, phase[1])) {
			new ^= 1;
		}
		last[i] = new;
		enc_delta[i] = 0;
	}

	// Run timer with about 7.5 kHz
	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS01) | (1 << CS00);
	OCR0A = 37;
	TIMSK0 |= (1 << OCIE0A);
}


ISR(TIMER0_COMPA_vect)
{
	if (++button_timer == 75) {
		// cheap way to get a 10ms timer ;)
		button_timer = 0;
	}

	for (int8_t i = 0; i < num_encoders; i++) {
		int8_t new, diff, input;

		// update rotary data
		new = 0;
		if (READ_FIELD(i, phase[0])) {
			new = 3;
		}
		if (READ_FIELD(i, phase[1])) {
			new ^= 1;
		}
		diff = last[i] - new;
		if (diff & 1) {
			last[i] = new;
			enc_delta[i] += (diff & 2) - 1;
		}

		// update buttons if the 10ms timer is over
		if (!button_timer) {
			input = !READ_FIELD(i, button);
			if (input != button_state[i]) {
				if (--button_counter[i] == 0xff) {
					button_counter[i] = BUTTON_DEOUNCE_DELAY;
					button_press[i] = button_state[i] = input;
				}
			}
			else {
				button_counter[i] = BUTTON_DEOUNCE_DELAY;
			}
		}
	}
}


int16_t* rotary_read_all()
{
	static int16_t vals[MAX_ROTARY_ENCODERS];

	cli();
	for (uint8_t i = 0; i < num_encoders; i++) {
		int16_t tmp = enc_delta[i];
		enc_delta[i] = tmp & 3;
		vals[i] = tmp >> 2;
	}
	sei();
	return vals;
}


int16_t rotary_read(uint8_t num)
{
	int16_t val;

	cli();
	val = enc_delta[num];
	enc_delta[num] = val & 3;
	sei();
	return val >> 2;
}


uint8_t rotary_read_button(uint8_t num, uint8_t clear)
{
	uint8_t rc;

	cli();
	rc = button_press[num];
	if (clear) {
		button_press[num] = 0;
	}
	sei();

	return rc;

}


uint8_t* rotary_read_button_all(uint8_t clear)
{
	static uint8_t states[MAX_ROTARY_ENCODERS];

	cli();
	for (uint8_t i = 0; i < num_encoders; i++) {
		states[i] = button_press[i];
		if (clear) {
			button_press[i] = 0;
		}
	}
	sei();

	return states;

}


void rotary_set_leds(uint8_t num, uint8_t leds)
{
	for (uint8_t i = 0; i < 2; i++) {
		if (leds & (1 << i)) {
			SET_FIELD(num, leds[i]);
		}
		else {
			CLR_FIELD(num, leds[i]);
		}
	}
}