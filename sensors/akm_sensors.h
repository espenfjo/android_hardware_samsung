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

#ifndef __AKM_SENSORS_H__
#define __AKM_SENSORS_H__

#include "akm.h"

int default_enable(struct akm_sensor_info *sensor_info);
int default_disable(struct akm_sensor_info *sensor_info);

/* kr3dm */
extern struct akm_chip_sensors kr3dm;
extern struct akm_sensor_info kr3dm_accelerometer;

/* akm8973 */
extern struct akm_chip_sensors akm8973;
extern struct akm_publisher akm8973_publisher;
extern struct akm_sensor_info akm8973_magnetic_field;
extern struct akm_sensor_info akm8973_orientation;

#endif
