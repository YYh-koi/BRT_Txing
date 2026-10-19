/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Copyright (c) 1999-2014 BARROT Corporation
 *
 * All rights reserved.
 ---------------------------------------------------------------------------*/

#include "fileio.h"
//#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <sys/stat.h>
#include <sys/types.h>
//#include <dirent.h>
#include <stdlib.h>

/**
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * Module Name:
     fileio.c
 * @Description:
       This file defines the file-associated operation interfaces.

 * @author Zhu Minjue
 * @version 1.0
 * @date 2014-07-08
 ---------------------------------------------------------------------------*/

UCHAR *App_GetFileName(UCHAR *path)
{
	return (UCHAR *) basename((const char *) path);
}

WCHAR *App_GetFileNameW(WCHAR *path)
{
	return (WCHAR *) basename((const char *) path); // XXX
}

void *App_OpenFile(UCHAR *path)
{
	FILE *fp;
	fp = fopen((const char *) path, "rb");

	return (void *) fp;
}

void *App_CreateFile(UCHAR *path)
{
	FILE *fp;
	fp = fopen((const char *) path, "wb");

	return (void *) fp;
}

void *App_OpenFileW(WCHAR *path)
{
	FILE *fp;
	fp = fopen((const char *) path, "rb");

	return (void *) fp;
}

void *App_CreateFileW(WCHAR *path)
{
	FILE *fp;
	fp = fopen((const char *) path, "wb");

	return (void *) fp;
}

UINT32 App_WriteFile(void *obj_hdl, UINT8 *data, UINT32 len)
{
	FILE *fp = (FILE *) obj_hdl;
	if (fp != NULL) {
		return fwrite(data, 1, len, fp);
	}

	return 0;
}

UINT32 App_ReadFile(void *obj_hdl, UINT8 *buf, UINT32 buf_len)
{
	FILE *fp = (FILE *) obj_hdl;
	int count = 0;
	if (fp != NULL) {
		count = fread(buf, 1, buf_len, fp);
	}

	return count;
}

#define _FILE_OFFSET_BITS 64

void App_GetFileSize(void *obj_hdl, BtUINT64Stru *obj_len)
{
	FILE *fp = (FILE *) obj_hdl;
	if (fp != NULL) {
		fseeko(fp, 0L, SEEK_END);
		off_t offset = ftell(fp);
		fseeko(fp, 0L, SEEK_SET);
		obj_len->low32 = offset;

		if (sizeof(offset) > 4) {
			obj_len->high32 = ((long long) offset) >> 32;
		} else {
			obj_len->high32 = 0;
		}
	}
}

