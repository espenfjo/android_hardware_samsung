/*
 * libakm: this is a free replacement for the non-free libakm that comes with
 * android devices such as the Nexus S.
 * Copyright (C) 2011  Paul Kocialkowski
 * Copyright (C) 2012  Espen Fjellvær Olsen <espen@mrfjo.org>
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

/*
 * The purpose of this file is to contain a mention to all the defined
 * chips, sensors and publishers. If you add support for a chip, you must add
 * these structures here (as external).
 */

/* Functions definitions, as defined in default.c. */
int default_enable(struct akm_sensor *sensor);
int default_disable(struct akm_sensor *sensor);

/* kr3dm */
extern struct akm_chip_sensors kr3dm;
extern struct akm_sensor kr3dm_accelerometer;

/* akm8975 */
extern struct akm_chip_sensors akm8975;
extern struct akm_publisher akm8975_publisher;
extern struct akm_sensor akm8975_magnetic_field;
extern struct akm_sensor akm8975_orientation;

#endif
