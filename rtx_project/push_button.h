#ifndef PUSH_BUTTON_H
#define PUSH_BUTTON_H
typedef enum {
	BUTTON_DOWN = 1,
	BUTTON_UP = 0
} button_state_type;
void button_init(void);
button_state_type button_read(void);
#endif
