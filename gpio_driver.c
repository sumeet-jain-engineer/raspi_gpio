#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <mach/platform.h>

#include "gpio.h"

#define BCM_GPIO_BASE	(0x20200000)
#define GPIOFSEL(x)	(0x00+(x)*4)
#define GPIOSET(x)	(0x1c+(x)*4)
#define GPIOCLR(x)	(0x28+(x)*4)
#define GPIOGET(x)	(0x34+(x)*4)

#define GPIO_DRIVER "bcm-gpio"
#define SUCCESS 1

void __iomem *gpio_base = NULL;
struct platform_device *device;

static DEFINE_SPINLOCK(lock);

static int gpio_get_value(GPIO_USER_DATA *GPIO_user_data)
{
	unsigned gpio_bank = GPIO_user_data->gpio_num / 32;
	unsigned gpio_field_offset = (GPIO_user_data->gpio_num - 32 * gpio_bank);
	unsigned gpio_read_value;

	printk(KERN_INFO " %s called...\n", __func__);

	gpio_read_value = readl(gpio_base + GPIOGET(gpio_bank));
	return 0x1 & (gpio_read_value >> gpio_field_offset);
}

static void gpio_set_direction(GPIO_USER_DATA *GPIO_user_data)
{
	unsigned gpio_reg_val, gpio_bank, gpio_field;
	unsigned long flags;

	printk(KERN_INFO " %s called...\n", __func__);
	
	gpio_bank = GPIO_user_data->gpio_num / 10;
	gpio_field = (GPIO_user_data->gpio_num - 10 * gpio_bank) * 3;

	if(GPIO_user_data->gpio_direction)
		printk(KERN_INFO "Setting direction OUT for GPIO %u\n", GPIO_user_data->gpio_num);
	else
		printk(KERN_INFO "Setting direction IN for GPIO %u\n", GPIO_user_data->gpio_num);
	
	/* Set direction for GPIO */
	spin_lock_irqsave(&lock, flags);

	gpio_reg_val = readl(gpio_base + GPIOFSEL(gpio_bank));
	gpio_reg_val &= ~(7 << gpio_field);
	gpio_reg_val |= GPIO_user_data->gpio_direction << gpio_field;
	writel(gpio_reg_val, gpio_base + GPIOFSEL(gpio_bank));

	spin_unlock_irqrestore(&lock, flags);
	
	gpio_reg_val = readl(gpio_base + GPIOFSEL(gpio_bank));
}

static void gpio_set_value(GPIO_USER_DATA *GPIO_user_data)
{
	unsigned gpio_bank = GPIO_user_data->gpio_num / 32;
	unsigned gpio_field = GPIO_user_data->gpio_num;

	printk(KERN_INFO " %s called...\n", __func__);

	if (GPIO_user_data->gpio_value)
		writel(1 << gpio_field, gpio_base + GPIOSET(gpio_bank));
	else
		writel(1 << gpio_field, gpio_base+ GPIOCLR(gpio_bank));
}

int gpio_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO " %s called...\n", __func__);
	return 0;
}

static long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO " %s called...\n", __func__);

	GPIO_USER_DATA GPIO_user_data;

	if (copy_from_user(&GPIO_user_data, (GPIO_USER_DATA *) arg, sizeof(GPIO_user_data)))
		return -EFAULT;

	switch (cmd) {
		case GPIO_SET_DIRECTION:
			gpio_set_direction(&GPIO_user_data);
			break;
		case GPIO_SET_VALUE:
			gpio_set_value(&GPIO_user_data);
			break;
		case GPIO_GET_VALUE:
			GPIO_user_data.gpio_value = gpio_get_value(&GPIO_user_data);
			copy_to_user((GPIO_USER_DATA *) arg, &GPIO_user_data, sizeof(GPIO_user_data));
			break;
	};
	return SUCCESS;
}

static int gpio_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "GPIO Close function called...\n");
        return SUCCESS;
}

static int gpio_probe(struct platform_device *pdev)
{
	printk("BCM GPIO Probe called...\n");
	
	gpio_base = __io_address(BCM_GPIO_BASE);
	
	return 0;
}

static const struct file_operations gpio_fops =
{
	.owner		= THIS_MODULE,
	.open		= gpio_open,
	.release	= gpio_release,
	.unlocked_ioctl	= gpio_ioctl,
};

static struct platform_driver bcm_gpio_driver = 
{
	.driver.name	= GPIO_DRIVER,
	.driver.owner	= THIS_MODULE,
	.probe		= gpio_probe,
};


static int __init bcm_gpio_init(void)
{
	int major_num;

	major_num = register_chrdev(0, "bcm_gpio", &gpio_fops);
	if (major_num < 0) 
	{
		printk(KERN_ERR "In %s, Failed to register character device\n", __func__);
		return major_num;
	}

	printk(KERN_INFO "Major number allocated to BCM GPIO %u\n", major_num);
	
	printk(KERN_INFO "Registering GPIO Platform Device\n");
	device = platform_device_register_simple(GPIO_DRIVER, -1, NULL, 0);

	printk(KERN_INFO "Registering GPIO Platform Driver\n");
	return platform_driver_register(&bcm_gpio_driver);
}

static void __exit bcm_gpio_exit(void)
{
	printk(KERN_INFO "Deregistering GPIO Platform Device\n");
	platform_device_unregister(device);

	printk(KERN_INFO "Deregistering GPIO Platform Driver\n");
	platform_driver_unregister(&bcm_gpio_driver);
}

module_init(bcm_gpio_init);
module_exit(bcm_gpio_exit);

MODULE_AUTHOR("Sumeet Jain");
MODULE_DESCRIPTION("BCM2708 GPIO Driver");
MODULE_LICENSE("GPL");
