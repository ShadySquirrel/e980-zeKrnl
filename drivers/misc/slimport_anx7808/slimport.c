/*
 * Copyright(c) 2012-2013, LGE Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt)	"%s %s: " fmt, "anx7808", __func__

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_data/slimport_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slimport.h>
#include <linux/async.h>

#include "slimport_tx_drv.h"

struct i2c_client *anx7808_client;

struct anx7808_data {
	struct anx7808_platform_data    *pdata;
	struct delayed_work    work;
	struct workqueue_struct    *workqueue;
	struct mutex    lock;
	struct wake_lock slimport_lock;
};

#ifdef HDCP_EN
static bool hdcp_enable = 1;
#else
static bool hdcp_enable;
#endif

#ifdef SP_REGISTER_SET_TEST /* for cts phy test */
/* sysfs write interface */
static ssize_t ctrl_reg0_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[0] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[0] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg0_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[0]);
}

/* sysfs write interface */
static ssize_t ctrl_reg1_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[1] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[1] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg1_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[1]);
}

/* sysfs write interface */
static ssize_t ctrl_reg2_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[2] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[2] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg2_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[2]);
}

/* sysfs write interface */
static ssize_t ctrl_reg3_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[3] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[3] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg3_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[3]);
}

/* sysfs write interface */
static ssize_t ctrl_reg4_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[4] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[4] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg4_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[4]);
}

/* sysfs write interface */
static ssize_t ctrl_reg5_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[5] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[5] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg5_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[5]);
}

/* sysfs write interface */
static ssize_t ctrl_reg6_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[6] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[6] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg6_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[6]);
}

/* sysfs write interface */
static ssize_t ctrl_reg7_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[7] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[7] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg7_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[7]);
}

/* sysfs write interface */
static ssize_t ctrl_reg8_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[8] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[8] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg8_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[8]);
}

/* sysfs write interface */
static ssize_t ctrl_reg9_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[9] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[9] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg9_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[9]);
}

/* sysfs write interface */
static ssize_t ctrl_reg10_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[10] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[10] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg10_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[10]);
}

/* sysfs write interface */
static ssize_t ctrl_reg11_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[11] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[11] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg11_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[11]);
}

/* sysfs write interface */
static ssize_t ctrl_reg12_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[12] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[12] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg12_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[12]);
}

/* sysfs write interface */
static ssize_t ctrl_reg13_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[13] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[13] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg13_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[13]);
}

/* sysfs write interface */
static ssize_t ctrl_reg14_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[14] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[14] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg14_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[14]);
}

/* sysfs write interface */
static ssize_t ctrl_reg15_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[15] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[15] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg15_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[15]);
}

/* sysfs write interface */
static ssize_t ctrl_reg16_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[16] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[16]  = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg16_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[16]);
}

/* sysfs write interface */
static ssize_t ctrl_reg17_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[17] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[17] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg17_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[17]);
}

/* sysfs write interface */
static ssize_t ctrl_reg18_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[18] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[18] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg18_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[18]);
}

/* sysfs write interface */
static ssize_t ctrl_reg19_store(struct device *dev, struct device_attribute *attr,
							   const char *buf, size_t count)
{
    int ret;
    long val;
    ret = strict_strtol(buf, 10, &val);
    if (ret)
        return ret;
    val_SP_TX_LT_CTRL_REG[19] = val;
    val_SP_TX_LT_CTRL_REG_write_flag[19] = true;
    return count;
}

/* sysfs read interface */
static ssize_t ctrl_reg19_show(struct device *dev, struct device_attribute *attr,
							  char *buf)
{
    return sprintf(buf, "0x%x\n", val_SP_TX_LT_CTRL_REG[19]);
}

