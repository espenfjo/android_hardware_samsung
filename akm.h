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

#ifndef __AKM_H__
#define __AKM_H__

#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <pthread.h>

#include "hardware/sensors.h"

#define AKM_REGISTERED 42

struct akm_sensor_info
{
	uint8_t registered;
	uint8_t type;
	uint8_t enabled;

	int (*enable)(struct akm_sensor_info *sensor_info);
	int (*disable)(struct akm_sensor_info *sensor_info);
	int (*set_delay)(struct akm_sensor_info *sensor_info, uint64_t delay);

	struct akm_chip_sensors *chip;
};

struct akm_publisher
{
	int control_fd;
	int publish_fd;
	char *input_name;
	uint8_t inited;
	int (*init)(struct akm_chip_sensors *chip);
	int (*deinit)(struct akm_chip_sensors *chip);
	void (*data_publish)(struct akm_chip_sensors *chip, uint8_t type, void *data);
};

struct akm_chip_sensors
{
	uint8_t registered;
	struct akm_publisher *publisher;
	void (*data_get)(struct akm_chip_sensors *chip);
	int (*init)(struct akm_chip_sensors *chip);
	int (*deinit)(struct akm_chip_sensors *chip);
	uint8_t inited;
	char *device_name;
	int fd;
	pthread_mutex_t mutex;
	int sensors_count;
	struct akm_sensor_info *sensors[];
};

struct akm_publish_vector
{
	int x;
	int y;
	int z;
};

#endif
