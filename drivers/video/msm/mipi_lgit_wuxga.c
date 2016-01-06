/* Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
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
#include <mach/board_lge.h>

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lgit_wuxga.h"

#include "mdp4.h"

static struct msm_panel_common_pdata *mipi_lgit_pdata;

static struct dsi_buf lgit_tx_buf;
static struct dsi_buf lgit_rx_buf;
static struct dsi_buf lgit_camera_tx_buf;
static struct dsi_buf lgit_shutdown_tx_buf;

static int __init mipi_lgit_lcd_init(void);
static bool lgit_lcd_cabc_state(void);

#if defined(CONFIG_LGIT_COLOR_ENGINE_SWITCH)
static int is_color_engine_on = 1;
struct msm_fb_data_type *local_mfd0 = NULL;
int mipi_lgit_lcd_color_engine_off(void)
{
	if(local_mfd0!=NULL && local_mfd0->panel_power_on && is_color_engine_on) {
		printk("Color Engine_OFF Starts with Camera\n");
		mutex_lock(&local_mfd0->dma->ov_mutex);

		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);//HS mode
		mipi_dsi_cmds_tx(&lgit_camera_tx_buf,
		mipi_lgit_pdata->color_engine_off,
		mipi_lgit_pdata->color_engine_off_size);
		printk("%s, %d\n", __func__,is_color_engine_on);
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);//LP mode

		mutex_unlock(&local_mfd0->dma->ov_mutex);
		printk("Color Engine_OFF Ends with Camera\n");
	}
	is_color_engine_on = 0;
	return 0;
}

int mipi_lgit_lcd_color_engine_on(void)
{
	if(local_mfd0!=NULL && local_mfd0->panel_power_on && !is_color_engine_on) {
		printk("Color Engine_ON Starts with Camera\n");
		mutex_lock(&local_mfd0->dma->ov_mutex);

		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);//HS mode
		mipi_dsi_cmds_tx(&lgit_camera_tx_buf,
		mipi_lgit_pdata->color_engine_on,
		mipi_lgit_pdata->color_engine_on_size);
		printk("%s, %d\n", __func__,is_color_engine_on);
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000); //LP mode

		mutex_unlock(&local_mfd0->dma->ov_mutex);
		printk("Color Engine_ON Ends with Camera\n");
	}
	is_color_engine_on = 1;
	return 0;  
}
#endif //CONFIG_LGIT_COLOR_ENGINE_SWITCH

static int check_stable_lcd_on = 1;

static int mipi_stable_lcd_on(struct platform_device *pdev)
{
       int ret = 0;
       int retry_cnt = 0;

       do {
              pr_info("%s: retry_cnt = %d\n", __func__, retry_cnt);
              ret = mipi_lgit_lcd_off(pdev);

              if (ret < 0) {
                     msleep(3);
                     retry_cnt++;
              }
              else {
                     check_stable_lcd_on = 0;
                     break;
              }
       } while(retry_cnt < 10);
       mdelay(10);

       return ret;
}

int mipi_lgit_lcd_on(struct platform_device *pdev)
{
	int cnt = 0;
	bool cabc_off_state = 1;
	struct msm_fb_data_type *mfd;

	if (check_stable_lcd_on)
		mipi_stable_lcd_on(pdev);

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
#if defined(CONFIG_LGIT_COLOR_ENGINE_SWITCH)
	if(local_mfd0 == NULL)
		local_mfd0 = mfd;
#endif

	pr_info("%s:+ wuxga \n", __func__);

//                                                                                         
	//MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);     //HS mode

	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
		mipi_lgit_pdata->power_on_set_1,
		mipi_lgit_pdata->power_on_set_size_1);
	if (cnt < 0)
		return cnt;


#if defined(CONFIG_LGE_BACKLIGHT_CABC)
	cabc_off_state = lgit_lcd_cabc_state();

	if (cabc_off_state == 1) {
		if ((mipi_lgit_pdata->power_on_set_3_noCABC != NULL) &&
			(mipi_lgit_pdata->power_on_set_size_3_noCABC > 0)) {
			cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
				mipi_lgit_pdata->power_on_set_3_noCABC,
				mipi_lgit_pdata->power_on_set_size_3_noCABC);

			if (cnt < 0)
				return cnt;
			pr_info("%s: CABC OFF\n", __func__);
		}
	} else {
		if ((mipi_lgit_pdata->power_on_set_3 != NULL) &&
			(mipi_lgit_pdata->power_on_set_size_3 > 0)) {
			cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
				mipi_lgit_pdata->power_on_set_3,
				mipi_lgit_pdata->power_on_set_size_3);

			if (cnt < 0)
				return cnt;

			pr_info("%s: CABC ON\n", __func__);
		}
	}
#endif

	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
	mipi_lgit_pdata->power_on_set_2,
	mipi_lgit_pdata->power_on_set_size_2);
	if (cnt < 0)
		return cnt;

	mipi_dsi_op_mode_config(DSI_VIDEO_MODE);
	mdp4_overlay_dsi_video_start();
	mdelay(120);
//                                                                                         

	pr_info("%s:- wuxga \n", __func__);

	return cnt;
}

int mipi_lgit_lcd_off(struct platform_device *pdev)
{
	int cnt = 0;
	struct msm_fb_data_type *mfd;
	
	pr_info("%s:+ wuxga \n", __func__);

	mfd =  platform_get_drvdata(pdev);
	
	if (!mfd)
		return -ENODEV;
	
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);//HS mode
//	mipi_dsi_cmds_tx(mfd, &lgit_tx_buf, 
	cnt = mipi_dsi_cmds_tx(&lgit_tx_buf,
		mipi_lgit_pdata->power_off_set_1,	
		mipi_lgit_pdata->power_off_set_size_1);
	if (cnt < 0) {
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);//LP mode
		return cnt;
	}

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);//LP mode
	pr_info("%s:- wuxga \n", __func__);

	return cnt;
}

static void mipi_lgit_set_backlight_board(struct msm_fb_data_type *mfd)
{
	int level;

	level = (int)mfd->bl_level;
	mipi_lgit_pdata->backlight_level(level, 0, 0);
}

#if defined(CONFIG_LGIT_COLOR_ENGINE_SWITCH)
static ssize_t color_engine_show_on_off(struct device *dev, struct device_attribute *attr, char *buf)
{
	int r = 0;

	pr_info("%s received (prev color_engine_on: %s)\n", __func__, is_color_engine_on? "ON" : "OFF");

	return r;
}

static ssize_t color_engine_store_on_off(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int on_off;

	if (!count)
		return -EINVAL;

	pr_info("%s received (prev color_engine_on: %s)\n", __func__, is_color_engine_on ? "ON" : "OFF");

	on_off = simple_strtoul(buf, NULL, 10);

	printk(KERN_ERR "color_engine_on : %d", on_off);

	if (on_off == 1) {
	    mipi_lgit_lcd_color_engine_on();
	} else if (on_off == 0)
	    mipi_lgit_lcd_color_engine_off();

	return count;
}
DEVICE_ATTR(color_engine_on_off, 0644, color_engine_show_on_off, color_engine_store_on_off);
#endif //CONFIG_LGIT_COLOR_ENGINE_SWITCH

#if defined(CONFIG_LGE_BACKLIGHT_CABC)
static ssize_t lgit_lcd_show_cabc_off(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = snprintf(buf, PAGE_SIZE, "%d\n", mipi_lgit_pdata->cabc_off);
	pr_info("%s: '%d'\n", __func__, mipi_lgit_pdata->cabc_off);
	return ret;
}

static ssize_t lgit_lcd_set_cabc_off(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (!count)
		return -EINVAL;
	mipi_lgit_pdata->cabc_off = simple_strtoul(buf, NULL, 10);
	pr_info("%s: cabc (%s)\n", __func__, mipi_lgit_pdata->cabc_off ? "OFF" : "ON");
	return count;
}
DEVICE_ATTR(cabc_off, 0644, lgit_lcd_show_cabc_off, lgit_lcd_set_cabc_off);

static bool lgit_lcd_cabc_state(void)
{
	return mipi_lgit_pdata->cabc_off;
}
#endif

static int mipi_lgit_lcd_probe(struct platform_device *pdev)
{
#if defined(CONFIG_LGE_R63311_COLOR_ENGINE)
    int err;
#endif
    int err;

	if (pdev->id == 0) {
		mipi_lgit_pdata = pdev->dev.platform_data;
		return 0;
	}

	printk(KERN_INFO "[LCD][DEBUG] %s: mipi lgit lcd probe start\n", __func__);

	msm_fb_add_device(pdev);

#if defined(CONFIG_LGIT_COLOR_ENGINE_SWITCH)
	err = device_create_file(&pdev->dev, &dev_attr_color_engine_on_off);
	if(err < 0)
		printk("[LCD][DEBUG] %s : Cannot create the sysfs\n" , __func__);
#endif //CONFIG_LGIT_COLOR_ENGINE_SWITCH

#if defined(CONFIG_LGE_BACKLIGHT_CABC)
	err = device_create_file(&pdev->dev, &dev_attr_cabc_off);
	if(err < 0)
		printk("[LCD][DEBUG] %s : Cannot create the sysfs\n" , __func__);
#endif

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_lgit_lcd_probe,
	.driver = {
		.name   = "mipi_lgit",
	},
};

static struct msm_fb_panel_data lgit_panel_data = {
	.on		= mipi_lgit_lcd_on,
	.off		= mipi_lgit_lcd_off,
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
		printk(KERN_ERR
		  "[LCD][DEBUG] %s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "[LCD][DEBUG] %s: platform_device_register failed!\n", __func__);
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
	mipi_dsi_buf_alloc(&lgit_camera_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&lgit_shutdown_tx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_lgit_lcd_init);
