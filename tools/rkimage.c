// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * See README.rockchip for details of the rkimage format
 */

#include "imagetool.h"
#include <image.h>
#include "rkcommon.h"

static uint32_t header;

static void rkimage_set_header(void *buf, struct stat *sbuf, int ifd,
			       struct image_tool_params *params)
{
	memcpy(buf, rkcommon_get_spl_hdr(params), RK_SPL_HDR_SIZE);

	if (rkcommon_need_bootrom_spl(params))
		/*
		 * BootROM with a public hardcoded key encodes the SPL payload for
		 * Rockchip BootROM backward compatibility (legacy SoCs only:
		 * rk3066, rk3188). This is BootROM-mandated obfuscation, not
		 * a security mechanism. v2 Rockchip headers use SHA256 instead.
		 */
		// lgtm[cpp/weak-cryptographic-algorithm]
		rkcommon_bootrom_encode_spl(buf, 0, params->file_size);
}

static int rkimage_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_RKIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/*
 * rk_image parameters
 */
U_BOOT_IMAGE_TYPE(
	rkimage,
	"Rockchip Boot Image support",
	0,
	&header,
	rkcommon_check_params,
	NULL,
	NULL,
	rkimage_set_header,
	NULL,
	rkimage_check_image_type,
	NULL,
	NULL
);
