/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include "hisi_fb.h"

/*
** for debug, S_IRUGO
** /sys/module/hisifb/parameters
*/
//lint -e305, -e514, -e84, -e21, -e846, -e778, -e866, -e708
int g_dss_effect_sharpness1D_en = 1;

int g_dss_effect_sharpness2D_en = 0;

int g_dss_effect_acm_ce_en = 1;
//lint +e305, +e514, +e84, +e21, +e846, +e778, +e866, +e708


static struct dsm_dev dsm_lcd = {
	.name = "dsm_lcd",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

struct dsm_client *lcd_dclient = NULL;

void dss_underflow_debug_func(struct work_struct *work)
{
	struct clk *ddr_clk = NULL;
	unsigned long curr_ddr = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	static u32 underflow_index = 0;
	static ktime_t underflow_timestamp[UNDERFLOW_EXPIRE_COUNT];
	s64 underflow_msecs = 0;
	static bool init_underflow_timestamp = false;
	int i;

	if (!init_underflow_timestamp) {
		underflow_timestamp[underflow_index] = ktime_get();
		underflow_index ++;
	}
	if (underflow_index == UNDERFLOW_EXPIRE_COUNT) {
		init_underflow_timestamp = true;
		underflow_timestamp[UNDERFLOW_EXPIRE_COUNT - 1] = ktime_get();
		underflow_msecs = ktime_to_ms(underflow_timestamp[UNDERFLOW_EXPIRE_COUNT - 1]) - ktime_to_ms(underflow_timestamp[0]);
		for(i = 0; i < UNDERFLOW_EXPIRE_COUNT - 1; i ++)
			underflow_timestamp[i] = underflow_timestamp[i+1];
	}

	ddr_clk = clk_get(NULL, "clk_ddrc_freq");
	if (ddr_clk) {
		curr_ddr = clk_get_rate(ddr_clk);
		clk_put(ddr_clk);
	} else {
		HISI_FB_ERR("Get ddr clk failed");
	}

	if (work) {
		hisifd = container_of(work, struct hisi_fb_data_type, dss_underflow_debug_work);
		if (g_underflow_stop_perf_stat) {
			dumpDssOverlay(hisifd, &hisifd->ov_req, false);
		}
	} else {
		HISI_FB_ERR("Get hisifd failed");
	}

	HISI_FB_INFO("Current ddr is %lu\n", curr_ddr);

	if ((underflow_msecs <= UNDERFLOW_INTERVAL) && (underflow_msecs > 0)) {
		HISI_FB_INFO("abnormal, underflow times:%d, interval:%llu, expire interval:%d\n",
			UNDERFLOW_EXPIRE_COUNT, underflow_msecs, UNDERFLOW_INTERVAL);
	} else {
		HISI_FB_INFO("normal, underflow times:%d, interval:%llu, expire interval:%d\n",
			UNDERFLOW_EXPIRE_COUNT, underflow_msecs, UNDERFLOW_INTERVAL);
		return;
	}

	if (lcd_dclient) {
		if (!dsm_client_ocuppy(lcd_dclient)) {
			dsm_client_record(lcd_dclient, "ldi underflow!\n");
			dsm_client_notify(lcd_dclient, DSM_LCD_LDI_UNDERFLOW_NO);
		}
	}
}

void hisifb_debug_register(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	// dsm lcd
	if(!lcd_dclient) {
		lcd_dclient = dsm_register_client(&dsm_lcd);
	}

	// dss underflow debug
	hisifd->dss_underflow_debug_workqueue = create_singlethread_workqueue("dss_underflow_debug");
	if (!hisifd->dss_underflow_debug_workqueue) {
		HISI_FB_ERR("fb%d, create dss underflow debug workqueue failed!\n", hisifd->index);
	} else {
		INIT_WORK(&hisifd->dss_underflow_debug_work, dss_underflow_debug_func);
	}
}

void hisifb_debug_unregister(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);
}
