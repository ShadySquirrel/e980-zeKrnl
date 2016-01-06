/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
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


//FULL HD LCD (BSP_LCD_LCD Driver) ADD LCD Driver
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lgit_wuxga.h"
#include <mach/board_lge.h>


static struct msm_panel_info pinfo;

#define DSI_BIT_CLK_900MHZ

#if 0
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db_LD070WU1 = {
	/* 1200*1920, RGB888, 4 Lane 60 fps video mode */
#if defined(DSI_BIT_CLK_900MHZ)
	/* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0xFA, 0x9B, 0x3B, 0x00, 0x44, 0xE4, 0x3e, 0x9d, 0x41, 0x03, 0x04},
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x00, 0xC1, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	0x40, 0x07, 0x01, 0x00, 0x14, 0x03, 0x00, 0x02,
	0x00, 0x20, 0x00, 0x01},
#endif
};
#endif


static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db_LD089WU1 = {
	/* 1920*1200, RGB888, 4 Lane 60 fps video mode */
#if defined(DSI_BIT_CLK_900MHZ)
	/* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0xE6, 0x9A, 0x3A, 0x00, 0x40, 0xA6, 0x3B, 0x9C, 0x40, 0x03, 0x04},
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x00, 0xBB, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	0x40, 0x07, 0x01, 0x00, 0x14, 0x03, 0x00, 0x02,
	0x00, 0x20, 0x00, 0x01},
#endif
};

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db_LD083WU1 = {
	/* 1200*1920, RGB888, 4 Lane 60 fps video mode */
#if defined(DSI_BIT_CLK_900MHZ)
	/* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0xF9, 0x9B, 0x3D, 0x00, 0x44, 0xDE, 0x40, 0x9D, 0x44, 0x03, 0x04},
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x00, 0xD1, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	0x40, 0x07, 0x01, 0x00, 0x14, 0x03, 0x00, 0x02,
	0x00, 0x20, 0x00, 0x01},
#endif
};

static int __init mipi_video_lgit_wuxga_pt_init(void)
{
	int ret;

#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_video_lgit_wuxga"))
		return 0;
#endif

	if (lge_get_board_revno() == HW_REV_EVB2) {
		pinfo.xres = 1920;
		pinfo.yres = 1200;

		pinfo.lcdc.h_pulse_width = 16;
		pinfo.lcdc.h_back_porch = 40;
		pinfo.lcdc.h_front_porch = 24;

		pinfo.lcdc.v_pulse_width = 6;
		pinfo.lcdc.v_back_porch = 26;
		pinfo.lcdc.v_front_porch = 3;

#if defined(DSI_BIT_CLK_900MHZ)
		pinfo.mipi.t_clk_post = 0x21;
		pinfo.mipi.t_clk_pre = 0x4D;
		pinfo.clk_rate = 889200000;

		pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db_LD089WU1;
#endif
	} else {
		pinfo.xres = 1200;
		pinfo.yres = 1920;

		pinfo.lcdc.h_pulse_width = 32;
		pinfo.lcdc.h_back_porch = 60;
		pinfo.lcdc.h_front_porch = 44;

		pinfo.lcdc.v_pulse_width = 5;
		pinfo.lcdc.v_back_porch = 10;
		pinfo.lcdc.v_front_porch = 5;

#if defined(DSI_BIT_CLK_900MHZ)
		pinfo.mipi.t_clk_post = 0x20;
		pinfo.mipi.t_clk_pre = 0x38;
		pinfo.clk_rate = 933060000;

		pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db_LD083WU1;
#endif
	}

/* For the LD070WU1 */
#if 0
	pinfo.xres = 1200;
	pinfo.yres = 1920;

	pinfo.lcdc.h_pulse_width = 32;
	pinfo.lcdc.h_back_porch = 60;
	pinfo.lcdc.h_front_porch = 48;

	pinfo.lcdc.v_pulse_width = 5;
	pinfo.lcdc.v_back_porch = 6;
	pinfo.lcdc.v_front_porch = 3;

#if defined(DSI_BIT_CLK_900MHZ)
	pinfo.mipi.t_clk_post = 0x21;
	pinfo.mipi.t_clk_pre = 0x51;
	pinfo.clk_rate = 900300000;
#endif
#endif

	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.border_clr = 0;         /* blk */
	pinfo.lcdc.underflow_clr = 0x00;   /* blk */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 0xFF;
	pinfo.bl_min = 0;
	pinfo.fb_num = 2;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = FALSE;

/* jinho.jang 2011.03.22,  Modify code to apply IEF function */

	pinfo.mipi.hfp_power_stop = FALSE;//TRUE; // FALSE; //TRUE;
	pinfo.mipi.hbp_power_stop = FALSE;//TRUE; // FALSE; //TRUE;
	pinfo.mipi.hsa_power_stop = FALSE;//FALSE; //TRUE;

	pinfo.mipi.esc_byte_ratio = 6;
	pinfo.mipi.eof_bllp_power_stop = TRUE; //FALSE; //TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
 	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
#if defined(DSI_BIT_CLK_900MHZ)
	pinfo.mipi.frame_rate = 60;	
#endif
	pinfo.mipi.stream = 0;		/* dma_p */
	pinfo.mipi.mdp_trigger = 0;        /* DSI_CMD_TRIGGER_SW */
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;


	ret = mipi_lgit_device_register(&pinfo, MIPI_DSI_PRIM, MIPI_DSI_PANEL_WUXGA);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_lgit_wuxga_pt_init);

