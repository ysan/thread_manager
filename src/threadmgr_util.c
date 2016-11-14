#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

#include "threadmgr.h"
#include "threadmgr_util.h"


/*
 * Constant define
 */
#define SYSTIME_STRING_SIZE			(2+1+2+1+2+1+2+1+2 +1)
#define THREAD_NAME_STRING_SIZE		(10+1)
#define LOG_STRING_SIZE				(128)

/*
 * Type define
 */

/*
 * Variables
 */

/*
 * Prototypes
 */
void putsSysTime (void); // extern
static void getSysTime (char *pszOut, size_t nSize);
void putsThreadName (void); // extern
static void getThreadName (char *pszOut, size_t nSize);
void getTimeOfDay (struct timeval *p); //extern
void putsLog (
	FILE *pFp,
	EN_LOG_TYPE enLogType,
	const char *pszFile,
	const char *pszFunc,
	int nLine,
	const char *pszFormat,
	...
);
void deleteLF (char *p); // extern


/**
 * システム現在時刻を出力
 * ログマクロ用
 */
void putsSysTime (void)
{
	char szTime [SYSTIME_STRING_SIZE];
	memset (szTime, 0x00, SYSTIME_STRING_SIZE);
	getSysTime (szTime, sizeof (szTime));
	fprintf (stdout, "%s", szTime);
}

/**
 * システム現在時刻を取得
 * MM/dd HH:mm:ss形式
 */
static void getSysTime (char *pszOut, size_t nSize)
{
	time_t timer;
	struct tm *pstTmLocal = NULL;
	struct tm stTmLocalTmp;

	timer = time (NULL);
	pstTmLocal = localtime_r (&timer, &stTmLocalTmp); /* スレッドセーフ */

	snprintf (
		pszOut,
		nSize,
		"%02d/%02d %02d:%02d:%02d",
		pstTmLocal->tm_mon+1,
		pstTmLocal->tm_mday,
		pstTmLocal->tm_hour,
		pstTmLocal->tm_min,
		pstTmLocal->tm_sec
	);
}

/**
 * pthread名称を出力
 * ログマクロ用
 */
void putsThreadName (void)
{
	char szPutName [THREAD_NAME_STRING_SIZE];
	memset (szPutName, 0x00, THREAD_NAME_STRING_SIZE);
	getThreadName (szPutName, THREAD_NAME_STRING_SIZE);
	fprintf (stdout, "%s", szPutName);
}

/**
 * pthread名称を取得
 */
static void getThreadName (char *pszOut, size_t nSize)
{
	char szName[16];
	memset (szName, 0x00, sizeof(szName));
	pthread_getname_np (pthread_self(), szName, sizeof(szName));

	strncpy (pszOut, szName, nSize -1);
	uint32_t len = (uint32_t)strlen(pszOut);
	if (len < THREAD_NAME_STRING_SIZE -1) {
		int i = 0;
		for (i = 0; i < (THREAD_NAME_STRING_SIZE -1 -len); i ++) {
			*(pszOut + len + i) = ' ';
		}
	}
}

/**
 * getTimeOfDay wrapper
 * gettimeofdayはスレッドセーフ
 */
void getTimeOfDay (struct timeval *p)
{
	if (!p) {
		return ;
	}

	gettimeofday (p, NULL);
}

/**
 * putsLog
 * ログ出力
 */
void putsLog (
	FILE *pFp,
	EN_LOG_TYPE enLogType,
	const char *pszFile,
	const char *pszFunc,
	int nLine,
	const char *pszFormat,
	...
)
{
	char szBufVa [LOG_STRING_SIZE];
	char szTime [SYSTIME_STRING_SIZE];
    char szThreadName [THREAD_NAME_STRING_SIZE];
	va_list va;
	char type;
	char szPerror[32];

	memset (szBufVa, 0x00, sizeof (szBufVa));
	memset (szTime, 0x00, sizeof (szTime));
    memset (szThreadName, 0x00, sizeof (szThreadName));
	memset (szPerror, 0x00, sizeof (szPerror));

	switch (enLogType) {
	case EN_LOG_TYPE_I:
		type = 'I';
		break;

	case EN_LOG_TYPE_N:
		type = 'N';
		fprintf (stdout, THM_TEXT_GREEN);
		break;

	case EN_LOG_TYPE_W:
		type = 'W';
		fprintf (stdout, THM_TEXT_BOLD_TYPE);
		fprintf (stdout, THM_TEXT_YELLOW);
		break;

	case EN_LOG_TYPE_E:
		type = 'E';
		fprintf (stdout, THM_TEXT_UNDER_LINE);
		fprintf (stdout, THM_TEXT_BOLD_TYPE);
		fprintf (stdout, THM_TEXT_RED);
		break;

	case EN_LOG_TYPE_PE:
		type = 'E';
		fprintf (stdout, THM_TEXT_REVERSE);
		fprintf (stdout, THM_TEXT_BOLD_TYPE);
		fprintf (stdout, THM_TEXT_MAGENTA);
		strerror_r(errno, szPerror, sizeof (szPerror));
		break;

	default:
		type = 'I';
		break;
	}

	va_start (va, pszFormat);
	vsnprintf (szBufVa, sizeof(szBufVa), pszFormat, va);
	va_end (va);

	getSysTime (szTime, SYSTIME_STRING_SIZE);
	getThreadName (szThreadName, THREAD_NAME_STRING_SIZE);

	deleteLF (szBufVa);

	switch (enLogType) {
	case EN_LOG_TYPE_PE:
		fprintf (
			pFp,
			"[%s] %c %s  %s: %s   src=[%s %s()] line=[%d]\n",
			szThreadName,
			type,
			szTime,
			szBufVa,
			szPerror,
			pszFile,
			pszFunc,
			nLine
		);
		break;

	case EN_LOG_TYPE_I:
	case EN_LOG_TYPE_N:
	case EN_LOG_TYPE_W:
	case EN_LOG_TYPE_E:
	default:
		fprintf (
			pFp,
			"[%s] %c %s  %s   src=[%s %s()] line=[%d]\n",
			szThreadName,
			type,
			szTime,
			szBufVa,
			pszFile,
			pszFunc,
			nLine
		);
		break;
	}

	fprintf (stdout, THM_TEXT_ATTR_RESET);
	fflush (pFp);
}

/**
 * deleteLF
 * 改行コードLFを削除  (文字列末尾についてるものだけです)
 * CRLFの場合 CRも削除する
 */
void deleteLF (char *p)
{
	if (!p) {
		return;
	}

	if (strlen(p) == 0) {
		return;
	}

	if (*(p + (strlen(p) -1)) == '\n') {

		/* LF削除 */
		*(p + (strlen(p) -1)) = '\0';

		/* CRLFの場合 CRも削除 */
		if (*(p + (strlen(p) -1)) == '\r') {
			*(p + (strlen(p) -1)) = '\0';
		}
	}
}
