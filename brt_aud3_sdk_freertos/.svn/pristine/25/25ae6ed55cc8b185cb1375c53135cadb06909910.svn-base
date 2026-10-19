#include "btdatatype.h"
#include "profile.h"
#include "profiles_def.h"
#ifdef CONFIG_SPP
#include "app_rfcomm.h"
#endif
profile_t *g_profiles[MAX_PROFILE_NUM] = {
	&g_mgr_profile,
	&g_gap_profile,
	&g_hf_profile,
	&g_a2dp_profile,
	&g_avrcp_profile,
#ifdef CONFIG_PBAP
	&g_pbap_profile,
#else
	NULL,
#endif
#ifdef CONFIG_HID
	&g_hid_profile,
#else
	NULL,
#endif
#ifdef CONFIG_SPP
	&g_spp_profile,
#else
	NULL,
#endif
#ifdef CONFIG_CBP
	&g_cbp_profile,
#else
	NULL,
#endif
#ifdef CONFIG_FTP
	&g_ftp_profile,
#else
	NULL,
#endif
#ifdef CONFIG_DUN
	&g_dun_profile,
#else
	NULL,
#endif
#ifdef CONFIG_GATT
	&g_gatt_profile,
#else
	NULL,
#endif
#ifdef CONFIG_OPP
	&g_opp_profile,
#else
	NULL,
#endif
#ifdef CONFIG_MAP
	&g_map_profile,
#else
	NULL,
#endif
#ifdef CONFIG_PAN
	&g_pan_profile,
#else
	NULL,
#endif
	NULL,
#ifdef CONFIG_ANCS_CLIENT
	&g_ancs_profile,
#endif
};