static struct device_attribute slimport_device_attrs[] = {
	__ATTR(ctrl_reg0,  S_IRUGO | S_IWUSR, ctrl_reg0_show,  ctrl_reg0_store),
	__ATTR(ctrl_reg1,  S_IRUGO | S_IWUSR, ctrl_reg1_show,  ctrl_reg1_store),
	__ATTR(ctrl_reg2,  S_IRUGO | S_IWUSR, ctrl_reg2_show,  ctrl_reg2_store),
	__ATTR(ctrl_reg3,  S_IRUGO | S_IWUSR, ctrl_reg3_show,  ctrl_reg3_store),
	__ATTR(ctrl_reg4,  S_IRUGO | S_IWUSR, ctrl_reg4_show,  ctrl_reg4_store),
	__ATTR(ctrl_reg5,  S_IRUGO | S_IWUSR, ctrl_reg5_show,  ctrl_reg5_store),
	__ATTR(ctrl_reg6,  S_IRUGO | S_IWUSR, ctrl_reg6_show,  ctrl_reg6_store),
	__ATTR(ctrl_reg7,  S_IRUGO | S_IWUSR, ctrl_reg7_show,  ctrl_reg7_store),
	__ATTR(ctrl_reg8,  S_IRUGO | S_IWUSR, ctrl_reg8_show,  ctrl_reg8_store),
	__ATTR(ctrl_reg9,  S_IRUGO | S_IWUSR, ctrl_reg9_show,  ctrl_reg9_store),
	__ATTR(ctrl_reg10, S_IRUGO | S_IWUSR, ctrl_reg10_show, ctrl_reg10_store),
	__ATTR(ctrl_reg11, S_IRUGO | S_IWUSR, ctrl_reg11_show, ctrl_reg11_store),
	__ATTR(ctrl_reg12, S_IRUGO | S_IWUSR, ctrl_reg12_show, ctrl_reg12_store),
	__ATTR(ctrl_reg13, S_IRUGO | S_IWUSR, ctrl_reg13_show, ctrl_reg13_store),
	__ATTR(ctrl_reg14, S_IRUGO | S_IWUSR, ctrl_reg14_show, ctrl_reg14_store),
	__ATTR(ctrl_reg15, S_IRUGO | S_IWUSR, ctrl_reg15_show, ctrl_reg15_store),
	__ATTR(ctrl_reg16, S_IRUGO | S_IWUSR, ctrl_reg16_show, ctrl_reg16_store),
	__ATTR(ctrl_reg17, S_IRUGO | S_IWUSR, ctrl_reg17_show, ctrl_reg17_store),
	__ATTR(ctrl_reg18, S_IRUGO | S_IWUSR, ctrl_reg18_show, ctrl_reg18_store),
	__ATTR(ctrl_reg19, S_IRUGO | S_IWUSR, ctrl_reg19_show, ctrl_reg19_store),
};

static int create_sysfs_interfaces(struct device *dev)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(slimport_device_attrs); i++)
        if (device_create_file(dev, &slimport_device_attrs[i]))
            goto error;
    return 0;

error:
    for ( ; i >= 0; i--)
        device_remove_file(dev, &slimport_device_attrs[i]);
    pr_err("Unable to create interface");
    return -EINVAL;
}
#endif

int sp_read_reg(uint8_t slave_addr, uint8_t offset, uint8_t *buf)
{
	int ret = 0;

	anx7808_client->addr = (slave_addr >> 1);
	ret = i2c_smbus_read_byte_data(anx7808_client, offset);
	if (ret < 0) {
		pr_err("failed to read i2c addr=%x\n",
				slave_addr);
		return ret;
	}
	*buf = (uint8_t) ret;

	return 0;
}

int sp_write_reg(uint8_t slave_addr, uint8_t offset, uint8_t value)
{
	int ret = 0;

	anx7808_client->addr = (slave_addr >> 1);
	ret = i2c_smbus_write_byte_data(anx7808_client, offset, value);
	if (ret < 0) {
		pr_err("failed to write i2c addr=%x\n",
				slave_addr);
	}
	return ret;
}

void sp_tx_hardware_poweron(void)
{
	struct anx7808_platform_data *pdata = anx7808_client->dev.platform_data;

	gpio_set_value(pdata->gpio_reset, 0);
	msleep(1);
	gpio_set_value(pdata->gpio_p_dwn, 0);
	msleep(2);
	pdata->dvdd_power(1);
	msleep(20);
	gpio_set_value(pdata->gpio_reset, 1);

	pr_info("anx7808 power on\n");
}

