#ifndef SERVO_H
#define SERVO_H

// Hardware limitations information
#define MIN_SERVO_PULSE_WIDTH 		544
#define MAX_SERVO_PULSE_WIDTH 		2400
#define DEFAULT_SERVO_PULSE_WIDTH       1500
#define REFRESH_INTERVAL 		20000

#define SERVO_NUM                       4

#define F_CPU                           8000000
#define SERVO_MAX       180
#define SERVO_MIN       35

// command
#define DISPENSE 1
#define SERVO_READY 2

//#define SERVO_MAX 140
//#define SERVO_MIN 45

/*
 * Initializes PortD to be used as controller signals for 6 servos
 * and timer2 for servo
 */
void servo_Init();

/*
 * write a pulse width of the given degress to the servo with index number
 */
void servo_write(uint32_t index, uint32_t val);

//void dispense();

void dispenseSlot(int slotNum);

#endif     //SERVO_H