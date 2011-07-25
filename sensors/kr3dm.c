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

#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <pthread.h>

#include "kr3dm.h"

#include "akm.h"
#include "sensors/akm_sensors.h"

#define SENSOR_NAME	"kr3dm"

int kr3dm_init(struct akm_chip_sensors *chip)
{
	int fd;

	if(chip->inited)
		return 0;

	fd=open(chip->device_name, O_RDWR);
	if(fd < 0)
	{
		LOGE("Error while opening chip device: %s.\n", strerror(errno));
		return 1;
	}

	chip->fd=fd;

	chip->data_get(chip);

	chip->inited=1;

	return 0;
}

int kr3dm_deinit(struct akm_chip_sensors *chip)
{
	if(!chip->inited)
		return 0;

	if(chip->fd < 0)
	{
		LOGE("Error while closing chip device: negative fd.\n");
		return 1;
	}

	close(chip->fd);
	chip->fd=-1;

	chip->inited=0;

	return 0;
}

void kr3d_data_get_thread(void *chip_p)
{
	struct akm_chip_sensors *chip;
	struct kr3dm_acceldata data;
	struct akm_publish_vector data_vector;

	int rc;
	int i;

	int get_data=0;

	chip=(struct akm_chip_sensors *) chip_p;

	if(!chip->publisher->inited)
	{
		rc=chip->publisher->init(chip);
		if(rc)
		{
			LOGE("publisher init failed, aborting\n");
			return;
		}
	}

	do
	{
		get_data=0;

		for(i=0 ; i < chip->sensors_count ; i++)
		{
			if(chip->sensors[i]->enabled)
			{
				get_data=1;
			}
		}

		if(!get_data)
			return;

		rc=ioctl(chip->fd, KR3DM_IOCTL_READ_ACCEL_XYZ, &data);
		if(rc < 0)
		{
			LOGE("ioctl failed, aborting: %s\n", strerror(errno));
			return;
		}

		for(i=0 ; chip->sensors[i]->registered == AKM_REGISTERED ; i++)
		{
			if(chip->sensors[i]->enabled)
			{
				switch(chip->sensors[i]->type)
				{
					case SENSOR_TYPE_ACCELEROMETER:
						data_vector.x=data.x * -1;
						data_vector.y=data.y;
						data_vector.z=data.z;

						chip->publisher->data_publish(chip, chip->sensors[i]->type, &data_vector);
					break;
				}
			}
		}
	} while(get_data);

	/* thsi should be in deinit */
	pthread_mutex_unlock(&chip->mutex);

	chip->publisher->deinit(chip);	
}

void kr3d_data_get(struct akm_chip_sensors *chip)
{
	pthread_t thread;

	pthread_mutex_lock(&chip->mutex);
	pthread_create(&thread, NULL, kr3d_data_get_thread, chip);
}

struct akm_chip_sensors kr3dm = {
	.registered=AKM_REGISTERED,
	.publisher=&akm8973_publisher,
	.sensors_count=1,
	.sensors={
		&kr3dm_accelerometer
	},
	.data_get=kr3d_data_get,
	.init=kr3dm_init,
	.inited=0,
	.fd=-1,
	.device_name="/dev/accelerometer",
	.mutex=PTHREAD_MUTEX_INITIALIZER,
};

struct akm_sensor_info kr3dm_accelerometer = {
	.registered=AKM_REGISTERED,
	.type=SENSOR_TYPE_ACCELEROMETER,
	.enabled=0,
	.enable=default_enable,
	.disable=default_disable,
	.chip=&kr3dm,
};
