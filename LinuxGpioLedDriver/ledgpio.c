#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include "ledgpio.h"

#define DEVICE_CLASS_NAME "gpioled"
#define GPIO_NUM 26

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Toumilovich");
MODULE_DESCRIPTION("Test driver Raspberry Pi 4");
MODULE_VERSION("0.1");

typedef enum
{
    NOT_INITIALIZED = 0,
    INITIALIZED
} driver_state_t;

static int            device_major_number;
static struct class   *driver_class  = NULL;
static struct device  *device_driver = NULL;
static driver_state_t driver_state = NOT_INITIALIZED;
static int            leds_number = 0;
static struct gpio    *leds;
static unsigned       led_state = 0;

static DEFINE_MUTEX(device_mutex);

// Character driver handlers
static int     dev_open(struct inode*, struct file*);
static int     dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

/*
 * Character device handlers
 */
static struct file_operations file_ops =
{
    .open    = dev_open,
    .read    = dev_read,
    .write   = dev_write,
    .release = dev_release
};

/*
 * @brief drv_init Driver initialization function
 */
static int __init drv_init(void)
{
    int ret = 0;

    printk(KERN_INFO DEVICE_NAME": starting driver\n");

    // Allocate device major number
    device_major_number = register_chrdev(0, DEVICE_NAME, &file_ops);

    if (device_major_number < 0)
    {
        printk(KERN_ERR DEVICE_NAME": failed to register driver\n");

        return device_major_number;
    }

    printk(KERN_INFO DEVICE_NAME": registered - major number %d\n", device_major_number);

    // Register device class
    driver_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);

    if (IS_ERR(driver_class))
    {
        unregister_chrdev(device_major_number, DEVICE_NAME);
        printk(KERN_ERR DEVICE_NAME": failed to register device class\n");

        return PTR_ERR(driver_class);
    }

    printk(KERN_INFO DEVICE_NAME": device class registered successfully\n");

    // Register device driver
    device_driver = device_create(driver_class, NULL, MKDEV(device_major_number, 0), NULL, DEVICE_NAME);

    if (IS_ERR(device_driver))
    {
        class_destroy(driver_class);
        unregister_chrdev(device_major_number, DEVICE_NAME);
        printk(KERN_ERR DEVICE_NAME": failed to create device\n");

        return PTR_ERR(device_driver);
    }

    mutex_init(&device_mutex);

    printk(KERN_INFO DEVICE_NAME": driver started\n");

    return ret;
}

/*
 * @brief drv_exit Driver close function
 */
static void __exit drv_exit(void)
{
    printk(KERN_INFO DEVICE_NAME": unloading driver\n");
    device_destroy(driver_class, MKDEV(device_major_number, 0));
    class_unregister(driver_class);
    class_destroy(driver_class);
    unregister_chrdev(device_major_number, DEVICE_NAME);
    mutex_destroy(&device_mutex);
}

/*
 * @brief dev_open Device open callback
 * @param i [IN] - File inode object
 * @param f [IN] - File object
 */
static int dev_open(struct inode *i, struct file *f)
{
    (void)i;
    (void)f;

    if (!mutex_trylock(&device_mutex))
    {
        printk(KERN_ERR DEVICE_NAME": device already opened\n");

        return -EBUSY;
    }

    printk(KERN_INFO DEVICE_NAME": device opened\n");

    return 0;
}

/*
 * @brief dev_release Device close callback
 * @param i [IN] - File inode object
 * @param f [IN] - File object
 */
static int dev_release(struct inode *i, struct file *f)
{
    (void)i;
    (void)f;

    printk(KERN_INFO DEVICE_NAME": device closed\n");
    mutex_unlock(&device_mutex);

    return 0;
}

/*
 * @brief led_gpio_init Initialize LEDs
 * @param cmd [IN] - command and GPIO port map
 */
static int led_gpio_init(const led_cmd_t *cmd)
{
    int ret, i, m;
    int l[GPIO_NUM];

    if (driver_state == INITIALIZED)
    {
        printk(KERN_ERR DEVICE_NAME": driver already initialized\n");

        return 1;
    }

    printk(KERN_INFO DEVICE_NAME": initializing...\n");

    leds_number = 0;

    for(i = 0; i < GPIO_NUM; i++)
    {
        m = 1 << i;

        if ((cmd->leds & m) != 0)
        {
            l[leds_number] = i + GPIO_MIN_PORT;
            leds_number++;
        }
    }

    if (leds_number)
        leds = kmalloc(leds_number * sizeof(struct gpio), GFP_KERNEL);

    for(i = 0; i < leds_number; i++)
    {
        leds[i].gpio = l[i];
        leds[i].flags = GPIOF_OUT_INIT_LOW;
        leds[i].label = "led";
        printk(KERN_INFO DEVICE_NAME": port %d\n", leds[i].gpio);
    }

    if (0 != (ret = gpio_request_array(leds, leds_number)))
    {
        printk(KERN_ERR DEVICE_NAME": unable to request GPIO ports: %d\n", ret);
    }

    for(i = 0; i < leds_number; i++)
    {
        gpio_direction_output(leds[i].gpio, GPIOF_OUT_INIT_LOW);
    }

    driver_state = INITIALIZED;
    printk(KERN_INFO DEVICE_NAME": initialized\n");

    return ret;
}

