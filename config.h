#ifndef CONFIG_H
#define CONFIG_H

/* Default recording device, sample rate. Sound is always recorded in
 * mono. The device can be overriden at runtime using the environment
 * variable SOUND_DEVICE_ENV. */
#define SOUND_DEVICE_ENV (char*)"RTSPECCY_CAPTURE_DEVICE"

#define CLEFT 0
#define CRIGHT 2

#define _I_ 0
#define _Q_ 1


#endif /* CONFIG_H */

/* vim: set ft=cpp : */
