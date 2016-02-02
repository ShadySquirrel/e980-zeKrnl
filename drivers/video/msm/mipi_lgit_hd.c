/*
 *  Copyright (C) 2011-2012, LG Eletronics,Inc. All rights reserved.
 *      LGIT LCD device driver
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/board_lge.h>
#include <linux/syscore_ops.h>

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lgit.h"

#include "mdp4.h"
static struct msm_panel_common_pdata *mipi_lgit_pdata;

static struct dsi_buf lgit_tx_buf;
static struct dsi_buf lgit_rx_buf;
static int __init mipi_lgit_lcd_init(void);

#define PM8921_GPIO_BASE		NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8921_GPIO_BASE)

#define PM8921_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8921_GPIO_OUTPUT(_gpio, _val, _strength) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

static int DSV_EN;

static int lgit_external_dsv_onoff(uint8_t on_off)
{
	int ret = 0;
	static int init_done = 0;

	if(!init_done){

		DSV_EN = PM8921_GPIO_PM_TO_SYS(22);

		if (gpio_is_valid(DSV_EN)) {
			ret = gpio_request_one(DSV_EN, GPIOF_DIR_OUT, "dsv_en");
			if(ret < 0) {
				pr_err("%s: failed to request DSV_EN qpio\n", __func__);
				goto err_gpio;
			}
		}
		init_done = 1;
	}

	gpio_set_value(DSV_EN, on_off);
	mdelay(20);
	goto out;

err_gpio:
	gpio_free(DSV_EN);
out:
	return ret;
}

//                                                                              
#define LGIT_IEF_SWITCH

#ifdef LGIT_IEF_SWITCH
struct msm_fb_data_type *local_mfd0 = NULL;
static int is_ief_on = 1;
#endif

#ifdef LGIT_IEF_SWITCH
int mipi_lgit_lcd_ief_off(void)
{
	if(local_mfd0->panel_power_on && is_ief_on) {
		printk("IEF_OFF Starts with Camera\n");
		mutex_lock(&local_mfd0->dma->ov_mutex);
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);//HS mode
		mipi_dsi_cmds_tx(&lgit_tx_buf, mipi_lgit_pdata->power_off_set_ief, mipi_lgit_pdata->power_off_set_ief_size);

		printk("%s, %d\n", __func__,is_ief_on);
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);//LP mode
		mutex_unlock(&local_mfd0->dma->ov_mutex);
		printk("IEF_OFF Ends with Camera\n");
	}
	is_ief_on = 0;

	return 0;
}

int mipi_lgit_lcd_ief_on(void)
{
	if(local_mfd0->panel_power_on && !is_ief_on) {
		printk("IEF_ON Starts with Camera\n");
		mutex_lock(&local_mfd0->dma->ov_mutex);
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);//HS mode
		mipi_dsi_cmds_tx(&lgit_tx_buf, mipi_lgit_pdata->power_on_set_ief, mipi_lgit_pdata->power_on_set_ief_size);

		printk("%s, %d\n", __func__,is_ief_on);
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000); //LP mode
		mutex_unlock(&local_mfd0->dma->ov_mutex);
		printk("IEF_ON Ends with Camera\n");
	}
        is_ief_on = 1;

	return 0;
}
#endif
//                                                                              

 int mipi_lgit_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	int cnt = 0;

	pr_info("%s:+ hd \n", __func__);

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

#ifdef LGIT_IEF_SWITCH
	if(local_mfd0 == NULL)
		local_mfd0 = mfd;
#endif

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_on_set_1,
			mipi_lgit_pdata->power_on_set_size_1);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (cnt < 0) {
		pr_err("%s: failed to transmit power_on_set_1 cmds\n", __func__);
		return cnt;
	}

	mipi_dsi_op_mode_config(DSI_VIDEO_MODE);
	mdp4_overlay_dsi_video_start();

	mdelay(10);


	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);//HS mode
	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
		mipi_lgit_pdata->power_on_set_2,
		mipi_lgit_pdata->power_on_set_size_2);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);//LP mode
	if (cnt < 0) {
		pr_err("%s: failed to transmit power_on_set_2 cmds\n", __func__);
		return cnt;
	}

	cnt = lgit_external_dsv_onoff(1);
	if (cnt < 0) {
		pr_err("%s: failed to turn on external dsv\n", __func__);
		return cnt;
	}

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_on_set_3,
			mipi_lgit_pdata->power_on_set_size_3);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (cnt < 0) {
		pr_err("%s: failed to transmit power_on_set_3 cmds\n", __func__);
		return cnt;
	}

	pr_info("%s:- hd \n", __func__);

	return cnt;
}

int mipi_lgit_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	int cnt = 0;

	pr_info("%s:+ hd \n", __func__);

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_off_set_1,
			mipi_lgit_pdata->power_off_set_size_1);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (cnt < 0) {
		pr_err("%s: failed to transmit power_off_set_1 cmds\n", __func__);
		return cnt;
	}

	cnt = lgit_external_dsv_onoff(0);
	if (cnt < 0) {
		pr_err("%s: failed to turn off external dsv\n", __func__);
		return cnt;
	}

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_off_set_2,
			mipi_lgit_pdata->power_off_set_size_2);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (cnt < 0) {
		pr_err("%s: failed to transmit power_off_set_2 cmds\n", __func__);
		return cnt;
	}

	pr_info("%s:- hd \n", __func__);

	return cnt;
}

static void mipi_lgit_lcd_shutdown(void)
{
	int ret = 0;

    if(!local_mfd0 || !local_mfd0->panel_power_on) {
        ;
    } else {

        /* Temporary work around to turn off BLU
           before panel power off */
        mipi_lgit_pdata->backlight_level(0, 0, 0);

        MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
        ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
                mipi_lgit_pdata->power_off_set_1,
                mipi_lgit_pdata->power_off_set_size_1);
        MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
        if (ret < 0) {
            pr_err("%s: failed to transmit power_off_set_1 cmds\n", __func__);
        }

        ret = lgit_external_dsv_onoff(0);
        if (ret < 0) {
            pr_err("%s: failed to turn off external dsv\n", __func__);
        }

        MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
        ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
                mipi_lgit_pdata->power_off_set_2,
                mipi_lgit_pdata->power_off_set_size_2);
        MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
        if (ret < 0) {
            pr_err("%s: failed to transmit power_off_set_2 cmds\n", __func__);
        }
    }

	pr_info("%s finished\n", __func__);
}

