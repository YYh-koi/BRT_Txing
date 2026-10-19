/*************************************************************************
    > File Name: app_active.c
    > Author: hualiangyu
    > Created Time: Mon 25 May 2015 04:55:47 PM CST
 ************************************************************************/
#include "btdatatype.h"
#include<stdio.h>
#include <unistd.h>

#include <pthread.h>
#include "debug.h"
#include "btdatatype.h"
#include "fsm.h"
#include "brt_trace.h"
#include "../ProductActivation/BsPA_CommFunc/BsPA_def.h"
#include "../ProductActivation/BsPA_Sdk/BsPA_Sdk_ui.h"
#include "../ProductActivation/curl/curl.h"
#include "../chk_lic/CheckLicense.h"

#define TAG "bse_active"
#ifdef LICENSE_BY_NETWORK
#define SN_ACTIVE 1
#define LICENSE_ACTIVE 2

//BTSETTINGS
#define INI_NAME "/data/misc/brlink/BtSettings.INI"
#define MAX_PATH 256
#define BRLINKCE_FEATURE        0x100000
#define BRLINKCE_FEATURE_AEC    0x000001


#define MAXBUF 100
#define MAXSNBUF 400
#define URL_ACTIVATION_KEY "http://license.brlink.com/activate.asp?para="
#ifdef OLD_URL
#define URL_ACTIVATION_SN  "http://www.brlink.com/client/Activate.aspx?InstallID="
#define URL_FIRSTACTIVATION_SN  "http://www.brlink.com/client/Activate.aspx?first=true&InstallID="
#else
#define URL_ACTIVATION_SN  "http://123.56.191.137:8082/activate.asp?InstallID="
#define URL_FIRSTACTIVATION_SN  "http://123.56.191.137:8082/activate.asp?first=true&InstallID="
#endif
//#define AC_CONFIG_FILE "BtActive.INI"
//#define AC_FILE_ROOT_DIR "/data/misc/brlink"
#define AC_CONFIG_FILE_PATH             "/data/misc/brlink/BtActive.INI"
#define AC_REF_CONFIG_FILE_PATH			"/system/etc/brlink/BtActive.INI"
//compose_file_path(AC_FILE_ROOT_DIR, AC_CONFIG_FILE)

enum {LICENSE_EVA, LICENSE_REL, LICENSE_UNKNOWN, LICENSE_CHECKING};
enum {ACTIVATE_NETWORK_ERR = -1, ACTIVATE_FAIL, ACTIVATE_SUCESS};


#ifndef BSPACM_PUT32
#define BSPACM_PUT32(p, n) \
	    *((UCHAR*)p) = (UCHAR)(n >> 24); \
    *(((UCHAR*)p)+1) = (UCHAR)((n << 8) >> 24);\
    *(((UCHAR*)p)+2) = (UCHAR)((n << 16) >> 24);\
    *(((UCHAR*)p)+3) = (UCHAR)((n << 24) >> 24);
#endif

static UINT8 g_sn[BSPA_SERIALNUMBER_LENGTH + 1] = {0};
static BOOL g_isfirst_active = TRUE;
static BOOL g_b_5m_data_ctrl = FALSE;
static int nStatus = LICENSE_UNKNOWN;

size_t LicenseNetworkCbk(void *ptr, size_t size, size_t nmemb, void *stream);
size_t SNNetworkCbk(void *ptr, size_t size, size_t nmemb, void *stream);

static BOOL g_is_activate = TRUE;

int check_license()
{
	return g_is_activate;
}
void SNCheckProc()
{
	bse_log_api(TAG, "%s", __func__);
	char activiationID[MAX_PATH] = {0};

	check_product_process(activiationID, MAX_PATH);
	//TAG 1
	//bse_log_verbose(TAG, "activiationID is %s\r\n", activiationID);
	if (strcmp(activiationID, "ERROR_CHECK") == 0) {
		bse_log_verbose(TAG, "SNCheckProc ERROR_CHECK");
		int ret = access(AC_CONFIG_FILE_PATH, F_OK);
		if (0 == ret) {
			remove(AC_CONFIG_FILE_PATH);
		}
		g_is_activate = FALSE;
		return;
	}
	if (strcmp(activiationID, "PASS_CHECK") == 0) {
		bse_log_verbose(TAG, "SNCheckProc PASS_CHECK");
		g_is_activate = TRUE;
		return;
	}

	char strURL[MAXSNBUF] = { 0 };

	if (g_isfirst_active == FALSE) {
		memcpy(strURL, URL_ACTIVATION_SN, strlen(URL_ACTIVATION_SN));
		memcpy(strURL + strlen(URL_ACTIVATION_SN), activiationID, MAX_PATH);
	} else {
		memcpy(strURL, URL_FIRSTACTIVATION_SN, strlen(URL_FIRSTACTIVATION_SN));
		memcpy(strURL + strlen(URL_FIRSTACTIVATION_SN), activiationID, MAX_PATH);
	}
	//bse_log_verbose(TAG, "strURL is %s", strURL);
	ClientUrlThreadProc(strURL, SN_ACTIVE);

}

