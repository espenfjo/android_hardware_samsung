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

#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>

#include "akm.h"
#include "sensors/akm_sensors.h"

#define SENSOR_NAME	"akm8973"

int akm8973_init(struct akm_chip_sensors *chip)
{
	int rc;
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

	/* Do more if needed */

	chip->inited=1;

	return 0;
}

int akm8973_deinit(struct akm_chip_sensors *chip)
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

void akm8973_data_publish(struct akm_chip_sensors *chip, uint8_t type, void *data)
{
	struct input_event event;
	memset(&event, 0, sizeof(struct input_event));

	switch(type)
	{
		case SENSOR_TYPE_ACCELEROMETER:
			event.type=EV_REL;
			event.code=REL_X;
			event.value=((struct akm_publish_vector *)data)->x;
			write(chip->publisher->control_fd, &event, sizeof(event));

			event.type=EV_REL;
			event.code=REL_Y;
			event.value=((struct akm_publish_vector *)data)->y;
			write(chip->publisher->control_fd, &event, sizeof(event));

			event.type=EV_REL;
			event.code=REL_Z;
			event.value=((struct akm_publish_vector *)data)->z;
			write(chip->publisher->control_fd, &event, sizeof(event));

			event.type=EV_SYN;
			event.code=0;
			event.value=0;
			write(chip->publisher->control_fd, &event, sizeof(event));
		break;
	}
	return;
}

int akm8973_publisher_init(struct akm_chip_sensors *chip)
{
	struct uinput_user_dev uinput_dev;
	int rc;
	int control_fd;
	int publish_fd;

	memset(&uinput_dev, 0, sizeof(uinput_dev));

	strcpy(uinput_dev.name, chip->publisher->input_name);

	uinput_dev.id.bustype=BUS_I2C;
	uinput_dev.id.vendor=0;
	uinput_dev.id.product=0;
	uinput_dev.id.version=0;

	control_fd=open("/dev/uinput", O_RDWR);
	if(control_fd < 0)
	{
		LOGE("Error while opening uinput device: %s.\n", strerror(errno));
		return 1;
	}

	if(ioctl(control_fd, UI_SET_EVBIT, EV_REL) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(control_fd, UI_SET_RELBIT, REL_X) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(control_fd, UI_SET_RELBIT, REL_Y) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(control_fd, UI_SET_RELBIT, REL_Z) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(write(control_fd, &uinput_dev, sizeof(uinput_dev)) < 0)
	{
		LOGE("Error while writing uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(control_fd, UI_DEV_CREATE) < 0)
	{
		LOGE("Error while creating uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	publish_fd=default_open_input_by_name(chip->publisher->input_name);

	chip->publisher->control_fd=control_fd;
	chip->publisher->publish_fd=publish_fd;
	chip->publisher->inited=1;

	return 0;
}

int akm8973_publisher_deinit(struct akm_chip_sensors *chip)
{
	int rc;

	if(ioctl(chip->publisher->control_fd, UI_DEV_DESTROY) < 0)
	{
		LOGE("Error while destroying uinput device: %s.\n", strerror(errno));
	}

	if(close(chip->publisher->control_fd) < 0)
	{
		LOGE("Error while closing uinput device: %s.\n", strerror(errno));
		return 1;
	}

	if(close(chip->publisher->publish_fd) < 0)
	{
		LOGE("Error while closing uinput device: %s.\n", strerror(errno));
		return 1;
	}

	chip->publisher->control_fd=-1;
	chip->publisher->publish_fd=-1;
	chip->publisher->inited=0;
	return 0;
}

struct akm_chip_sensors akm8973 = {
	.registered=AKM_REGISTERED,
	.publisher=&akm8973_publisher,
	.sensors_count=2,
	.sensors={
		&akm8973_magnetic_field,
		&akm8973_orientation
	},
//	.data_get=akm8973_data_get,
	.init=akm8973_init,
	.inited=0,
	.fd=-1,
	.device_name="/dev/akm8973",
};

struct akm_publisher akm8973_publisher = 
{
	.control_fd=-1,
	.publish_fd=-1,
	.input_name="compass",
	.inited=0,
	.init=akm8973_publisher_init,
	.deinit=akm8973_publisher_deinit,
	.data_publish=akm8973_data_publish,
};

struct akm_sensor_info akm8973_magnetic_field = {
	.registered=AKM_REGISTERED,
	.type=SENSOR_TYPE_MAGNETIC_FIELD,
	.enabled=0,
	.enable=default_enable,
	.disable=default_disable,
	.chip=&akm8973,
};

struct akm_sensor_info akm8973_orientation = {
	.registered=AKM_REGISTERED,
	.type=SENSOR_TYPE_ORIENTATION,
	.enabled=0,
	.enable=default_enable,
	.disable=default_disable,
	.chip=&akm8973,
};
