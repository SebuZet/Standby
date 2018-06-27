/*
* Copyright (c) 2014-2018
* Konrad Kusnierz <iryont@gmail.com>
*
* All information, intellectual and technical concepts contained herein is,
* and remains the property of the above person. Dissemination of this
* information or reproduction of this material is strictly forbidden unless
* prior written permission is obtained.
*
* Modified by Sebastian Zaorski
*/

#include "standby.h"
#include "config.h"
#include "dev/led.h"
#include "dev/relay.h"
#include "dev/player.h"
#include "irmp/irmp.h"

// standby structure with all the necessary information
struct Standby* standby = NULL;

static const uint16_t CONFIG_RCU_CHECK_INTERVAL					=    100;
static const uint16_t CONFIG_RCU_LEARNING_TIMEOUT				=  10000;
static const uint16_t CONFIG_LEDS_BLINKING_TIMEOUT				=    500;
static const uint16_t CONFIG_BUTTON_CHECK_INTERVAL				=   1500;
static const uint16_t CONFIG_PLAYER_POWEROFF_TIMEOUT			=  10000;
static const uint16_t CONFIG_STARTUP_DELAY						=    300;
static const uint16_t CONFIG_SHUTDOWN_REQUEST_INTERVAL			=   1000;

void Standby_LedsBlinkingHandler(void);
void Standby_RcuLearningTimeout(void);
void Standby_ButtonListener(enum Buttonstate_t state);
void Standby_RcuHandler(void);
void Standby_PowerOffTimeout(void);
void Standby_ShutDownRequestHandler(void);

void Standby_TryToggleMode(void);
void Standby_SynchronizeLeds(void);
void Standby_enterNormalMode(void);
void Standby_enterLearningMode(void);

#define doesRcuCodeMatch(c1, c2)(c1.protocol == c2.protocol && c1.address == c2.address && c1.command == c2.command)

void Standby_enterNormalMode(void)
{
	standby->mode = STANDBY_MODE_NORMAL;
	Button01_join(Standby_ButtonListener);
	Standby_SynchronizeLeds();
	Tasks_create(CONFIG_SHUTDOWN_REQUEST_INTERVAL, TASK_REPEAT, Standby_ShutDownRequestHandler, NULL);
}

void Standby_enterLearningMode(void)
{
	standby->mode = STANDBY_MODE_LEARN_POWER_ON;
	Tasks_create(CONFIG_RCU_LEARNING_TIMEOUT, TASK_SCHEDULE, Standby_RcuLearningTimeout, NULL);
	Standby_SynchronizeLeds();
}

void Standby_modeSelection(void)
{
	if (Button01_isPressed())
	{
		Standby_enterLearningMode();
	}
	else
	{
		Standby_enterNormalMode();
	}

	// private tasks
	Tasks_create(CONFIG_RCU_CHECK_INTERVAL, TASK_REPEAT, Standby_RcuHandler, NULL);
}

void Standby_init(void)
{
	// create standby structure
	standby = (struct Standby*)malloc(sizeof(struct Standby));

	// zero-fill
	memset(standby, 0, sizeof(struct Standby));

	// load configuration
	standby->config = Config_get();

	// normal mode
	standby->mode = STANDBY_MODE_STARTING;

	// update LEDs
	Standby_SynchronizeLeds();
	
	Player_NotifySystemShutDown(FALSE);

	// wait a moment and check if user is holding button to enter RCU learning mode
	Tasks_create(CONFIG_STARTUP_DELAY, TASK_SCHEDULE, Standby_modeSelection, NULL);

	// public tasks
	standby->blinkTask = Tasks_create(CONFIG_LEDS_BLINKING_TIMEOUT, TASK_REPEAT, Standby_LedsBlinkingHandler, NULL);
}

void Standby_LedsBlinkingHandler(void)
{
	if (standby->mode == STANDBY_MODE_NORMAL)
	{
		LED_off(LED_MAIN);
	}
	else if (standby->mode == STANDBY_MODE_GOING_TO_STANDBY)
	{
		LED_off(LED_MAIN);
		LED_toggle(LED_POWER_ON); // blink standby led while waiting for SBC player to turn off
	}
	else if (standby->mode == STANDBY_MODE_LEARN_POWER_OFF || standby->mode == STANDBY_MODE_LEARN_POWER_ON)
	{
		LED_toggle(LED_MAIN);
	}
}

void Standby_RcuLearningTimeout(void)
{
	if (standby->mode == STANDBY_MODE_LEARN_POWER_ON)			// clear current RCU settings
	{
		Config_reset();
		Config_save();
		Standby_enterNormalMode();
	}
	else if (standby->mode == STANDBY_MODE_LEARN_POWER_OFF)		// use one RCU key for power ON and power OFF
	{
		standby->config->remoteCodeOff = standby->config->remoteCodeOn;
		Config_save();
		Standby_enterNormalMode();
	}
}