void sp_tx_hardware_powerdown(void)
{
	struct anx7808_platform_data *pdata = anx7808_client->dev.platform_data;

	gpio_set_value(pdata->gpio_reset, 0);
	msleep(1);
	pdata->dvdd_power(0);
	msleep(5);
	gpio_set_value(pdata->gpio_p_dwn, 1);
	msleep(1);

	pr_info("anx7808 power down\n");
}


static void sp_tx_power_down_and_init(void)
{
	sp_tx_vbus_powerdown();
	sp_tx_power_down(SP_TX_PWR_REG);
	sp_tx_power_down(SP_TX_PWR_TOTAL);
	sp_tx_hardware_powerdown();
	sp_tx_pd_mode = 1;
	sp_tx_link_config_done = 0;
	sp_tx_hw_lt_enable = 0;
	sp_tx_hw_lt_done = 0;
	sp_tx_rx_type = RX_NULL;
	sp_tx_rx_type_backup = RX_NULL;
	sp_tx_set_sys_state(STATE_CABLE_PLUG);
}

static void slimport_cable_plug_proc(struct anx7808_data *anx7808)
{
	struct anx7808_platform_data *pdata = anx7808->pdata;

	if (gpio_get_value_cansleep(pdata->gpio_cbl_det)) {
		/* Previously, if sp tx is turned on, turn it off to
		 * avoid the cable detection erorr.
		 */
		if ((!sp_tx_pd_mode)
			&& (sp_tx_rx_type != RX_VGA_9832)
			&& (sp_tx_rx_type != RX_VGA_GEN))
			sp_tx_power_down_and_init();

		/* debounce time for avoiding glitch */
		msleep(100);
		if (gpio_get_value_cansleep(pdata->gpio_cbl_det)) {
			if (sp_tx_pd_mode) {
				sp_tx_pd_mode = 0;
				sp_tx_hardware_poweron();
				sp_tx_power_on(SP_TX_PWR_REG);
				sp_tx_power_on(SP_TX_PWR_TOTAL);
				hdmi_rx_initialization();
				sp_tx_initialization();
				sp_tx_vbus_poweron();
				if (!sp_tx_get_cable_type(1)) {
					pr_err("AUX ERR\n");
					sp_tx_power_down_and_init();
					return;
				}
				sp_tx_rx_type_backup = sp_tx_rx_type;
			}

			switch (sp_tx_rx_type) {
			case RX_HDMI:
				if (sp_tx_get_hdmi_connection())
					sp_tx_set_sys_state(STATE_PARSE_EDID);
				break;
			case RX_DP:
				if (sp_tx_get_dp_connection())
					sp_tx_set_sys_state(STATE_PARSE_EDID);
				break;
			case RX_VGA_GEN:
				if (sp_tx_get_vga_connection())
					sp_tx_set_sys_state(STATE_PARSE_EDID);
				break;
			case RX_VGA_9832:
				if (sp_tx_get_vga_connection()) {
					sp_tx_send_message(MSG_CLEAR_IRQ);
					sp_tx_set_sys_state(STATE_PARSE_EDID);
				}
				break;
			case RX_NULL:
			default:
				break;
			}
		}
	} else if (sp_tx_pd_mode == 0) {
			sp_tx_power_down_and_init();
	}
}

static void slimport_edid_proc(void)
{
	sp_tx_aux_polling_enable(0);
	sp_tx_edid_read();

	if (bedid_break)
		pr_err("EDID corruption!\n");
	sp_tx_aux_polling_enable(1);
	hdmi_rx_set_hpd(1);
	hdmi_rx_set_termination(1);
	sp_tx_set_sys_state(STATE_LINK_TRAINING);
}

