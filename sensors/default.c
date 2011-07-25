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
#include <sys/types.h>
#include <dirent.h>
#include <linux/input.h>

#include "akm.h"
#include "sensors/akm_sensors.h"

#define SENSOR_NAME	"default"

int default_enable(struct akm_sensor_info *sensor_info)
{
	int rc;

	if(sensor_info->enabled)
	{
		LOGV("%s already enabled.\n", SENSOR_NAME);
		return 0;
	}

	rc=sensor_info->chip->init(sensor_info->chip);

	sensor_info->enabled=1;

	return rc;
}

int default_disable(struct akm_sensor_info *sensor_info)
{
	int rc;

	if(!sensor_info->enabled)
	{
		LOGV("%s already disabled.\n", SENSOR_NAME);
		return 0;
	}

	sensor_info->enabled=0;

	return rc;
}

/* to rewrite please */
int default_open_input_by_name(const char *inputName)
{
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' ||
                        (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);

    return fd;
}