/*
 * @brief led_gpio_shutdown Uninitialize LEDs
 * @param cmd [IN] - command and GPIO port map
 */
static int led_gpio_shutdown(void)
{
    int i;

    if (driver_state == NOT_INITIALIZED)
    {
        printk(KERN_ERR DEVICE_NAME": driver not initialized\n");

        return 1;
    }

    printk(KERN_INFO DEVICE_NAME": uninitializing...\n");

    for(i = 0; i < leds_number; i++)
    {
        printk(KERN_INFO DEVICE_NAME": port %d\n", leds[i].gpio);
        gpio_set_value(leds[i].gpio, GPIOF_OUT_INIT_LOW);
    }

    gpio_free_array(leds, leds_number);

    if (leds)
        kfree(leds);

    driver_state = NOT_INITIALIZED;

    printk(KERN_INFO DEVICE_NAME": uninitialized\n");

    return 0;
}

/*
 * @brief led_gpio_set Set LEDs state
 * @param cmd [IN] - command and GPIO port map
 */
static int led_gpio_set(const led_cmd_t *cmd)
{
    int i, state;

    if (driver_state == NOT_INITIALIZED)
    {
        printk(KERN_ERR DEVICE_NAME": driver not initialized\n");

        return 1;
    }

    for(i = 0; i < leds_number; i++)
    {
        state = (cmd->leds & (1 << (leds[i].gpio - GPIO_MIN_PORT))) ? GPIOF_OUT_INIT_HIGH : GPIOF_OUT_INIT_LOW;
        gpio_set_value(leds[i].gpio, state);
    }

    led_state = cmd->leds;

    return 0;
}

/*
 * @brief led_gpio_get Get current LEDs state
 * @param cmd [IN] - command and GPIO port map
 */
static int led_gpio_get(led_cmd_t *cmd)
{
    if (driver_state == NOT_INITIALIZED)
    {
        printk(KERN_ERR DEVICE_NAME": driver not initialized\n");

        return 1;
    }

    cmd->leds = led_state;

    return 0;
}

/*
 * @brief dev_read Device read callback
 * @param f      [IN]  - File object
 * @param buf    [OUT] - Data buffer
 * @param len    [IN]  - Buffer size
 * @param offset [IN]  - Buffer offset
 */
static ssize_t dev_read(struct file *f, char *buf, size_t len, loff_t *offset)
{
    led_cmd_t *cmd = (led_cmd_t*)buf;
    ssize_t   ret;
    (void) offset;

    if (len != sizeof(led_cmd_t))
    {
        printk(KERN_ERR DEVICE_NAME": bad command size\n");

        return -EFAULT;
    }

    ret = led_gpio_get(cmd) ? -EFAULT : 0;

    if (ret == 0 && copy_to_user(buf, cmd, sizeof(led_cmd_t)))
    {
        printk(KERN_INFO DEVICE_NAME": read buffer is too small\n");

        ret = -EFAULT;
    }

    ret = len;

    return ret;
}

/*
 * @brief dev_write Device write callback
 * @param f      [IN] - File object
 * @param buf    [IN] - Data buffer
 * @param len    [IN] - Buffer size
 * @param offset [IN] - Buffer offset
 */
static ssize_t dev_write(struct file *f, const char *buf, size_t len, loff_t *offset)
{
    led_cmd_t *cmd;
    ssize_t   ret;
    (void) offset;

    if (len != sizeof(led_cmd_t))
    {
        printk(KERN_ERR DEVICE_NAME": bad command size\n");

        return -EFAULT;
    }

    cmd = (led_cmd_t*)buf;
    ret = len;

    switch(cmd->operation)
    {
    case LED_OP_START:
        ret = led_gpio_init(cmd) ? -EFAULT : 0;
        break;
    case LED_OP_STOP:
        ret = led_gpio_shutdown() ? -EFAULT : 0;
        break;
    case LED_OP_SET:
        ret = led_gpio_set(cmd) ? -EFAULT : 0;
        break;
    default:
        printk(KERN_ERR DEVICE_NAME": bad command\n");
        return -EFAULT;
    }

    return ret;
}

module_init(drv_init);
module_exit(drv_exit);
