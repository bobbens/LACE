


#ifndef _HBRIDGE_H
#  define _HBRIDGE_H


/*
 * The commands.
 */
#define DHB_CMD_NONE     0x00 /**< Invalid command. */
#define DHB_CMD_VERSION  0x01 /**< Gets version. */
#define DHB_CMD_MODESET  0x02 /**< Sets operating mode. */
#define DHF_CMD_MODEGET  0x03 /**< Gets operating mode. */
#define DHB_CMD_MOTORSET 0x04 /**< Sets motor velocity. */
#define DHB_CMD_MOTORGET 0x05 /**< Gets motor velocity. */
#define DHB_CMD_CURRENT  0x06 /**< Gets motor current. */


/*
 * The modes.
 */
#define DHB_MODE_PWM    0x00 /**< PWM open loop mode. */
#define DHB_MODE_FBKS   0x01 /**< Feedback closed loop mode. */
#define DHB_MODE_TRQ    0x02 /**< Torque feedback loop mode. */


#endif /* _HBRIDGE_H */
