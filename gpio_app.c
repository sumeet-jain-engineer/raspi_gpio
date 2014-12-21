#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "gpio.h"

int ERR=0;

void usage(void)
{
	printf("\n************************** Usage *******************************\n");
	printf(" -n	: GPIO Number\n -d	: GPIO Direction\n -v	: GPIO Value\n -s/g	: Set/Get GPIO Information\n\n");
	printf("INFO	: For GET provide only GPIO number and -g as the argument\n");
	printf("WARNING	: Please use a valid GPIO\n\n");
	ERR = -1;
}

/* Set direction IN or OUT for GPIO */
int gpio_set_direction(int fd, GPIO_USER_DATA *gpio_data)
{
	int ret;

	printf("%s\n", __func__);

	ret = ioctl(fd, GPIO_SET_DIRECTION, gpio_data);
        if (ret < 0)
                printf("GPIO_SET_VALUE ioctl failed, %s\n", __func__);
}

/* Set value for GPIO */
int gpio_set_value(int fd, GPIO_USER_DATA *gpio_data)
{
	int ret;

	printf("%s\n", __func__);

	ret = ioctl(fd, GPIO_SET_VALUE, gpio_data);
	if (ret < 0)
                printf("GPIO_SET_VALUE ioctl failed, %s\n", __func__);
}

/* Get GPIO value */
int gpio_get_value(int fd, GPIO_USER_DATA *gpio_data)
{
	int ret;

        ret = ioctl(fd, GPIO_GET_VALUE, gpio_data);
        if (ret < 0)
                printf("GPIO_GET_VALUE ioctl failed, %s\n", __func__);
}

main(int argc, char *argv[])
{
	int fd, c;
	int SET=0, GET=0;

	GPIO_USER_DATA gpio_data;

	/* memset the structures */
	memset(&gpio_data, 0, sizeof(gpio_data));

	/* Get the arguments from the command line and parse them */
	for (;;) {
		c = getopt(argc, argv, "hn:d:v:s:g:");
		if (c <= 0)
			break;
		switch (c) {
			case 'h':
				ERR = -1;
				break;
			case 'n':
				gpio_data.gpio_num = atoi (optarg);
				if(gpio_data.gpio_num < 0 || gpio_data.gpio_num > 25)
				{
					printf("Invalid GPIO Number\n");
					ERR = -1;
				}
				break;
			case 'd':  
				gpio_data.gpio_direction = atoi (optarg);
				if(gpio_data.gpio_direction < 0 && gpio_data.gpio_direction > 1)
				{
					printf("Invalid GPIO Direction\n");
					ERR = -1;
				}
				break;
			case 'v':
				gpio_data.gpio_value = atoi (optarg);
				if(gpio_data.gpio_value < 0 || gpio_data.gpio_value > 1)
				{
					printf("Invalid GPIO Value\n");
					ERR = -1;
				}
				break;
			case 's':
				SET = 1;
				break;
			case 'g':
				GET = 1;
				break;
			default :
				ERR = -1;
		}
	}

	if ( ((argc != 9) && SET) || !(SET || GET) || (SET && GET) || (-1 == ERR))
	{
		ERR = -1;
		usage();
		exit(1);
	}

	printf("GPIO number	: %u\n", gpio_data.gpio_num);
	printf("GPIO direction	: %u\n", gpio_data.gpio_direction);
	printf("GPIO value	: %u\n", gpio_data.gpio_value);

	/* Open the interface we created using "mknod" command */
	fd = open("/dev/bcm-gpio", O_RDWR);
	if(fd < 0)
	{
		printf("Cannot open file\n");
		exit(3);
	}

	if(GET)
	{
		gpio_get_value(fd, &gpio_data);
		printf("The output value of GPIO %u = %u\n", gpio_data.gpio_num, gpio_data.gpio_value);
	}
	if(SET)
	{
		gpio_set_direction(fd, &gpio_data);
		gpio_set_value(fd, &gpio_data);
		printf("The GPIO %u is set with value %u (Direction %u)\n", gpio_data.gpio_num, gpio_data.gpio_value, gpio_data.gpio_direction);
	}

	/* Close the interface after use */
	close(fd);
}