void StartSNActivition()
{
	bse_log_verbose(TAG, "%s", __func__);
	pthread_attr_t attr;
	pthread_t active;

	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


	pthread_create(&active, &attr, (void*)SNCheckProc, NULL);
	pthread_detach(active);
}

#define ACTIVATION_SN

#define LINE_NUM 4
const char* line[LINE_NUM] = {
	"[BRLinkPA]",
	ACTIVATION_SN,
	"ActivationID=PASS_CHECK",
	"onceactive=no"
};

int createSNFile(const char *destination)
{
	FILE* desFp;
	if ((desFp = fopen(destination, "w+")) == NULL) {
		//open failed
		return FALSE;
	}

#if 1
	char cmd[100] = {0};
	sprintf(cmd, "chmod 666 %s", destination);
	system(cmd);
#endif
	int i = 0;
	for(i = 0; i < LINE_NUM; i++) {
		if (fwrite(line[i], strlen(line[i]), 1, desFp) != 1) {
			bse_log_verbose(TAG, "createTextFile %s has not been written completely", destination);
			return FALSE;
		}
		fwrite("\n", 1, 1, desFp);
	}
	fclose(desFp);

	return TRUE;
}

int CopySNFile(const char *original, const char *destination)
{
	FILE *oriFp, *desFp;
	const int BUF_LEN = 10;
	char *buffer = (char *)calloc(BUF_LEN, sizeof(char));
	int writtenlen = BUF_LEN;

	if (original == NULL || destination == NULL) {
		return FALSE;
	}

	if (buffer == NULL) {
		return FALSE;
	}

	if (access(original, 0) < 0) {
		if (buffer != NULL) {
			free(buffer);
			buffer = NULL;
		}

		return FALSE;
	}

	if ((oriFp = fopen(original, "r")) == NULL) {
		if (buffer != NULL) {
			free(buffer);
			buffer = NULL;
		}

		return FALSE;
	}

	if ((desFp = fopen(destination, "w+")) == NULL) {
		if (buffer != NULL) {
			free(buffer);
			buffer = NULL;
		}

		fclose(oriFp);
		return FALSE;
	}

	while (!feof(oriFp)) {
		writtenlen = fread(buffer, 1, BUF_LEN, oriFp);

		if (fwrite(buffer, writtenlen, 1, desFp) != 1) {
			bse_log_verbose(TAG, "copyTextFile %s has not been written completely", destination);
		}
	}

	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}

	fclose(oriFp);
	fclose(desFp);

	return TRUE;
}

int check_product_process(char* key, int len)
{
	bse_log_verbose(TAG, "%s", __func__);
	int ret = -1;
	const char *error_string = "ERROR_CHECK";
	const char *pass_string = "PASS_CHECK";

	char activation_id[128] = {0};
	char activehistory[10] = {0};
	if (access(AC_CONFIG_FILE_PATH, 0) != 0) {
		//int ret = CopySNFile(AC_REF_CONFIG_FILE_PATH, AC_CONFIG_FILE_PATH);//AC_REF_FILE_PATN);
		ret = createSNFile(AC_CONFIG_FILE_PATH);
		if(!ret) {
			bse_log_verbose(TAG, "Copy SN error, stop BT");
			g_is_activate = FALSE;
		}
	}
	get_private_profile_string(BSPA_REG_PASECTION, "ActivationID", "", activation_id, 127, AC_CONFIG_FILE_PATH);
	get_private_profile_string(BSPA_REG_PASECTION, "onceactive", "", activehistory, 10, AC_CONFIG_FILE_PATH);

	//bse_log_verbose(TAG, "check_product_process activehistory, is  %s", activehistory);
	if (strcmp(activehistory, "no") == 0) {
		g_isfirst_active = TRUE;
	} else {
		g_isfirst_active = FALSE;
	}
	if (strlen(activation_id) <= 1) {
		//bse_log_verbose(TAG, "check_product_process iStatus strlen(activation_id) <= 1");
		if (get_sn_from_device() == FALSE) {

			bse_log_verbose(TAG, "check_product_process iStatus get_sn_from_device() == FALSE");
			strcpy(key, error_string);
			return 0;
		} else {
			bse_log_verbose(TAG, "check_product_process iStatus get_sn_from_device() != FALSE");
			char szInstallID[300] = { 0 };
			int nInstallIDLen = 300;
			UINT16 nManufID = 0x0048;
			unsigned char byteBrlinkFeature[BSPA_MAX_FEATURE_LENGTH] = { 0 };
			unsigned char * pCurrPos = byteBrlinkFeature + BSPA_MAX_FEATURE_LENGTH - 4;

			BSPACM_PUT32(pCurrPos, BRLINKCE_FEATURE);
			UINT8 *szBrlinkVersion = "1";
			ret = BsPA_GetInstallationID(nManufID, byteBrlinkFeature, szBrlinkVersion, szInstallID, &nInstallIDLen);
			//bse_log_verbose(TAG, "check_product_process ret is %x", ret);
			if (ret != BSPA_OK) {
				strcpy(key, error_string);
				return 0;
			} else {
				strcpy(key, szInstallID);
				return 0;
			}
		}

	} else {
		//bse_log_verbose(TAG, "check_product_process iStatus strlen(activation_id) > 1");
		UINT feature = 0;
		ret = BsPA_CheckActivationIDExpired(&feature);
		//bse_log_info(TAG, "Check activation id expired , feature is %x", feature);
		if (ret == BSPA_OK) {
			g_b_5m_data_ctrl = FALSE;
			strcpy(key, pass_string);
		} else {
			char szInstallID[300] = { 0 };
			int nInstallIDLen = 300;
			UINT16 nManufID = 0x0048;
			unsigned char byteBrlinkFeature[BSPA_MAX_FEATURE_LENGTH] = { 0 };
			unsigned char * pCurrPos = byteBrlinkFeature + BSPA_MAX_FEATURE_LENGTH - 4;

			BSPACM_PUT32(pCurrPos, BRLINKCE_FEATURE);
			UINT8 *szBrlinkVersion = "1";

			ret = BsPA_GetInstallationID(nManufID, byteBrlinkFeature, szBrlinkVersion, szInstallID, &nInstallIDLen);
			bse_log_verbose(TAG, "ret is %x", ret);

			if (ret != BSPA_OK) {
				strcpy(key, error_string);
				return 0;
			} else {
				strcpy(key, szInstallID);
				return 0;
			}
		}
	}
	return 0;
}


