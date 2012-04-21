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

#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 

#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>

#include "akm.h"
#include "sensors/akm_sensors.h"

#define SENSOR_NAME	"akm8975"

int akm8975_init(struct akm_chip_sensors *chip)
{
	int rc;
	int fd;

	if(chip->inited)
		return 0;

	/* Add the file descriptor to the chip struct. */
	fd=open(chip->device_name, O_RDWR);
	if(fd < 0)
	{
		LOGE("Error while opening chip device: %s.\n", strerror(errno));
		return 1;
	}

	chip->fd=fd;

	/* Do more if needed */

	chip->inited=1;

exit:
	/* Finally, start to get data from the chip. */
//	chip->data_get(chip);

	return 0;
}

int akm8975_deinit(struct akm_chip_sensors *chip)
{
	if(!chip->inited)
		return 0;

	if(chip->fd < 0)
	{
		LOGE("Error while closing chip device: negative fd.\n");
		return 1;
	}

	/* Close the descriptor that gives access to the data. */
	close(chip->fd);
	chip->fd=-1;

	chip->inited=0;

	return 0;
}

/* This function publishes the data to the specified input node. */
void akm8975_data_publish(struct akm_chip_sensors *chip, uint8_t type, void *data)
{
	struct input_event event;
	memset(&event, 0, sizeof(struct input_event));

	switch(type)
	{
	case SENSOR_TYPE_ACCELEROMETER:
		event.type=EV_REL;
		event.code=REL_X;
		event.value=((struct akm_publish_vector *)data)->x;
		write(chip->publisher->fd, &event, sizeof(event));

		event.type=EV_REL;
		event.code=REL_Y;
		event.value=((struct akm_publish_vector *)data)->y;
		write(chip->publisher->fd, &event, sizeof(event));

		event.type=EV_REL;
		event.code=REL_Z;
		event.value=((struct akm_publish_vector *)data)->z;
		write(chip->publisher->fd, &event, sizeof(event));

		event.type=EV_SYN;
		event.code=0;
		event.value=0;
		write(chip->publisher->fd, &event, sizeof(event));
		break;
	}
	return;
}

int akm8975_publisher_init(struct akm_chip_sensors *chip)
{
	struct uinput_user_dev uinput_dev;
	int rc;
	int fd;

	/* If the publisher is already inited, reeturn success. */
	if(chip->publisher->inited)
		return 0;

	memset(&uinput_dev, 0, sizeof(uinput_dev));

	/* Set the uinput device name. */
	strcpy(uinput_dev.name, chip->publisher->input_device_name);

	/* Set the ids of the uinput device. */
	uinput_dev.id.bustype=BUS_I2C;
	uinput_dev.id.vendor=0;
	uinput_dev.id.product=0;
	uinput_dev.id.version=0;

	/* Open the uinput node. */
	fd=open(chip->publisher->input_node_name, O_RDWR);
	if(fd < 0)
	{
		LOGE("Error while opening uinput device: %s.\n", strerror(errno));
		return 1;
	}

	/* Configure all that we're gonna need to publish data. */
	if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(fd, UI_SET_RELBIT, REL_Z) < 0)
	{
		LOGE("Error while configuring uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	/* Write the uinput device to the node and ask to create the input. */
	if(write(fd, &uinput_dev, sizeof(uinput_dev)) < 0)
	{
		LOGE("Error while writing uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	if(ioctl(fd, UI_DEV_CREATE) < 0)
	{
		LOGE("Error while creating uinput device: %s.\n", strerror(errno));
		chip->publisher->deinit(chip);	
		return 1;
	}

	/* 
	 * Copy the file descriptor to the uinput node and set the publisher as 
	 * initialized 
	 */
	chip->publisher->fd=fd;
	chip->publisher->inited=1;

	/* Sleep a moment to wait for the compass input to be created. */
	usleep(100000);

	return 0;
}

int akm8975_publisher_deinit(struct akm_chip_sensors *chip)
{

	/* Ask to destroy the input node and close the file descriptor. */
	if(ioctl(chip->publisher->fd, UI_DEV_DESTROY) < 0)
	{
		LOGE("Error while destroying uinput device: %s.\n", strerror(errno));
	}

	if(close(chip->publisher->fd) < 0)
	{
		LOGE("Error while closing uinput device: %s.\n", strerror(errno));
		return 1;
	}

	chip->publisher->fd=-1;
	chip->publisher->inited=0;
	return 0;
}

int akm8975_set_delay(struct akm_sensor *sensor_info, uint64_t delay)
{
	int rc;

	return 0;
}

/* This is the structure for the akm8975 chip. */
struct akm_chip_sensors akm8975 = {
	.publisher=&akm8975_publisher,
	.sensors_count=2,
	.sensors={
		&akm8975_magnetic_field,
		&akm8975_orientation
	},
//	.data_get=akm8975_data_get,
	.init=akm8975_init,
	.inited=0,
	.fd=-1,
	.device_name="/dev/akm8975",
};

/* This is the structure for the akm8975 data publisher. */
struct akm_publisher akm8975_publisher = 
{
	.fd=-1,
	.input_device_name="compass",
	.input_node_name="/dev/uinput",
	.inited=0,
	.init=akm8975_publisher_init,
	.deinit=akm8975_publisher_deinit,
	.data_publish=akm8975_data_publish,
};

/* This is the magnetic field sensor structure. */
struct akm_sensor akm8975_magnetic_field = {
	.type=SENSOR_TYPE_MAGNETIC_FIELD,
	.enabled=0,
	.enable=default_enable,
	.disable=default_disable,
	.set_delay=akm8975_set_delay,
	.chip=&akm8975,
};

/* This is the orientation sensor structure. */
struct akm_sensor akm8975_orientation = {
	.type=SENSOR_TYPE_ORIENTATION,
	.enabled=0,
	.enable=default_enable,
	.disable=default_disable,
	.set_delay=akm8975_set_delay,
	.chip=&akm8975,
};
