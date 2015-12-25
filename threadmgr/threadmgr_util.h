#ifndef _THREADMGR_UTIL_H_
#define _THREADMGR_UTIL_H_


/*
 * Constant define
 */
#define THM_TEXT_ATTR_RESET			"\x1B[0m"
#define THM_TEXT_BOLD_TYPE			"\x1B[1m"
#define THM_TEXT_UNDER_LINE			"\x1B[4m"
#define THM_TEXT_REVERSE			"\x1B[7m"
#define THM_TEXT_BLACK				"\x1B[30m"
#define THM_TEXT_RED				"\x1B[31m"
#define THM_TEXT_GREEN				"\x1B[32m"
#define THM_TEXT_YELLOW				"\x1B[33m"
#define THM_TEXT_BLUE				"\x1B[34m"
#define THM_TEXT_MAGENTA			"\x1B[35m"
#define THM_TEXT_CYAN				"\x1B[36m"
#define THM_TEXT_WHITE				"\x1B[37m"
#define THM_TEXT_STANDARD_COLOR		"\x1B[39m"


/*
 * Type define
 */
typedef enum {
	EN_LOG_TYPE_I,		// information
	EN_LOG_TYPE_N,		// notice
	EN_LOG_TYPE_W,		// warning
	EN_LOG_TYPE_E,		// error
	EN_LOG_TYPE_PE,		// perror
} EN_LOG_TYPE;


/*
 * log macro
 */
/* --- Information --- */
#if 0
#if defined (_THM_LOG_OFF)
#define THM_LOG_I(fmt, ...) {}
#else
#define THM_LOG_I(fmt, ...) {\
	fprintf (stdout, "[");\
	putsThreadName();\
	fprintf (stdout, "] ");\
	putsSysTime();\
	fprintf (stdout, " I ");\
	fprintf (stdout, "[%s,%s(),%d] ", __FILE__, __func__, __LINE__);\
	fprintf (stdout, fmt, ##__VA_ARGS__);\
	fflush (stdout);\
}
#endif
#endif

#define THM_LOG_I(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_I, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}

/* --- Notice --- */
#if 0
#if defined (_THM_LOG_OFF)
#define THM_LOG_N(fmt, ...) {}
#else
#define THM_LOG_N(fmt, ...) {\
	fprintf (stdout, THM_TEXT_GREEN);\
	fprintf (stdout, "[");\
	putsThreadName();\
	fprintf (stdout, "] ");\
	putsSysTime();\
	fprintf (stdout, " N ");\
	fprintf (stdout, "[%s,%s(),%d] ", __FILE__, __func__, __LINE__);\
	fprintf (stdout, fmt, ##__VA_ARGS__);\
	fprintf (stdout, THM_TEXT_ATTR_RESET);\
	fflush (stdout);\
}
#endif
#endif

#define THM_LOG_N(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_N, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}

/* --- Warning --- */
#if 0
#if defined (_THM_LOG_OFF)
#define THM_LOG_W(fmt, ...) {}
#else
#define THM_LOG_W(fmt, ...) {\
	fprintf (stdout, THM_TEXT_BOLD_TYPE);\
	fprintf (stdout, THM_TEXT_YELLOW);\
	fprintf (stdout, "[");\
	putsThreadName();\
	fprintf (stdout, "] ");\
	putsSysTime();\
	fprintf (stdout, " W ");\
	fprintf (stdout, "[%s,%s(),%d] ", __FILE__, __func__, __LINE__);\
	fprintf (stdout, fmt, ##__VA_ARGS__);\
	fprintf (stdout, THM_TEXT_ATTR_RESET);\
	fflush (stdout);\
}
#endif
#endif

#define THM_LOG_W(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_W, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}

/* --- Error --- */
#if 0
#define THM_LOG_E(fmt, ...) {\
	fprintf (stdout, THM_TEXT_UNDER_LINE);\
	fprintf (stdout, THM_TEXT_BOLD_TYPE);\
	fprintf (stdout, THM_TEXT_RED);\
	fprintf (stdout, "[");\
	putsThreadName();\
	fprintf (stdout, "] ");\
	putsSysTime();\
	fprintf (stdout, " E ");\
	fprintf (stdout, "[%s,%s(),%d] ", __FILE__, __func__, __LINE__);\
	fprintf (stdout, fmt, ##__VA_ARGS__);\
	fprintf (stdout, THM_TEXT_ATTR_RESET);\
	fflush (stdout);\
}
#endif

#define THM_LOG_E(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_E, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}

/* --- perror --- */
#if 0
#define THM_PERROR(fmt) {\
	fprintf (stdout, THM_TEXT_REVERSE);\
	fprintf (stdout, THM_TEXT_BOLD_TYPE);\
	fprintf (stdout, THM_TEXT_MAGENTA);\
	fprintf (stdout, "[");\
	putsThreadName();\
	fprintf (stdout, "] ");\
	putsSysTime();\
	fprintf (stdout, " E ");\
	fprintf (stdout, "[%s,%s(),%d] ", __FILE__, __func__, __LINE__);\
	char thm_perror_s[ 32 ] = {0};\
	fprintf (stdout, "%s: %s\n", fmt, strerror_r(errno, thm_perror_s, sizeof (thm_perror_s)));\
	fprintf (stdout, THM_TEXT_ATTR_RESET);\
	fflush (stdout);\
}
#endif

#define THM_PERROR(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_PE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}


/*
 * External
 */
extern void putsSysTime (void);
extern void putsThreadName (void);
extern void getTimeOfDay (struct timeval *p);
extern void putsLog (
	FILE *pFp,
	EN_LOG_TYPE enLogType,
	const char *pszFile,
	const char *pszFunc,
	int nLine,
	const char *pszFormat,
	...
);
void deleteLF (char *p);

#endif