int slimport_read_edid_block(int block, uint8_t *edid_buf)
{
	if (block == 0) {
		memcpy(edid_buf, bedid_firstblock, sizeof(bedid_firstblock));
	} else if (block == 1) {
		memcpy(edid_buf, bedid_extblock, sizeof(bedid_extblock));
	} else {
		pr_err("%s: block number %d is invalid\n", __func__, block);
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL(slimport_read_edid_block);

unchar sp_get_link_bw(void)
{
	return slimport_link_bw;
}
EXPORT_SYMBOL(sp_get_link_bw);

void sp_set_link_bw(unchar link_bw)
{
	slimport_link_bw = link_bw;
}
EXPORT_SYMBOL(sp_set_link_bw);

enum RX_CBL_TYPE sp_get_ds_cable_type(void)
{
	return sp_tx_rx_type;
}
EXPORT_SYMBOL(sp_get_ds_cable_type);

bool slimport_is_connected(void)
{
	struct anx7808_platform_data *pdata = NULL;
	bool result = false;

	if (!anx7808_client)
		return false;

	pdata = anx7808_client->dev.platform_data;
	if (!pdata)
		return false;

	spin_lock(&pdata->lock);

	if (gpio_get_value_cansleep(pdata->gpio_cbl_det)) {
		mdelay(10);
		if (gpio_get_value_cansleep(pdata->gpio_cbl_det)) {
			pr_info("Slimport Dongle is detected\n");
			result = true;
		}
	}

	spin_unlock(&pdata->lock);

	return result;
}
EXPORT_SYMBOL(slimport_is_connected);

bool is_slimport_vga(void)
{
	return (sp_tx_rx_type == RX_VGA_9832) || (sp_tx_rx_type == RX_VGA_GEN);
}
EXPORT_SYMBOL(is_slimport_vga);

bool is_slimport_dp(void)
{
	return (sp_tx_rx_type == RX_DP);
}
EXPORT_SYMBOL(is_slimport_dp);

static void slimport_config_output(void)
{
	sp_tx_clean_hdcp();
	sp_tx_set_colorspace();
	sp_tx_avi_setup();
	sp_tx_config_packets(AVI_PACKETS);
	sp_tx_enable_video_input(1);
	sp_tx_set_sys_state(STATE_HDCP_AUTH);
}

static void slimport_playback_proc(void)
{
	return;
}

static void slimport_main_proc(struct anx7808_data *anx7808)
{
	mutex_lock(&anx7808->lock);

	if (!sp_tx_pd_mode) {
		sp_tx_int_irq_handler();
		hdmi_rx_int_irq_handler();
	}

	if (sp_tx_system_state == STATE_CABLE_PLUG)
		slimport_cable_plug_proc(anx7808);

	if (sp_tx_system_state == STATE_PARSE_EDID)
		slimport_edid_proc();

	if (sp_tx_system_state == STATE_CONFIG_HDMI)
		sp_tx_config_hdmi_input();

	if (sp_tx_system_state == STATE_LINK_TRAINING) {
		if (!sp_tx_lt_pre_config())
			sp_tx_hw_link_training();
	}

	if (sp_tx_system_state == STATE_CONFIG_OUTPUT)
		slimport_config_output();

	if (sp_tx_system_state == STATE_HDCP_AUTH) {
		if ((hdcp_enable)
			&& (sp_tx_rx_type != RX_VGA_9832)
			&& (sp_tx_rx_type != RX_VGA_GEN)) {
			sp_tx_hdcp_process();
		} else {
			sp_tx_power_down(SP_TX_PWR_HDCP);
			sp_tx_video_mute(0);
			hdmi_rx_show_video_info();
			sp_tx_show_infomation();
			sp_tx_set_sys_state(STATE_PLAY_BACK);
		}
	}

	if (sp_tx_system_state == STATE_PLAY_BACK)
		slimport_playback_proc();

	mutex_unlock(&anx7808->lock);
}

static uint8_t anx7808_chip_detect(void)
{
	return sp_tx_chip_located();
}

static void anx7808_chip_initial(void)
{
#ifdef EYE_TEST
	sp_tx_eye_diagram_test();
#else
	sp_tx_variable_init();
	sp_tx_vbus_powerdown();
	sp_tx_hardware_powerdown();
	sp_tx_set_sys_state(STATE_CABLE_PLUG);
#endif
}

static void anx7808_free_gpio(struct anx7808_data *anx7808)
{
	gpio_free(anx7808->pdata->gpio_cbl_det);
	gpio_free(anx7808->pdata->gpio_int);
	gpio_free(anx7808->pdata->gpio_reset);
	gpio_free(anx7808->pdata->gpio_p_dwn);
}

static int anx7808_init_gpio(struct anx7808_data *anx7808)
{
	int ret = 0;

	pr_info("anx7808 init gpio\n");

	ret = gpio_request_one(anx7808->pdata->gpio_p_dwn,
				GPIOF_OUT_INIT_HIGH, "anx_p_dwn_ctl");
	if (ret) {
		pr_err("failed to request gpio %d \n",
				anx7808->pdata->gpio_p_dwn);
		goto out;
	}

	ret = gpio_request_one(anx7808->pdata->gpio_reset,
				GPIOF_OUT_INIT_LOW, "anx7808_reset_n");
	if (ret) {
		pr_err("failed to request gpio %d \n",
				anx7808->pdata->gpio_reset);
		goto err0;
	}

	ret = gpio_request_one(anx7808->pdata->gpio_int,
				GPIOF_IN, "anx7808_int_n");

	if (ret) {
		pr_err("failed to request gpio %d \n",
				anx7808->pdata->gpio_int);
		goto err1;
	}

	ret = gpio_request_one(anx7808->pdata->gpio_cbl_det,
				GPIOF_IN, "anx7808_cbl_det");
	if (ret) {
		pr_err("failed to request gpio %d \n",
				anx7808->pdata->gpio_cbl_det);
		goto err2;
	}

	gpio_set_value(anx7808->pdata->gpio_reset, 0);
	gpio_set_value(anx7808->pdata->gpio_p_dwn, 1);

	goto out;

err2:
	gpio_free(anx7808->pdata->gpio_int);
err1:
	gpio_free(anx7808->pdata->gpio_reset);
err0:
	gpio_free(anx7808->pdata->gpio_p_dwn);
out:
	return ret;
}

static int anx7808_system_init(void)
{
	int ret = 0;

	ret = anx7808_chip_detect();
	if (ret == 0) {
		pr_err("failed to detect anx7808\n");
		return -ENODEV;
	}

	anx7808_chip_initial();
	return 0;
}

static irqreturn_t anx7808_cbl_det_isr(int irq, void *data)
{
	struct anx7808_data *anx7808 = data;

	if (gpio_get_value(anx7808->pdata->gpio_cbl_det)) {
		wake_lock(&anx7808->slimport_lock);
		hdmi_common_set_hpd_on(1);
		pr_info("detect cable insertion\n");
		queue_delayed_work(anx7808->workqueue, &anx7808->work, 300);
	} else {
		pr_info("detect cable removal\n");
		hdmi_common_set_hpd_on(0);
		cancel_delayed_work_sync(&anx7808->work);
		wake_unlock(&anx7808->slimport_lock);
		wake_lock_timeout(&anx7808->slimport_lock, 2*HZ);
	}
	return IRQ_HANDLED;
}

static void anx7808_work_func(struct work_struct *work)
{
#ifndef EYE_TEST
	struct anx7808_data *td = container_of(work, struct anx7808_data,
								work.work);

	slimport_main_proc(td);
	queue_delayed_work(td->workqueue, &td->work,
			msecs_to_jiffies(300));
#endif
}

static int anx7808_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{

	struct anx7808_data *anx7808;
	int ret = 0;

#ifdef SP_REGISTER_SET_TEST
	val_SP_TX_LT_CTRL_REG[0] = 0x19;
	val_SP_TX_LT_CTRL_REG[1] = 0x26;
	val_SP_TX_LT_CTRL_REG[2] = 0x3A;
	val_SP_TX_LT_CTRL_REG[3] = 0x3F;
	val_SP_TX_LT_CTRL_REG[4] = 0x1b;
	val_SP_TX_LT_CTRL_REG[5] = 0x28;
	val_SP_TX_LT_CTRL_REG[6] = 0x3c;
	val_SP_TX_LT_CTRL_REG[7] = 0x22;
	val_SP_TX_LT_CTRL_REG[8] = 0x2F;
	val_SP_TX_LT_CTRL_REG[9] = 0x23;

	val_SP_TX_LT_CTRL_REG[12] = 0x0A;

	val_SP_TX_LT_CTRL_REG[14] = 0x09;
	val_SP_TX_LT_CTRL_REG[15] = 0x10;
	val_SP_TX_LT_CTRL_REG[16] = 0x1A;
	val_SP_TX_LT_CTRL_REG[17] = 0x16;
	val_SP_TX_LT_CTRL_REG[18] = 0x1F;
	val_SP_TX_LT_CTRL_REG[19] = 0x1F;
#endif
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_I2C_BLOCK)) {
		pr_err("i2c bus does not support anx7808\n");
		ret = -ENODEV;
		goto exit;
	}

	anx7808 = kzalloc(sizeof(struct anx7808_data), GFP_KERNEL);
	if (!anx7808) {
		pr_err("failed to allocate driver data\n");
		ret = -ENOMEM;
		goto exit;
	}

	anx7808->pdata = client->dev.platform_data;
	i2c_set_clientdata(client, anx7808);
	anx7808_client = client;



	if (!anx7808->pdata) {
		ret = -EINVAL;
		goto err0;
	}
	mutex_init(&anx7808->lock);

	ret = anx7808_init_gpio(anx7808);
	if (ret) {
		pr_err("failed to initialize gpio\n");
		goto err0;
	}

	INIT_DELAYED_WORK(&anx7808->work, anx7808_work_func);

	anx7808->workqueue = create_singlethread_workqueue("anx7808_work");
	if (!anx7808->workqueue) {
		pr_err("failed to create work queue\n");
		ret = -ENOMEM;
		goto err1;
	}

	anx7808->pdata->avdd_power(1);
	anx7808->pdata->dvdd_power(0);

	ret = anx7808_system_init();
	if (ret) {
		pr_err("failed to initialize anx7808\n");
		goto err2;
	}



	client->irq = gpio_to_irq(anx7808->pdata->gpio_cbl_det);
	if (client->irq < 0) {
		pr_err("failed to get gpio irq\n");
		goto err3;
	}

	wake_lock_init(&anx7808->slimport_lock, WAKE_LOCK_SUSPEND,
				"slimport_wake_lock");

	ret = request_threaded_irq(client->irq, NULL, anx7808_cbl_det_isr,
							   IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
							   "anx7808", anx7808);
	if (ret  < 0) {
		pr_err("failed to request irq \n");
		goto err3;
	}

#ifdef SP_REGISTER_SET_TEST
	ret = create_sysfs_interfaces(&client->dev);

	if (ret < 0) {
		pr_err("failed to request irq \n");
		goto err3;
	}
#endif

	ret = enable_irq_wake(client->irq);
	if (ret  < 0) {
		pr_err("interrupt wake enable fail\n");
		goto err4;
	}
	goto exit;

err4:
	free_irq(client->irq, anx7808);
err3:
	wake_lock_destroy(&anx7808->slimport_lock);
err2:
	destroy_workqueue(anx7808->workqueue);
err1:
	anx7808_free_gpio(anx7808);
err0:
	anx7808_client = NULL;
	kfree(anx7808);
exit:
	return ret;
}