void App_CloseFile(void *obj_hdl)
{
	FILE *fp = (FILE *) obj_hdl;
	if (fp != NULL) {
		fclose(fp);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Check whether the specified path is a folder or not.
	0  - Not a folder.
	1  - A folder.
	-1 - Invalid path.
------------------------------------------------------------------------------------*/
int App_IsFolderW(WCHAR *path)
{
	struct stat attrs;
	int ret = -1;
	int slen = bt_wcslen(path) + 1;
	UCHAR file_path[MAX_FILE_PATH_LEN];

	memset(file_path, 0, sizeof(file_path));
	WideToByte(path, file_path, 0);
	if (slen > MAX_FILE_PATH_LEN) {
		return -1;
	}
	if (stat(file_path, &attrs) != -1) {
		if ((attrs.st_mode & S_IFMT) == S_IFDIR) {
			ret = 1;
		} else {
			ret = 0;
		}
	}
	return ret;
}

WCHAR* App_GetFileNameW_NEW(WCHAR* fpath)
{
	WCHAR * end = fpath+bt_wcslen(fpath);

	while (end > fpath) {
		end --;
		if (*end == '/') {
			end ++;
			break;
		}
	}

	return end;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Navigate local to the parent.
------------------------------------------------------------------------------------*/
void App_UpPath(WCHAR *fpath)
{
	WCHAR *end = App_GetFileNameW_NEW(fpath);
	if (end) {
		if (end > (fpath + 1)) {
			*(end - 1) = 0;
		}
		memset(end, 0, bt_wcslen(end) * sizeof(WCHAR));
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Append the file or sub-folder name to the current path.
------------------------------------------------------------------------------------*/
void App_DownPath(WCHAR **src_path, UINT32 *src_size, WCHAR *file_name)
{
	WCHAR path_size;
	WCHAR *old;
	WCHAR *end;

	old = *src_path;
	end = *src_path + bt_wcslen(*src_path);

	path_size = bt_wcslen(file_name) + (end - *src_path) + 2; /* sizeof("\\") */
	if (path_size > *src_size) {
		*src_path = realloc(*src_path, path_size*sizeof(WCHAR));
		end = *src_path + (end - old);
		memset(end, 0, (path_size-bt_wcslen(*src_path))*sizeof(WCHAR));
		*src_size = path_size;
	}
	if (end == *src_path) {
		*end = '/';
	} else if (*(end - 1) != '/') {
		*end = '/';
	} else {
		end--;
	}
	memcpy(end+1, file_name, bt_wcslen(file_name)*sizeof(WCHAR));
}

int BT_GetUTF8CharCount(unsigned short  wCode)
{
	if (wCode < 0x80)
		return 1;
	else if (wCode < 0x800)
		return 2;
	else
		return 3;
}
#if 0
static void BT_Unicode2UTF8(unsigned short wCode, BYTE **lppUtf8)
{
	if (wCode < 0x80)
		*(*lppUtf8)++ = (BYTE)wCode;
	else if (wCode < 0x800) {
		*(*lppUtf8)++ = (BYTE)((wCode >> 6) | 0xC0);
		*(*lppUtf8)++ = (BYTE)((wCode & 0x3F) | 0x80);
	} else {
		*(*lppUtf8)++ = (BYTE)((wCode >> 12) | 0xE0);
		*(*lppUtf8)++ = (BYTE)(((wCode >> 6) & 0x3F) | 0x80);
		*(*lppUtf8)++ = (BYTE)((wCode & 0x3F) | 0x80);
	}
}
#else
static BYTE * BT_Unicode2UTF8(unsigned short wCode, BYTE *lppUtf8)
{
	if (wCode < 0x80) {
		*(lppUtf8) = (BYTE)wCode;
		lppUtf8++;
	} else if (wCode < 0x800) {
		*(lppUtf8) = (BYTE)((wCode >> 6) | 0xC0);
		*(lppUtf8+1) = (BYTE)((wCode & 0x3F) | 0x80);
		lppUtf8 = lppUtf8+2;
	} else {
		*(lppUtf8) = (BYTE)((wCode >> 12) | 0xE0);
		*(lppUtf8+1) = (BYTE)(((wCode >> 6) & 0x3F) | 0x80);
		*(lppUtf8+2) = (BYTE)((wCode & 0x3F) | 0x80);
		lppUtf8 = lppUtf8+3;
	}
	return lppUtf8;
}

#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
	Map a wide-character string to a new character string using CP_UTF8 code page.
Arguments:
	char*	lpw ;	wide-character string.
	char*	lpa	;	buffer for new string.
	int		len	;	size of buffer.
					If this value is zero, the function returns	the number of
					bytes required for the buffer.(In this case, the "lpa" is not used.)
return:
	the number of bytes written to the buffer pointed to by lpa.
	The number includes the byte for the null terminator.
---------------------------------------------------------------------------*/
#if 0
int WideToUTF8(char* lpw,char* lpa,int len)
{

	int utfLen=0;
	int i;
	char * p=lpw;
	unsigned short wcode;
	int uniLen=0;
	//get the unicode length
	while(!((*p==0)&&(*(p+1)==0)))
		p++;
	uniLen=p-lpw+2;
	if(uniLen%2!=0)
		++uniLen;
	char * cursor=lpa;
	for (i=0; i<uniLen; i=i+2) {
		wcode=*((unsigned short *)lpw);
		if(lpa!=0) {
			BT_Unicode2UTF8(wcode,& cursor);
		}
		utfLen=utfLen+BT_GetUTF8CharCount(wcode);
		lpw=lpw+2;
	}

	return utfLen;
}
#else
int WideToUTF8(char* lpw,char* lpa,int len)
{
	int utfLen=0;
	int i;
	unsigned short * p=(unsigned short *)lpw;
	unsigned short wcode;
	int uniLen=0;
	char * cursor=lpa;

	//get the unicode length
	while(*p != 0) {
		p++;
	}

	uniLen=(p-(unsigned short *)lpw);
	uniLen++;
	for (i=0; i<uniLen; i++) {
		wcode=*((unsigned short *)lpw);
		if((cursor-lpa+3)<(MAX_FILE_PATH_LEN)) {
			cursor = BT_Unicode2UTF8(wcode, cursor);
		} else {
			break;
		}
		utfLen=utfLen+BT_GetUTF8CharCount(wcode);
		lpw=lpw+2;
	}
	return utfLen;
}
#endif

int bt_wcslen(const unsigned short *string)
{
	int count = 0;
	while (string[count] != 0x0)
		count++;
	return count;
}

/*porting for lenovo
Unicode to Utf8
*/
int WideToByte(char *lpw, char *lpa, int len)
{
	return WideToUTF8( lpw,lpa,len);

}

int UTF82Unicode(BYTE *lppUtf8, int *pBufSize, unsigned short *pCode)
{
	int bufsize = 0;

	if (lppUtf8 == NULL || pCode == NULL || pBufSize == NULL || *pBufSize <= 0)
		return FALSE;
	//pengxi modify

	while (*pBufSize>0) {
		int nAdd;
		if (*lppUtf8 >= 0xE0) {
			if (*pBufSize < 3 || *(lppUtf8 + 1) < 0x80 || *(lppUtf8 + 2) < 0x80) {
				/* Invalid characters. Parse as separate character. */
				nAdd = 1;
				*pCode = *lppUtf8 & 0x7F;
				if (*pBufSize > 1) {
					*(pCode + 1) = *(lppUtf8 + 1) & 0x7F;
					nAdd++;
				}
				if (*pBufSize > 2) {
					*(pCode+ 2) = *(lppUtf8 + 2) & 0x7F;
					nAdd++;
				}
			} else {
				nAdd = 3;
				*pCode = ((*lppUtf8 & 0x0F) << 12) + ((*(lppUtf8 + 1) & 0x3F) << 6) + (*(lppUtf8 + 2) & 0x3F);
			}
			lppUtf8 += nAdd;
			*pBufSize -= nAdd;
		} else if (*lppUtf8 >= 0xC0) {
			if (*pBufSize < 2 || *(lppUtf8 + 1) < 0x80) {
				/* Invalid characters. Parse as separate character. */
				nAdd = 1;
				*pCode = *lppUtf8 & 0x7F;
				if (*pBufSize > 1) {
					*(pCode + 1) = *(lppUtf8 + 1) & 0x7F;
					nAdd++;
				}
			} else {
				nAdd = 2;
				*pCode = ((*lppUtf8 & 0x1F) << 6) + (*(lppUtf8 + 1) & 0x3F);
			}
			lppUtf8 += nAdd;
			*pBufSize -= nAdd;
		} else if (*lppUtf8 < 0x80) {
			*pCode = *lppUtf8;
			lppUtf8++;
			(*pBufSize)--;
		} else {
			/* An invalid character. */
			*pCode = *lppUtf8 & 0x7F;
			lppUtf8++;
			(*pBufSize)--;
		}
		pCode++;
		bufsize++;
	}

	return bufsize;
}


int UTF8ToWide(char* lpa, char* lpw, int len)
{
	return UTF82Unicode((BYTE*)lpa, &len, (unsigned short*)lpw);
}

#if 0
int ByteToWide(char* lpa, char* lpw, int len)
{
	// FIXME
	int i = 0;
	short * lw = (short *)lpw;
	for (i=0; i<len; i++) {
		lw[i] = lpa[i];
	}
	return len;
}
#else
int ByteToWide(char *lpa, char *lpw, int len)
{
	len=strlen(lpa) + 1;
	if(lpw==NULL) {
		return 0;
	}
	return UTF82Unicode((BYTE*)lpa, &len, (unsigned short*)lpw);
}
#endif

