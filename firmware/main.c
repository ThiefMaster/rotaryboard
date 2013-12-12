#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "rotary.h"

#define NUM_ENCODERS 5
#define NUM_BUTTONS 2
#define NUM_INPUTS (NUM_ENCODERS + NUM_BUTTONS)

static struct rotary_encoder rotary_encoders[NUM_INPUTS] = {
	ROTARY_ENCODER(/* phases */ B, 1, B, 2, /* button */ B, 3, /* leds */ B, 0, A, 2),
	ROTARY_ENCODER(/* phases */ A, 3, A, 1, /* button */ A, 0, /* leds */ A, 4, A, 5),
	ROTARY_ENCODER(/* phases */ C, 7, C, 6, /* button */ A, 7, /* leds */ A, 6, C, 5),
	ROTARY_ENCODER(/* phases */ D, 7, C, 1, /* button */ C, 2, /* leds */ C, 4, C, 3),
	ROTARY_ENCODER(/* phases */ D, 2, D, 3, /* button */ D, 4, /* leds */ D, 6, D, 5),
	LED_BUTTON(/* button */ B, 5, /* led */ C, 0),
	LED_BUTTON(/* button */ B, 6, /* led */ B, 4)
};

static volatile uint8_t t2_cnt;
static volatile uint8_t t2_triggered;


ISR(TIMER2_COMPA_vect)
{
	if (++t2_cnt == 5) {
		// 20hz here
		t2_cnt = 0;
		t2_triggered = 1;
	}
}

/*
 * PC ==> uC commands:
 *   RLED.n=c   set rotary encoder LEDs (n = 0..6, c = 0,R,G,Y)
 *   RST        reset the controller
 *
 *  uC ==> PC commands:
 *   READY      controller initialized and ready to send/receive data
 *   RVAL.n=i   rotary encoder value changed
 *   RBTN.n=b   rotary encoder button changed (n = 0..6, b = 0..1)
 */

int main()
{
	cli();

	// Disable watchdog in case we used it to force a reset
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	rotary_init(rotary_encoders, NUM_INPUTS);
	for (uint8_t i = NUM_ENCODERS; i < NUM_INPUTS; i++) {
		rotary_set_leds(i, 3);
	}

	// run t2 with 100hz
	TCCR2A = (1 << WGM21);
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
	OCR2A = 179;
	TIMSK2 |= (1 << OCIE2A);

	uart_init();
	_delay_ms(250); // otherwise the first transmission becomes nonsense

	sei();
	uart_puts("\r\nREADY\r\n");

	uint8_t last_button_states[NUM_INPUTS] = { 0 };
	while (1) {
		uint8_t check_encoder_vals = t2_triggered;
		t2_triggered = 0;

		// Read and send encoder values. To reduce jitter we do this with ~20hz
		if (check_encoder_vals) {
			int16_t *encoder_vals = rotary_read_all();
			for (uint8_t i = 0; i < NUM_ENCODERS; i++) {
				if (encoder_vals[i]) {
					uart_printf("RVAL.%u=%d\r\n", i, encoder_vals[i]);
				}
			}
		}

		// Read button values
		uint8_t *button_states = rotary_read_button_all(0);
		for (uint8_t i = 0; i < NUM_INPUTS; i++) {
			if (last_button_states[i] != button_states[i]) {
				uart_printf("RBTN.%u=%d\r\n", i, button_states[i]);
			}
			last_button_states[i] = button_states[i];
		}

		const char *line;
		while ((line = uart_getline())) {
			uint8_t handled = 0;
			if (!strcmp(line, "RST")) {
				handled = 1;
				wdt_enable(WDTO_15MS);
			}
			else if (!strncmp(line, "RLED", 4) && strlen(line) == 8 && line[4] == '.' && line[6] == '=') {
				uint8_t num = line[5] - '0';
				char color = line[7];
				uint8_t leds = 0xff;
				if (color == '0') leds = 0;
				else if (color == 'R') leds = 1;
				else if (color == 'G' && num < 5) leds = 2;
				else if (color == 'Y' && num < 5) leds = 3;
				if (num < NUM_INPUTS && leds != 0xff) {
					handled = 1;
					rotary_set_leds(num, leds);
				}
			}
			if (!handled) {
				uart_puts("?> ");
				uart_puts(line);
				uart_puts("\r\n");
			}
		}
	}

	return 0;
}