static int anx7808_i2c_remove(struct i2c_client *client)
{
	struct anx7808_data *anx7808 = i2c_get_clientdata(client);

	free_irq(client->irq, anx7808);
	wake_lock_destroy(&anx7808->slimport_lock);
	destroy_workqueue(anx7808->workqueue);
	anx7808_free_gpio(anx7808);
	anx7808_client = NULL;
	kfree(anx7808);
	return 0;
}

static const struct i2c_device_id anx7808_id[] = {
	{ "anx7808", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, anx7808_id);

static struct i2c_driver anx7808_driver = {
	.driver  = {
		.name  = "anx7808",
		.owner  = THIS_MODULE,
	},
	.probe  = anx7808_i2c_probe,
	.remove  = anx7808_i2c_remove,
	.id_table  = anx7808_id,
};

static void __init anx7808_init_async(void *data, async_cookie_t cookie)
{
	int ret = 0;

	ret = i2c_add_driver(&anx7808_driver);
	if (ret < 0)
		pr_err("failed to register anx7808 i2c drivern");
}

static int __init anx7808_init(void)
{
	async_schedule(anx7808_init_async, NULL);
	return 0;
}

static void __exit anx7808_exit(void)
{
	i2c_del_driver(&anx7808_driver);
}

module_init(anx7808_init);
module_exit(anx7808_exit);

MODULE_DESCRIPTION("Slimport  transmitter ANX7808 driver");
MODULE_AUTHOR("ChoongRyeol Lee <choongryeol.lee@lge.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.4");
