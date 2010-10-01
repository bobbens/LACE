
#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>


/*
 * MMA7260QT 3-Axis Accelerometer
 * Pololu Robotics and Electronics
 *
 * Jumper setup
 *
 *    GS1  GS2   Description
 *    on   on    ±1.5g
 *    on   off   ±2.0g
 *    off  on    ±4.0g
 *    off  off   ±6.0g
 *
 * For 5 V microcontrollers do not drive lines high:
 *
 *    on  = high impedance
 *    low = drive low
 */


/*
 * Sharp GP2Y0A21YK0F Analog Distance Sensor 10-80cm
 * SHARP
 *
 * distance = A + B / voltage
 *
 * Where A and B must be calibrated.
 *
 * B is aproximately 27 V*cm.
 */
uint16_t sharp_get (void)
{
   uint16_t adc_val;

   adc_val = (ADCH<<8) + ADCL;

   return 0 + (27*1024) / adc_val;
}


