/*
 * libakm: this is a free replacement for the non-free libakm that comes with
 * android devices such as the Nexus S.
 * Copyright (C) 2011  Paul Kocialkowski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <cutils/log.h>

#include "akm.h"
#include "sensors/akm_sensors.h"

#ifdef TARGET_DEVICE_CRESPO
/* chip config with these */
#define	UINPUT		"/dev/uinput"
#define	INPUT_DIR	"/dev/input"

struct akm_chip_sensors *akm_device_chips[]=
{
	&kr3dm,
	&akm8973
};
#endif

/* default preset to add , maybe a switch on a filled define TARGET_DEVICE? */

/* This routine is called when the lib is dlopened */
void _init(void)
{
	int i;
	int device_chips_count=sizeof(akm_device_chips) / sizeof(void *);

	for(i=0 ; i < device_chips_count ; i++)
		akm_device_chips[i]->publisher->init(akm_device_chips[i]);
}

struct akm_sensor_info *akm_get_sensor(struct akm_chip_sensors *device_chips[], int device_chips_count, uint32_t sensor_type)
{
	int i, j;

	for(i=0 ; i < device_chips_count ; i++)
		for(j=0 ; j < device_chips[i]->sensors_count ; j++)
			if(device_chips[i]->sensors[j]->type == sensor_type)
				return device_chips[i]->sensors[j];

	LOGE("Failed to get the asked sensor (%d)\n", sensor_type);

	return (struct akm_sensor_info *) NULL;
}

int akm_is_sensor_enabled(uint32_t sensor_type)
{
	struct akm_sensor_info *sensor=NULL;

	sensor=akm_get_sensor(akm_device_chips, sizeof(akm_device_chips) / sizeof(void *), sensor_type);
	if(sensor == NULL)
		return 0;
	else
		return sensor->enabled;
}

int akm_enable_sensor(uint32_t sensor_type)
{
	int rc;
	struct akm_sensor_info *sensor=NULL;

	sensor=akm_get_sensor(akm_device_chips, sizeof(akm_device_chips) / sizeof(void *), sensor_type);
	if(sensor == NULL)
		return 1;

	rc=sensor->enable(sensor);

	return rc;
}

int akm_disable_sensor(uint32_t sensor_type)
{
	int rc;
	struct akm_sensor_info *sensor=NULL;

	sensor=akm_get_sensor(akm_device_chips, sizeof(akm_device_chips) / sizeof(void *), sensor_type);

	/* 
	 * Return 0 because it's nothing bad to return success while disabeling
	 * a non-existent sensor.
	 */
	if(sensor == NULL)
		return 0;

	rc=sensor->disable(sensor);

	return rc;
}

int akm_set_delay(uint32_t sensor_type, uint64_t delay)
{
	return 0;
}
