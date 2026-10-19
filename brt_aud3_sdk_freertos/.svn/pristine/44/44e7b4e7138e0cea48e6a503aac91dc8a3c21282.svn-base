#ifndef _TEST_FILE_IO_H_
#define _TEST_FILE_IO_H_

#include "btdatatype.h"
#include <wchar.h>
#ifndef tAPP_FIND_HANDLE
#define tAPP_FIND_HANDLE void *
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

UCHAR *App_GetFileName(UCHAR *path);
WCHAR *App_GetFileNameW(WCHAR *path);
void *App_OpenFile(UCHAR *path);
void *App_CreateFile(UCHAR *path);
void *App_OpenFileW(WCHAR *path);
void *App_CreateFileW(WCHAR *path);
UINT32 App_WriteFile(void *obj_hdl, UINT8 *data, UINT32 len);
UINT32 App_ReadFile(void *obj_hdl, UINT8 *buf, UINT32 buf_len);
void App_GetFileSize(void *obj_hdl, BtUINT64Stru *obj_len);
void App_CloseFile(void *obj_hdl);
#ifdef CONFIG_UNLIMIT_RESOURCE
#define MAX_FILE_PATH_LEN 256
#else
#define MAX_FILE_PATH_LEN 128
#endif
void App_DownPath(WCHAR **src_path, UINT32 *src_size, WCHAR *file_name);
void App_UpPath(WCHAR *fpath);
int App_IsFolderW(WCHAR *path);
int WideToUTF8(char* lpw,char* lpa,int len);
int bt_wcslen(const unsigned short *string);
int WideToByte(char *lpw, char *lpa, int len);
int UTF82Unicode(BYTE *lppUtf8, int *pBufSize, unsigned short *pCode);
int UTF8ToWide(char* lpa, char* lpw, int len);
int ByteToWide(char *lpa, char *lpw, int len);
WCHAR* App_GetFileNameW_NEW(WCHAR* fpath);

#endif //_TEST_FILE_IO_H_
