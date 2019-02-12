#ifndef SERVO_H
#define SERVO_H

/*
 * Initializes PortD to be used as controller signals for 6 servos
 * and timer2 for servo
 */
void servo_Init(void);

/*
 * moves the servo with index by setting rad angles
 * index: The number of servo being rotated
 * rad: The rotate angle, in radius. A positive value is clockwise. 
 */
void rotate_servo(int index, float rad);
#endif     //SERVO_H