void Standby_ButtonListener(enum Buttonstate_t state)
{
	LED_on(LED_MAIN);
	Tasks_restart(standby->blinkTask);
	Standby_TryToggleMode();
}

void Standby_RcuHandler(void)
{
	IRMP_DATA irmp_data;

	if (irmp_get_data (&irmp_data))
	{
		if (standby->mustIgnoreRemote > 0)
			--standby->mustIgnoreRemote;

		switch (standby->mode)
		{
			case STANDBY_MODE_LEARN_POWER_ON:
			{
				if (standby->mustIgnoreRemote == 0)
				{
					standby->config->remoteCodeOn = irmp_data;
					standby->mode = STANDBY_MODE_LEARN_POWER_OFF;
					standby->mustIgnoreRemote = STANDBY_IGNORE_REMOTE;			
				}
				break;
			}
			
			case STANDBY_MODE_LEARN_POWER_OFF:
			{
				if (standby->mustIgnoreRemote == 0)
				{
					standby->config->remoteCodeOff = irmp_data;
					Config_save();
					Standby_enterNormalMode();
					standby->mustIgnoreRemote = STANDBY_IGNORE_REMOTE;
				}
				break;
			}
			
			case STANDBY_MODE_NORMAL:
			{
				BOOL bSwitchStandby = FALSE;
				if (!doesRcuCodeMatch(standby->config->remoteCodeOn, standby->config->remoteCodeOff))
				{
					if (Relay_get()) // if power is already ON
					{
						bSwitchStandby = doesRcuCodeMatch(irmp_data, standby->config->remoteCodeOff); // check power off RCU code
					}
					else // power is OFF
					{
						bSwitchStandby = doesRcuCodeMatch(irmp_data, standby->config->remoteCodeOn); // check for power on RCU code
					}
				}
				else
				{
					bSwitchStandby = doesRcuCodeMatch(irmp_data, standby->config->remoteCodeOn);
				}

				if (bSwitchStandby)
				{
					if (standby->mustIgnoreRemote == 0)
					{
						Standby_TryToggleMode();
						standby->mustIgnoreRemote = STANDBY_IGNORE_REMOTE;
	
						// blink
						LED_on(LED_MAIN);
						Tasks_restart(standby->blinkTask);
					}
				}

				break;
			}
			
			case STANDBY_MODE_GOING_TO_STANDBY:
				// ignore RCU commands, system is going into standby, waiting for SBC to shutdown
				break;
			
			default:
				standby->mustIgnoreRemote = 0;
				// TODO: send RCU command via UART
				break;
		}
	}
}

void Standby_ToggleMode(void)
{
	Player_NotifySystemShutDown(FALSE);

	// toggle relay
	Relay_toggle();

	// update LEDs
	Standby_SynchronizeLeds();
}

void Standby_PowerOffTimeout(void)
{
	if (standby->mode == STANDBY_MODE_GOING_TO_STANDBY)
	{
		standby->mode = STANDBY_MODE_NORMAL;
		Standby_ToggleMode();
	}
}

void Standby_ShutDownRequestHandler(void)
{
	if (standby->mode == STANDBY_MODE_NORMAL && Relay_get() && Player_isShutdownRequested())
	{
		Standby_TryToggleMode();
	}
}

void Standby_TryToggleMode(void)
{
	if (standby->mode == STANDBY_MODE_NORMAL)
	{
		if (Relay_get() && Player_isConnected())
		{
			standby->mode = STANDBY_MODE_GOING_TO_STANDBY;		// set mode to proper one
			Player_NotifySystemShutDown(TRUE);							// inform SBC that system is going down
			Tasks_create(CONFIG_PLAYER_POWEROFF_TIMEOUT, TASK_SCHEDULE, Standby_PowerOffTimeout, NULL);
			Tasks_restart(standby->blinkTask);					// reset timeout
			Standby_SynchronizeLeds();
		}
		else
		{
			Standby_ToggleMode();								// switch standby ode
		}
	}
}

void Standby_SynchronizeLeds(void)
{
	switch (standby->mode)
	{
		case STANDBY_MODE_NORMAL:
		{
			LED_set(LED_POWER_ON, Relay_get());
			LED_set(LED_POWER_OFF, !Relay_get());
			break;
		}

		case STANDBY_MODE_STARTING:
		{
			LED_on(LED_POWER_ON);
			LED_on(LED_POWER_OFF);
			break;
		}

		case STANDBY_MODE_GOING_TO_STANDBY:
		{
			LED_on(LED_POWER_ON);
			LED_off(LED_POWER_OFF);
			break;
		}

		case STANDBY_MODE_LEARN_POWER_ON:
		case STANDBY_MODE_LEARN_POWER_OFF:
		{
			LED_off(LED_POWER_ON);
			LED_off(LED_POWER_OFF);
			break;
		}
		
		default: // error???
		{
			LED_on(LED_POWER_ON);
			LED_on(LED_POWER_OFF);
			LED_on(LED_MAIN);
		}
	}
}
