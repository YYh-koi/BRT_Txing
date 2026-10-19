/*************************************************************************
    > File Name: app_license.c
    > Author: hualiangyu
    > Mail: sheldon_wy@126.com
    > Created Time: Thu 06 Aug 2015 01:51:22 PM CST
 ************************************************************************/
#include "btdatatype.h"
#include "brt_trace.h"

#define TAG "bse_license"

void app_start_check_license()
{
#ifdef LICENSE_BY_NETWORK
	StartSNActivition();
#elif LICENSE_BY_CHIP
	int ret = VerifyTimeQue(34, 35, 36);
	bse_log_info(TAG, "license by hw ret is %d.", ret);
	set_verify(ret);
#endif
}

BOOL app_check_license()
{
	BOOL ret = TRUE;
#ifdef LICENSE_BY_NETWORK
	ret = check_license();
#elif LICENSE_BY_CHIP
	ret = get_verify();
#endif

	return ret;
}