static void mipi_lgit_set_backlight_board(struct msm_fb_data_type *mfd)
{
	int level;

	level = (int)mfd->bl_level;
	mipi_lgit_pdata->backlight_level(level, 0, 0);
}

struct syscore_ops panel_syscore_ops = {
	.shutdown = mipi_lgit_lcd_shutdown,
};

static int mipi_lgit_lcd_probe(struct platform_device *pdev)
{
	if (pdev->id == 0) {
		mipi_lgit_pdata = pdev->dev.platform_data;
		return 0;
	}

	printk(KERN_INFO "%s: mipi lgit lcd probe start\n", __func__);

	msm_fb_add_device(pdev);

	register_syscore_ops(&panel_syscore_ops);

	return 0;
}

static struct platform_driver this_driver = {
	.probe = mipi_lgit_lcd_probe,
	.driver = {
		.name = "mipi_lgit",
	},
};

static struct msm_fb_panel_data lgit_panel_data = {
	.on = mipi_lgit_lcd_on,
	.off = mipi_lgit_lcd_off,
	.set_backlight = mipi_lgit_set_backlight_board,
};

static int ch_used[3];

int mipi_lgit_device_register(struct msm_panel_info *pinfo,
		u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_lgit", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	lgit_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &lgit_panel_data,
			sizeof(lgit_panel_data));
	if (ret) {
		printk(KERN_ERR "%s: platform_device_add_data failed!\n",
				__func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR "%s: platform_device_register failed!\n",
				__func__);
		goto err_device_put;
	}
	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init mipi_lgit_lcd_init(void)
{
	mipi_dsi_buf_alloc(&lgit_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&lgit_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_lgit_lcd_init);