BOOL ClientUrlThreadProc(const wchar_t * const pszAspUrl, int type)
{

	bse_log_verbose(TAG, "Enter %s\r\n", __func__);
	int nBytesRead = 0;

	CURL *curl;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, pszAspUrl);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	nStatus = LICENSE_UNKNOWN;
	switch (type) {
	case LICENSE_ACTIVE:
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, LicenseNetworkCbk);
		break;
	case SN_ACTIVE: {
		CURLcode ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SNNetworkCbk);
	}
	break;
	default:
		return FALSE;
	}
	bse_log_verbose(TAG, "ClientUrlThreadProc  @@@@@");

	curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return TRUE;
}


BOOL get_sn_from_device()
{
	BOOL isSnExist = TRUE;                // Set true first
	if (isSnExist) {
		memcpy(g_sn, "E3PGCVWK7CZAXYYDCPVZ2FBEZ", BSPA_SERIALNUMBER_LENGTH);
		int ret = BsPA_WriteSNToRegistry(g_sn);
		if (BSPA_OK != ret)
			return FALSE;
		else
			return TRUE;
	} else {
		return FALSE;
	}

}


size_t LicenseNetworkCbk(void *ptr, size_t size, size_t nmemb, void *stream)
{
	bse_log_verbose(TAG, "Enter %s", __func__);
	char *newKey = ptr;

	int nRet = CheckKeyInvalid(newKey);

	if (VALID_KEY == nRet) {
		set_private_profile_string("LICENCEKEY", "KEY", ptr, INI_NAME);
		return ACTIVATE_SUCESS;
	} else {
		nStatus = LICENSE_EVA;
		return ACTIVATE_NETWORK_ERR;
	}
}

int check_product_info_process()
{
	bse_log_verbose(TAG, "Enter %s", __func__);
	INT32 result = BSPA_ER_CODE_SN_INVALID;
	UINT feature = 0;
	result = BsPA_CheckActivationIDExpired(&feature);
	if(result == BSPA_ER_KEYEXPIRED) {
		//TODO
		return result;
	}
	if (result == BSPA_OK) {
		bse_log_info(TAG, "SNCheckProc Register success");
		g_b_5m_data_ctrl = FALSE;
		g_is_activate = TRUE;
		set_private_profile_string(BSPA_REG_PASECTION, "onceactive", "yes", AC_CONFIG_FILE_PATH);
	} else {
		bse_log_info(TAG, "SNCheckProc Register fail");
		g_b_5m_data_ctrl = TRUE;
		int ret = access(AC_CONFIG_FILE_PATH, F_OK);
		if (0 == ret) {
			remove(AC_CONFIG_FILE_PATH);
		}
		g_is_activate = FALSE;
	}
	return result;
}

size_t SNNetworkCbk(void *ptr, size_t size, size_t nmemb, void *stream)
{
	bse_log_api(TAG, "Enter %s\r\n", __func__);
	char *key = ptr;

	if (key == NULL) {
		return 0;
	}
	//bse_log_verbose(TAG, "key is %s", key);
	if (strlen(key) > 180) {
//		set_private_profile_string(BSPA_REG_PASECTION, "ActivationID", "", AC_CONFIG_FILE_PATH);
		int ret = access(AC_CONFIG_FILE_PATH, F_OK);
		if (0 == ret) {
			remove(AC_CONFIG_FILE_PATH);
		}
		return 0;
	} else {
		bse_log_verbose(TAG, "strlen_key is < 180");
		set_private_profile_string(BSPA_REG_PASECTION, "ActivationID", key, AC_CONFIG_FILE_PATH);
	}

	check_product_info_process();

	return 0;
}
#endif
