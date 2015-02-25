#include <linux/ioctl.h>
#include <linux/types.h>

#define GPIO_IOCTL_BASE 'G'

#define GPIO_SET_VALUE  _IOW(GPIO_IOCTL_BASE, 0, GPIO_USER_DATA)
#define GPIO_SET_DIRECTION  _IOW(GPIO_IOCTL_BASE, 1, GPIO_USER_DATA)
#define GPIO_GET_VALUE  _IOR(GPIO_IOCTL_BASE, 2, GPIO_USER_DATA)

typedef struct
{
	int gpio_num;
	int gpio_direction;
	int gpio_value;
} GPIO_USER_DATA;
