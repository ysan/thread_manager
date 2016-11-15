/*
 * 簡易スレッドマネージャ
 */
//TODO request timeoutの無限待ち
//TODO enableOverwrite
//TODO segv でbacktrace
//TODO commander
//TODO reqId類の配列は THREAD_IDX_MAX +1でもつのがわかりにくい


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
#include <signal.h>
#include <semaphore.h>

#include "threadmgr.h"
#include "threadmgr_if.h"
#include "threadmgr_util.h"


/*
 * Constant define
 */
#define SYS_RETURN_NORMAL				(0)

#define THREAD_NAME_SIZE				(16)
#define BASE_THREAD_NAME				"THMbase"
#define SIGWAIT_THREAD_NAME				"THMsigwait"

#define THREAD_IDX_MAX					(0x20) // 32  uint8の為 255以下の値で THREAD_IDX_BLANKより小さい値
#define THREAD_IDX_BLANK				(0x80) // 128 uint8の為 255以下の値
#define THREAD_IDX_EXTERNAL				THREAD_IDX_MAX // reqId類の配列は THREAD_IDX_MAX +1で確保されます

#define SEQ_IDX_MAX						(0x40) // 64  uint8の為 255以下の値で SEQ_IDX_BLANKより小さい値
#define SEQ_IDX_BLANK					(0x80) // 128 uint8の為 255以下の値

#define QUE_BASE_NUM					(64)
#define QUE_WORKER_MIN					(5)
#define QUE_WORKER_MAX					(100)

#define REQUEST_ID_MAX					(0x40) // REQUEST_ID_BLANK 以下の値で 2のべき乗の値
#define REQUEST_ID_BLANK				(0x80)
//TODO debug
//#define REQUEST_TIMEOUT_SEC			(300) // 5分
#define REQUEST_TIMEOUT_SEC				(30)

#define MSG_SIZE						(0x80) // 128

#define NOTIFY_CLIENT_ID_MAX			(0x40) // 64  1threadごとnotifyを登録できるクライアント数
#define NOTIFY_CLIENT_ID_BLANK			(0x80) // 128

#define SECT_ID_MAX						(0x40) // 64  1sequenceあたりsection分割可能な最大数
#define SECT_ID_INIT					(0)

#define SEQ_TIMEOUT_BLANK				(0)
#define SEQ_TIMEOUT_MAX					(0x05265C00) // 24時間

#define BASE_THREAD_LOOP_TIMEOUT_SEC	(10)



/*
 * Type define
 */
typedef enum {
	EN_STATE_INIT = 0,
	EN_STATE_READY,
	EN_STATE_BUSY,
	EN_STATE_WAIT_REPLY, // for requestSync
	EN_STATE_MAX
} EN_STATE;

typedef enum {
	EN_MONI_TYPE_INIT = 0,
	EN_MONI_TYPE_DEBUG,

} EN_MONI_TYPE;

typedef enum {
	EN_QUE_TYPE_INIT = 0,
	EN_QUE_TYPE_REQUEST,
	EN_QUE_TYPE_REPLY,
	EN_QUE_TYPE_NOTIFY,
	EN_QUE_TYPE_SEQ_TIMEOUT,
	EN_QUE_TYPE_REQ_TIMEOUT,
	EN_QUE_TYPE_MAX
} EN_QUE_TYPE;

typedef enum {
	EN_TIMEOUT_STATE_INIT = 0,
	EN_TIMEOUT_STATE_MEAS,
	EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT,
	EN_TIMEOUT_STATE_PASSED,
	EN_TIMEOUT_STATE_MAX,
} EN_TIMEOUT_STATE;

typedef enum {
	EN_NEAREST_TIMEOUT_NONE = 0,
	EN_NEAREST_TIMEOUT_REQ,
	EN_NEAREST_TIMEOUT_SEQ,
} EN_NEAREST_TIMEOUT;


typedef struct que_base {
	EN_MONI_TYPE enMoniType;
	bool isUsed;
} ST_QUE_BASE;

typedef struct que_worker {
	uint8_t nDestThreadIdx;
	uint8_t nDestSeqIdx;

	/* context */
	bool isValidSrcInfo;
	uint8_t nSrcThreadIdx;
	uint8_t nSrcSeqIdx;

	EN_QUE_TYPE enQueType;

	/* for Request */
	uint32_t nReqId;

	/* for Reply */
	EN_THM_RSLT enRslt;

	/* for Notify */
	uint8_t nClientId;

	/* message */
	struct {
		uint8_t szMsg [MSG_SIZE];
		bool isUsed;
	} msg;

	bool isUsed;
	bool isDrop;

} ST_QUE_WORKER;

typedef struct context {
	bool isValid;
	uint8_t nThreadIdx;
	uint8_t nSeqIdx;
} ST_CONTEXT;

/* 登録したシーケンスごとの情報 */
typedef struct seq_info {
	uint8_t nSeqIdx;

	/* 以下可変情報 */

	uint8_t nSectId;
	EN_THM_ACT enAct;
#ifndef _MULTI_REQUESTING
	uint32_t nReqId; // シーケンス中にrequestしたときのreqId  replyが返ってきたとき照合する
#endif
	ST_QUE_WORKER stSeqInitQueWorker; // このシーケンスを開始させたrequestキューを保存します

	/* Seqタイムアウト情報 */
	struct {
		EN_TIMEOUT_STATE enState;
		uint32_t nVal; // unit:mS
		struct timespec stTime;
	} timeout;

} ST_SEQ_INFO;

/* スレッド内部情報 (per thread) */
typedef struct inner_info {
	uint8_t nThreadIdx;
	char *pszName;
	pthread_t nPthreadId;
	uint8_t nQueWorkerNum;
	ST_QUE_WORKER *pstQueWorker; // nQueWorkerNum数分の配列をさします
	uint8_t nSeqNum;
	ST_SEQ_INFO *pstSeqInfo; // nSeqNum数分の配列をさします

	/* 以下可変情報 */

	EN_STATE enState;
	ST_QUE_WORKER stNowExecQueWorker;

} ST_INNER_INFO;

typedef struct sync_reply_info {
	uint32_t nReqId;
	EN_THM_RSLT enRslt;

	/* message */
	struct {
		uint8_t szMsg [MSG_SIZE];
		bool isUsed;
	} msg;

	bool isReplyAlready;
	bool isUsed;

} ST_SYNC_REPLY_INFO;

typedef struct request_id_info {
	uint32_t nId;

	uint8_t nSrcThreadIdx;
	uint8_t nSrcSeqIdx;

	/* Reqタイムアウト情報 */
	struct {
		EN_TIMEOUT_STATE enState;
		struct timespec stTime;
	} timeout;

} ST_REQUEST_ID_INFO;

typedef struct notify_client_info {
	uint8_t nThreadIdx; // clientのthreadIdx
	bool isUsed;
} ST_NOTIFY_CLIENT_INFO;

typedef struct external_control_info {
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	pthread_t nPthreadId; // key
	uint32_t nReqId;

	uint8_t szMsgEntity [MSG_SIZE];
	ST_THM_SRC_INFO stThmSrcInfo;

	bool isReplyAlready;

	struct external_control_info *pNext;

} ST_EXTERNAL_CONTROL_INFO;


/*
 * Variables
 */
static ST_THM_REG_TBL *gpstThmRegTbl [THREAD_IDX_MAX];
static ST_INNER_INFO gstInnerInfo [THREAD_IDX_MAX];

static uint8_t gnTotalWorkerThread;

static ST_QUE_BASE gstQueBase [QUE_BASE_NUM];

static uint32_t gnRequestIdInd [THREAD_IDX_MAX +1]; // 添え字 +1 は外部のスレッドからのリクエスト用
static ST_REQUEST_ID_INFO gstRequestIdInfo [THREAD_IDX_MAX +1][REQUEST_ID_MAX]; // 添え字 +1 は外部のスレッドからのリクエスト用

static ST_SYNC_REPLY_INFO gstSyncReplyInfo [THREAD_IDX_MAX];
static ST_THM_SRC_INFO gstThmSrcInfo [THREAD_IDX_MAX];

static ST_NOTIFY_CLIENT_INFO gstNotifyClientInfo [THREAD_IDX_MAX][NOTIFY_CLIENT_ID_MAX];

static ST_EXTERNAL_CONTROL_INFO *gpstExtInfoListTop;
static ST_EXTERNAL_CONTROL_INFO *gpstExtInfoListBtm;

static pthread_mutex_t gMutexBase;
static pthread_cond_t gCondBase;
static pthread_mutex_t gMutexWorker [THREAD_IDX_MAX];
static pthread_cond_t gCondWorker [THREAD_IDX_MAX];
static pthread_mutex_t gMutexSyncReply [THREAD_IDX_MAX];
static pthread_cond_t gCondSyncReply [THREAD_IDX_MAX];
static pthread_mutex_t gMutexOpeQueBase;
static pthread_mutex_t gMutexOpeQueWorker [THREAD_IDX_MAX];
static pthread_mutex_t gMutexOpeRequestId [THREAD_IDX_MAX +1]; // 添え字 +1 は外部のスレッドからのリクエスト用
static pthread_mutex_t gMutexNotifyClientInfo [THREAD_IDX_MAX];
static pthread_mutex_t gMutexOpeExtInfoList;

static sigset_t gSigset; // プロセス全体のシグナルセット
static sem_t gSem;

static bool gIsLogEnable = false;

static const char *gpszQueType [EN_QUE_TYPE_MAX] = {
	// for debug log
	"INIT",
	"REQ",
	"REPLY",
	"NOTIFY",
	"SEQ_TIMEOUT",
	"REQ_TIMEOUT",
};
static const char *gpszRslt [EN_THM_RSLT_MAX] = {
	// for debug log
	"IGNORE",
	"SUCCESS",
	"ERROR",
	"TIMEOUT",
};



/*
 * Prototypes
 */
bool setup (const ST_THM_REG_TBL *pTbl, uint8_t nTblMax); // extern
static void init (void);
static void initQue (void);
static void initCondMutex (void);
static void setupSignal (void);
static void setupSem (void);
static void postSem (void);
static void waitSem (void);
static uint8_t getTotalWorkerThreadNum (void);
static void setTotalWorkerThreadNum (uint8_t n);
static bool registerThreadMgrTbl (const ST_THM_REG_TBL *pTbl, uint8_t nTblMax);
static void setAllState (EN_STATE enState);
static EN_STATE getState (uint8_t nThreadIdx);
static void setState (uint8_t nThreadIdx, EN_STATE enState);
static void setThreadName (char *p);
static void checkWorkerThread (void);
static bool createBaseThread (void);
static bool createWorkerThread (uint8_t nThreadIdx);
static bool createAllThread (void);
static bool enQueBase (EN_MONI_TYPE enType);
static ST_QUE_BASE deQueBase (bool isGetOut);
static void clearQueBase (ST_QUE_BASE *p);
static bool enQueWorker (
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	EN_QUE_TYPE enQueType,
	ST_CONTEXT *pstContext,
	uint32_t nReqId,
	EN_THM_RSLT enRslt,
	uint8_t nClientId,
	uint8_t *pszMsg
);
static void dumpQueWorker (uint8_t nThreadIdx);
static void dumpQueAllThread (void);
//static ST_QUE_WORKER deQueWorker (uint8_t nThreadIdx, bool isGetOut);
static ST_QUE_WORKER check2deQueWorker (uint8_t nThreadIdx, bool isGetOut);
static void clearQueWorker (ST_QUE_WORKER *p);
static void *baseThread (void *pArg);
static void *sigwaitThread (void *pArg);
static void checkWaitWorkerThread (ST_INNER_INFO *pstInnerInfo);
static void *workerThread (void *pArg);
static void clearThmIf (ST_THM_IF *pIf);
static void clearThmSrcInfo (ST_THM_SRC_INFO *p);
static bool requestBaseThread (EN_MONI_TYPE enType);
static bool requestInner (
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	uint32_t nReqId,
	ST_CONTEXT *pstContext,
	uint8_t *pszMsg
);
bool requestSync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg); // extern
bool requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg, uint32_t *pnReqId); // extern
static bool replyInner (
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	uint32_t nReqId,
	ST_CONTEXT *pstContext,
	EN_THM_RSLT enRslt,
	uint8_t *pszMsg,
	bool isSync
);
static bool replyOuter (
	uint32_t nReqId,
	ST_CONTEXT *pstContext,
	EN_THM_RSLT enRslt,
	uint8_t *pszMsg
);
static bool reply (EN_THM_RSLT enRslt, uint8_t *pszMsg);
static ST_CONTEXT getContext (void);
static void clearContext (ST_CONTEXT *p);
static uint32_t getRequestId (uint8_t nThreadIdx, uint8_t nSeqIdx);
static void dumpRequestIdInfo (void);
static EN_TIMEOUT_STATE getReqTimeoutState (uint8_t nThreadIdx, uint32_t nReqId);
static ST_REQUEST_ID_INFO *getRequestIdInfo (uint8_t nThreadIdx, uint32_t nReqId);
static void enableReqTimeout (uint8_t nThreadIdx, uint32_t nReqId);
static void checkReqTimeout (uint8_t nThreadIdx);
static bool isReqTimeoutFromRequestId (uint8_t nThreadIdx, uint32_t nReqId);
static bool enQueReqTimeout (uint8_t nThreadIdx, uint32_t nReqId);
static ST_REQUEST_ID_INFO *searchNearestReqTimeout (uint8_t nThreadIdx);
static void releaseRequestId (uint8_t nThreadIdx, uint32_t nReqId);
static void clearRequestIdInfo (ST_REQUEST_ID_INFO *p);
static bool isSyncReplyFromRequestId (uint8_t nThreadIdx, uint32_t nReqId);
static bool setRequestIdSyncReply (uint8_t nThreadIdx, uint32_t nReqId);
static ST_SYNC_REPLY_INFO *getSyncReplyInfo (uint8_t nThreadIdx);
static bool cashSyncReplyInfo (uint8_t nThreadIdx, EN_THM_RSLT enRslt, uint8_t *pszMsg);
static void setReplyAlreadySyncReplyInfo (uint8_t nThreadIdx);
static void clearSyncReplyInfo (ST_SYNC_REPLY_INFO *p);
static bool registerNotify (uint8_t *pnClientId);
static bool unregisterNotify (uint8_t nClientId);
static uint8_t setNotifyClientInfo (uint8_t nThreadIdx, uint8_t nClientThreadIdx);
static bool unsetNotifyClientInfo (uint8_t nThreadIdx, uint8_t nClientId);
static bool notifyInner (
	uint8_t nThreadIdx,
	uint8_t nClientId,
	ST_CONTEXT *pstContext,
	uint8_t *pszMsg
);
static bool notify (uint8_t nClientId, uint8_t *pszMsg);
static void clearNotifyClientInfo (ST_NOTIFY_CLIENT_INFO *p);
static void setSectId (uint8_t nSectId, EN_THM_ACT enAct);
static uint8_t getSectId (void);
static void setTimeout (uint32_t nTimeoutMsec);
static void setSeqTimeout (uint32_t nTimeoutMsec);
static void enableSeqTimeout (uint8_t nThreadIdx, uint8_t nSeqIdx);
static void checkSeqTimeout (uint8_t nThreadIdx);
static bool isSeqTimeoutFromSeqIdx (uint8_t nThreadIdx, uint8_t nSeqIdx);
static bool enQueSeqTimeout (uint8_t nThreadIdx, uint8_t nSeqIdx);
static ST_SEQ_INFO *searchNearestSeqTimeout (uint8_t nThreadIdx);
static void checkSeqTimeoutFromCondTimedwait (uint8_t nThreadIdx);
static void checkSeqTimeoutFromNotCondTimedwait (uint8_t nThreadIdx);
static void clearTimeout (void);
static void clearSeqTimeout (void);
static void clearSeqTimeoutInfo (uint8_t nThreadIdx, uint8_t nSeqIdx);
static ST_SEQ_INFO *getSeqInfo (uint8_t nThreadIdx, uint8_t nSeqIdx);
static void clearSeqInfo (ST_SEQ_INFO *p);
static EN_NEAREST_TIMEOUT searchNearestTimeout (
	uint8_t nThreadIdx,
	ST_REQUEST_ID_INFO **pstRequestIdInfo,	// out
	ST_SEQ_INFO **pstSeqInfo		// out
);
static void addExtInfoList (ST_EXTERNAL_CONTROL_INFO *pstExtInfo);
static ST_EXTERNAL_CONTROL_INFO *searchExtInfoList (pthread_t key);
static ST_EXTERNAL_CONTROL_INFO *searchExtInfoListFromRequestId (uint32_t nReqId);
static void deleteExtInfoList (pthread_t key);
static void dumpExtInfoList (void);
bool createExternalCp (void); // extern
void destroyExternalCp (void); // extern
ST_THM_SRC_INFO *receiveExternal (void); // extern
static void clearExternalControlInfo (ST_EXTERNAL_CONTROL_INFO *p);
void finalize (void); // extern
static bool isLogEnable (void);
static void enableLog (void);
static void disableLog (void);

/*
 * inner log macro
 */
#define THM_INNER_FORCE_LOG_I(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_I, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#define THM_INNER_FORCE_LOG_N(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_N, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#define THM_INNER_FORCE_LOG_W(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_W, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}

#define THM_INNER_LOG_I(fmt, ...) {\
	if (isLogEnable()) {\
		putsLog (stdout, EN_LOG_TYPE_I, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
	}\
}
#define THM_INNER_LOG_N(fmt, ...) {\
	if (isLogEnable()) {\
		putsLog (stdout, EN_LOG_TYPE_N, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
	}\
}
#define THM_INNER_LOG_W(fmt, ...) {\
	if (isLogEnable()) {\
		putsLog (stdout, EN_LOG_TYPE_W, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
	}\
}
#define THM_INNER_LOG_E(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_E, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}
#define THM_INNER_PERROR(fmt, ...) {\
	putsLog (stdout, EN_LOG_TYPE_PE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
}


/**
 * threadmgrの初期化とセットアップ
 * 最初にこれを呼びます
 */
bool setup (const ST_THM_REG_TBL *pTbl, uint8_t nTblMax)
{
	if ((!pTbl) || (nTblMax == 0)) {
		THM_INNER_LOG_E( "invalid argument.\n" );
		return false;
	}

	init();

	setupSem ();

	/* 以降の生成されたスレッドはこのシグナルマスクを継承する */
	setupSignal();

	if (!registerThreadMgrTbl (pTbl, nTblMax)) {
		THM_INNER_LOG_E ("registerThreadMgrTbl() is failure.\n");
		return false;
	}

	if (getTotalWorkerThreadNum() == 0) {
		THM_INNER_LOG_E ("total thread is 0.\n");
		return false;
	}


	if (!createAllThread()) {
		THM_INNER_LOG_E ("createAllThread() is failure.\n");
		return false;
	}

	/* 全スレッド立ち上がるまで待つ */
	waitSem ();
	THM_INNER_FORCE_LOG_I ("----- setup complete. -----\n");

	return true;
}

/**
 * 初期化処理
 * 
 */
static void init (void)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		gpstThmRegTbl [i] = NULL;
	}

	/* init innerInfo */
//TODO gstInnerInfo詳細に初期化すべき
	memset (gstInnerInfo, 0x00, sizeof(gstInnerInfo));
	setAllState (EN_STATE_INIT);

	setTotalWorkerThreadNum (0);


	/* init requestIdInfo */
	for (i = 0; i < THREAD_IDX_MAX +1; i ++) { // 添え字 +1 は外部のスレッドからのリクエスト用
		gnRequestIdInd [i] = 0;
		for (j = 0; j < REQUEST_ID_MAX; j ++) {
			clearRequestIdInfo (&gstRequestIdInfo [i][j]);
		}
	}

	/* init syncReplyInfo  */
	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		clearSyncReplyInfo (&gstSyncReplyInfo [i]);
	}

	/* init thmSrcInfo */
	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		clearThmSrcInfo (&gstThmSrcInfo [i]);
	}

	/* init notifyInfo */
	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		for (j = 0; j < NOTIFY_CLIENT_ID_MAX; j ++) {
			clearNotifyClientInfo (&gstNotifyClientInfo [i][j]);
		}
	}

	gpstExtInfoListTop = NULL;
	gpstExtInfoListBtm = NULL;


	initQue();
	initCondMutex();
}

/**
 * 初期化
 * キュー
 */
static void initQue (void)
{
	int i = 0;

	for (i = 0; i < QUE_BASE_NUM; i ++) {
		clearQueBase (&gstQueBase [i]);
	}
}

/**
 * 初期化
 * cond & mutex
 */
static void initCondMutex (void)
{
	int i = 0;

	pthread_mutex_init (&gMutexBase, NULL);
	pthread_cond_init (&gCondBase, NULL);

//TODO 再帰mutexを使用します
	pthread_mutexattr_t attrMutexWorker;
	pthread_mutexattr_init (&attrMutexWorker);
	pthread_mutexattr_settype (&attrMutexWorker, PTHREAD_MUTEX_RECURSIVE_NP);
	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		pthread_mutex_init (&gMutexWorker [i], &attrMutexWorker);
		pthread_cond_init (&gCondWorker [i], NULL);
	}

	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		pthread_mutex_init (&gMutexSyncReply [i], NULL);
		pthread_cond_init (&gCondSyncReply [i], NULL);
	}

	pthread_mutex_init (&gMutexOpeQueBase, NULL);
	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		pthread_mutex_init (&gMutexOpeQueWorker [i], NULL);
	}

//TODO 再帰mutexを使用します
	pthread_mutexattr_t attrMutexOpeRequestId;
	pthread_mutexattr_init (&attrMutexOpeRequestId);
	pthread_mutexattr_settype (&attrMutexOpeRequestId, PTHREAD_MUTEX_RECURSIVE_NP);
	for (i = 0; i < THREAD_IDX_MAX +1; i ++) { // 添え字 +1 は外部のスレッドからのリクエスト用
		pthread_mutex_init (&gMutexOpeRequestId [i], &attrMutexOpeRequestId);
	}

	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		pthread_mutex_init (&gMutexNotifyClientInfo [i], NULL);
	}

	pthread_mutex_init (&gMutexOpeExtInfoList, NULL);
}

/**
 * setupSignal
 * プロセス全体のシグナル設定
 */
static void setupSignal (void)
{
	sigemptyset (&gSigset);

//TODO SIGQUIT (ctrl + \)
	sigaddset (&gSigset, SIGQUIT);
	sigprocmask (SIG_BLOCK, &gSigset, NULL);
}

/**
 * setupSem
 */
static void setupSem (void)
{
	sem_init (&gSem, 0, 0); 
}

/**
 * postSem
 */
static void postSem (void)
{
	sem_post (&gSem);
}

/**
 * waitSem
 */
static void waitSem (void)
{
	uint8_t i = 0;

	for (i = 0; i < getTotalWorkerThreadNum(); i ++) {
		sem_wait (&gSem);
		THM_INNER_LOG_I ("sem_wait return\n");
	}

	/* baseThreadとsigwaitThreadの分 */
	sem_wait (&gSem);
	THM_INNER_LOG_I ("sem_wait return\n");
	sem_wait (&gSem);
	THM_INNER_LOG_I ("sem_wait return\n");
}

/**
 * getTotalWorkerThreadNum
 */
static uint8_t getTotalWorkerThreadNum (void)
{
	return gnTotalWorkerThread;
}

/**
 * setTotalWorkerThreadNum
 */
static void setTotalWorkerThreadNum (uint8_t n)
{
	gnTotalWorkerThread = n;
}

/**
 * registerThreadMgrTbl
 * スレッドテーブルを登録する
 */
static bool registerThreadMgrTbl (const ST_THM_REG_TBL *pTbl, uint8_t nTblMax)
{
	if ((!pTbl) || (nTblMax == 0)) {
		THM_INNER_LOG_E ("invalid argument.\n");
		return false;
	}

	int i = 0;

	while (i < nTblMax) {
		if (i == THREAD_IDX_MAX) {
			// over flow
			THM_INNER_LOG_E ("thread idx is over. (inner thread table)\n");
			return false;
		}

		if ((((ST_THM_REG_TBL*)pTbl)->nQueNum < QUE_WORKER_MIN) && (((ST_THM_REG_TBL*)pTbl)->nQueNum > QUE_WORKER_MAX)) {
			THM_INNER_LOG_E ("que num is invalid. (inner thread table)\n");
			return false;
		}

		if ((((ST_THM_REG_TBL*)pTbl)->nSeqNum <= 0) && (((ST_THM_REG_TBL*)pTbl)->nSeqNum > SEQ_IDX_MAX)) {
			THM_INNER_LOG_E ("func idx is invalid. (inner thread table)\n");
			return false;
		}

		gpstThmRegTbl [i] = (ST_THM_REG_TBL*)pTbl;
		pTbl ++;
		i ++;
	}

	// set inner total
	setTotalWorkerThreadNum (i);
	THM_INNER_LOG_I ("gnTotalWorkerThread=[%d]\n", getTotalWorkerThreadNum());

	return true;
}

/**
 * setAllState
 */
static void setAllState (EN_STATE enState)
{
	uint8_t i = 0;

	for (i = 0; i < THREAD_IDX_MAX; i ++) {
		setState (i, enState);
	}
}

/**
 * getState
 */
static EN_STATE getState (uint8_t nThreadIdx)
{
	return gstInnerInfo [nThreadIdx].enState;
}

/**
 * setState
 */
static void setState (uint8_t nThreadIdx, EN_STATE enState)
{
	gstInnerInfo [nThreadIdx].enState = enState;
}

/**
 * setThreadName
 * thread名をセット  pthread_setname_np
 */
void setThreadName (char *p)
{
	if (!p) {
		return;
	}

	char szName [THREAD_NAME_SIZE];
	memset (szName, 0x00, sizeof(szName));
	strncpy (szName, p, sizeof(szName) -1);
	pthread_setname_np (pthread_self(), szName);
}

/**
 * checkWorkerThread
 * workerThreadの確認
 */
static void checkWorkerThread (void)
{
	uint8_t i = 0;
	int nRtn = 0;

	for (i = 0; i < getTotalWorkerThreadNum(); i ++) {

		/* フラグ値を確認 */
		if ((getState (i) != EN_STATE_READY) && (getState (i) != EN_STATE_BUSY) && (getState (i) != EN_STATE_WAIT_REPLY)) {
			/* 異常 */
			//TODO とりあえずログをだす
			THM_INNER_LOG_E ("[%s] EN_STATE flag is abnromal !!!\n", gstInnerInfo [i].pszName);
		}

		nRtn = pthread_kill (gstInnerInfo [i].nPthreadId, 0);
		if (nRtn != SYS_RETURN_NORMAL) {
			/* 異常 */
			//TODO とりあえずログをだす
			THM_INNER_LOG_E ("[%s] pthread_kill(0) abnromal !!!\n", gstInnerInfo [i].pszName);
		}
	}
}

/**
 * createBaseThread
 * baseThread生成
 */
static bool createBaseThread (void)
{
	pthread_t nPthreadId;
	pthread_attr_t attr;

	if (pthread_attr_init (&attr) != 0) {
		THM_PERROR ("pthread_attr_init()");
		return false;
	}

//TODO スタックサイズ/スタックガード設定するならここで
//pthread_attr_setstacksize
//pthread_attr_setguardsize

//TODO スケジューリングポリシー設定するならここで
//ポリシー に指定できる値は SCHED_FIFO, SCHED_RR, SCHED_OTHER
//pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED) これを事前にセットして
//pthread_attr_setschedpolicy

//TODO プライオリティ設定 ポリシーRRとFIFOだったら設定
//pthread_attr_setschedparam

	if (pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED) != 0) {
		THM_PERROR ("pthread_attr_setdetachstate()");
		return false;
	}

	if (pthread_create (&nPthreadId, &attr, baseThread, (void*)NULL) != 0) {
		THM_PERROR ("pthread_create()");
		return false;
	}

//TODO 暫定位置
	if (pthread_create (&nPthreadId, &attr, sigwaitThread, (void*)NULL) != 0) {
		THM_PERROR ("pthread_create()");
		return false;
	}

	return true;
}

/**
 * createWorkerThread
 * workerThread生成
 */
static bool createWorkerThread (uint8_t nThreadIdx)
{
	pthread_t nPthreadId;
	pthread_attr_t attr;


	/* set thread idx */
	gstInnerInfo [nThreadIdx].nThreadIdx = nThreadIdx;


	if (pthread_attr_init( &attr ) != 0) {
		THM_PERROR ("pthread_attr_init()");
		return false;
	}

//TODO スタックサイズ/スタックガード設定するならここで
//pthread_attr_setstacksize
//pthread_attr_setguardsize

//TODO スケジューリングポリシー設定するならここで
//ポリシー に指定できる値は SCHED_FIFO, SCHED_RR, SCHED_OTHER
//pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED) これを事前にセットして
//pthread_attr_setschedpolicy

//TODO プライオリティ設定 ポリシーRRとFIFOだったら設定
//pthread_attr_setschedparam

	if (pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED) != 0) {
		THM_PERROR ("pthread_attr_setdetachstate()");
		return false;
	}

	if (pthread_create (&nPthreadId, &attr, workerThread, (void*)&gstInnerInfo [nThreadIdx]) != 0) {
		THM_PERROR ("pthread_create()");
		return false;
	}

	return true;
}

/**
 * createAllThread
 * 全スレッド生成
 */
static bool createAllThread (void)
{
	uint8_t i = 0;

	if (!createBaseThread()) {
		return false;
	}

	for (i = 0; i < getTotalWorkerThreadNum(); i ++) {
		if (!createWorkerThread (i)) {
			return false;
		}
		THM_INNER_LOG_I ("create workerThread. thIdx:[%d]\n", i);
	}

	return true;
}

/**
 * enqueue (base)
 */
static bool enQueBase (EN_MONI_TYPE enType)
{
	int i = 0;

	pthread_mutex_lock (&gMutexOpeQueBase);

	for (i = 0; i < QUE_BASE_NUM; i ++) {

		/* 空きを探す */
		if (gstQueBase [i].isUsed == false) {
			gstQueBase [i].enMoniType = enType;
			gstQueBase [i].isUsed = true;
			break;
		}
	}

	pthread_mutex_unlock (&gMutexOpeQueBase);

	if (i == QUE_BASE_NUM) {
		/* 全部埋まってる */
		THM_INNER_LOG_E( "que is full. (base)\n" );
		return false;
	}

	return true;
}

/**
 * dequeue (base)
 */
static ST_QUE_BASE deQueBase (bool isGetOut)
{
	int i = 0;
	ST_QUE_BASE rtn;

//TODO いらないかも ここはbaseThreadで処理するから
	pthread_mutex_lock (&gMutexOpeQueBase);

	rtn = gstQueBase [0];

	if (isGetOut) {
		for (i = 0; i < QUE_BASE_NUM; i ++) {
			if (i < QUE_BASE_NUM-1) {
				memcpy (&gstQueBase [i], &gstQueBase [i+1], sizeof(ST_QUE_BASE));

			} else {
				/* 末尾 */
				gstQueBase [i].enMoniType = EN_MONI_TYPE_INIT;
				gstQueBase [i].isUsed = false;
			}
		}
	}

	pthread_mutex_unlock (&gMutexOpeQueBase);

	return rtn;
}

/**
 * baseキュークリア
 */
static void clearQueBase (ST_QUE_BASE *p)
{
	if (!p) {
		return;
	}

	p->enMoniType = EN_MONI_TYPE_INIT;
	p->isUsed = false;
}

/**
 * enqueue (worker)
 * 引数 nThreadIdx,nSeqIdxは宛先
 */
static bool enQueWorker (
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	EN_QUE_TYPE enQueType,
	ST_CONTEXT *pstContext,
	uint32_t nReqId,
	EN_THM_RSLT enRslt,
	uint8_t nClientId,
	uint8_t *pszMsg
)
{
	uint8_t i = 0;
	uint8_t nQueWorkerNum = gstInnerInfo [nThreadIdx].nQueWorkerNum;
	ST_QUE_WORKER *pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;

	/* 複数の人が同じthreadIdxのキューを操作するのをガード */
	pthread_mutex_lock (&gMutexOpeQueWorker [nThreadIdx]);

	for (i = 0; i < nQueWorkerNum; i ++) {

		/* 空きを探す */
		if (pstQueWorker->isUsed == false) {

			/* set */
			pstQueWorker->nDestThreadIdx = nThreadIdx;
			pstQueWorker->nDestSeqIdx = nSeqIdx;
			if (pstContext) {
				pstQueWorker->isValidSrcInfo = pstContext->isValid;
				pstQueWorker->nSrcThreadIdx = pstContext->nThreadIdx;
				pstQueWorker->nSrcSeqIdx = pstContext->nSeqIdx;
			}
			pstQueWorker->enQueType = enQueType;
			pstQueWorker->nReqId = nReqId;
			pstQueWorker->enRslt = enRslt;
			pstQueWorker->nClientId = nClientId;
			if (pszMsg) {
				memcpy (pstQueWorker->msg.szMsg, pszMsg, MSG_SIZE);
				pstQueWorker->msg.isUsed = true;
			}
			pstQueWorker->isUsed = true;
			break;
		}

		pstQueWorker ++;
	}

#if 1
	if (i < nQueWorkerNum) {
		pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;
		uint8_t j = 0;
		for (j = 0; j <= i; j ++) {
			THM_INNER_LOG_I (
				" %d: %s (%s %d-%d) -> %d-%d 0x%x %s 0x%x %s\n",
				j,
				gpszQueType [pstQueWorker->enQueType],
				pstQueWorker->isValidSrcInfo ? "T" : "F",
				pstQueWorker->nSrcThreadIdx,
				pstQueWorker->nSrcSeqIdx,
				pstQueWorker->nDestThreadIdx,
				pstQueWorker->nDestSeqIdx,
				pstQueWorker->nReqId,
				gpszRslt [pstQueWorker->enRslt],
				pstQueWorker->nClientId,
				pstQueWorker->isUsed ? "T" : "F"
			);
			pstQueWorker ++;
		}
	}
#else
	/* dump */
	pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;
	THM_INNER_LOG_I( "####  enQue dest[%s]  ####\n", gstInnerInfo [nThreadIdx].pszName );
	uint8_t j = 0;
	for (j = 0; j < nQueWorkerNum; j ++) {
		THM_INNER_LOG_I (
			" %d: %s (%s %d-%d) -> %d-%d 0x%x %s 0x%x %s\n",
			j,
			gpszQueType [pstQueWorker->enQueType],
			pstQueWorker->isValidSrcInfo ? "T" : "F",
			pstQueWorker->nSrcThreadIdx,
			pstQueWorker->nSrcSeqIdx,
			pstQueWorker->nDestThreadIdx,
			pstQueWorker->nDestSeqIdx,
			pstQueWorker->nReqId,
			gpszRslt [pstQueWorker->enRslt],
			pstQueWorker->nClientId,
			pstQueWorker->isUsed ? "T" : "F"
		);
		pstQueWorker ++;
	}
#endif

	pthread_mutex_unlock (&gMutexOpeQueWorker [nThreadIdx]);

	if (i == nQueWorkerNum) {
		/* 全部埋まってる */
		THM_INNER_LOG_E ("que is full. (worker) thIdx:[%d]\n", nThreadIdx);
		dumpQueWorker (nThreadIdx);
		return false;
	}

	return true;
}

/**
 * dumpQueWorker
 */
static void dumpQueWorker (uint8_t nThreadIdx)
{
	uint8_t i = 0;
	uint8_t nQueWorkerNum = gstInnerInfo [nThreadIdx].nQueWorkerNum;
	ST_QUE_WORKER *pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;

	/* lock */
	pthread_mutex_lock (&gMutexOpeQueWorker [nThreadIdx]);

	THM_INNER_FORCE_LOG_N ("####  dumpQue [%s]  ####\n", gstInnerInfo [nThreadIdx].pszName);
	for (i = 0; i < nQueWorkerNum; i ++) {
		THM_INNER_FORCE_LOG_N (
			" %d: %s (%s %d-%d) -> %d-%d 0x%x %s 0x%x %s\n",
			i,
			gpszQueType [pstQueWorker->enQueType],
			pstQueWorker->isValidSrcInfo ? "T" : "F",
			pstQueWorker->nSrcThreadIdx,
			pstQueWorker->nSrcSeqIdx,
			pstQueWorker->nDestThreadIdx,
			pstQueWorker->nDestSeqIdx,
			pstQueWorker->nReqId,
			gpszRslt [pstQueWorker->enRslt],
			pstQueWorker->nClientId,
			pstQueWorker->isUsed ? "T" : "F"
		);
		pstQueWorker ++;
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeQueWorker [nThreadIdx]);
}

/**
 * dumpQueAllThread
 */
static void dumpQueAllThread (void)
{
	uint8_t i = 0;
	for (i = 0; i < getTotalWorkerThreadNum(); i ++) {
		dumpQueWorker (i);
	}
}

#if 0
/**
 * dequeue (worker)
 */
static ST_QUE_WORKER deQueWorker (uint8_t nThreadIdx, bool isGetOut)
{
	uint8_t i = 0;
	uint8_t nQueWorkerNum = gstInnerInfo [nThreadIdx].nQueWorkerNum;
	ST_QUE_WORKER *pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;
	ST_QUE_WORKER rtn;

	clearQueWorker (&rtn);

	/* 複数の人が同じthreadIdxのキューを操作するのをガード */
//TODO いらないかも ここは個々のスレッドが処理するから
	pthread_mutex_lock (&gMutexOpeQueWorker [nThreadIdx]);

	memcpy (&rtn, pstQueWorker, sizeof (ST_QUE_WORKER));

	if (isGetOut) {
		for (i = 0; i < nQueWorkerNum; i ++) {
			if (i < nQueWorkerNum-1) {
				memcpy (
					pstQueWorker,
					pstQueWorker +1,
					sizeof (ST_QUE_WORKER)
				);

			} else {
				/* 末尾 */
				clearQueWorker (pstQueWorker);
			}

			pstQueWorker ++;
		}
	}

	pthread_mutex_unlock (&gMutexOpeQueWorker [nThreadIdx]);

	return rtn;
}
#endif

/**
 * dequeue (worker)
 * 実行できるキューの検索込のdequeue
 */
//TODO ここではisGetOut無くし チェックのみで deQueする関数は別にした方がいいのかもしれない
static ST_QUE_WORKER check2deQueWorker (uint8_t nThreadIdx, bool isGetOut)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t k = 0;
	uint8_t l = 0;
	uint8_t nQueWorkerNum = gstInnerInfo [nThreadIdx].nQueWorkerNum;
	ST_QUE_WORKER *pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;
	ST_QUE_WORKER rtn;
	uint8_t nSeqIdx = 0;
	EN_TIMEOUT_STATE enTimeoutState = EN_TIMEOUT_STATE_INIT;

	clearQueWorker (&rtn);


	/* 複数の人が同じthreadIdxのキューを操作するのをガード */
//TODO いらないかも ここは個々のスレッドが処理するから
	pthread_mutex_lock (&gMutexOpeQueWorker [nThreadIdx]);

	/* 新しいほうからキューを探す */
	for (i = 0; i < nQueWorkerNum; i ++) {

		if (pstQueWorker->isUsed) {

			if (pstQueWorker->enQueType == EN_QUE_TYPE_REQUEST) {
				/* * -------------------- REQUEST_QUE -------------------- * */ 

				nSeqIdx = pstQueWorker->nDestSeqIdx;
				if (getSeqInfo (nThreadIdx, nSeqIdx)->enAct == EN_THM_ACT_INIT) {
					/*
					 * 対象のシーケンスがEN_THM_ACT_INIT
					 * 実行してok
					 */
					memcpy (&rtn, pstQueWorker, sizeof(ST_QUE_WORKER));
					break;

				} else if (getSeqInfo (nThreadIdx, nSeqIdx)->enAct == EN_THM_ACT_WAIT) {
					/*
					 * 対象のシーケンスがEN_THM_ACT_WAIT シーケンスの途中
					 * 見送り
					 */
					THM_INNER_LOG_N ("enAct is EN_THM_ACT_WAIT @REQUEST_QUE  ---> through\n");

				} else {
					/* ありえない */
					THM_INNER_LOG_E ("BUG: enAct is [%d] @REQUEST_QUE\n", getSeqInfo(nThreadIdx, nSeqIdx)->enAct);
				}



			} else if (pstQueWorker->enQueType == EN_QUE_TYPE_REPLY) {
				/* * -------------------- REPLY_QUE -------------------- * */

				nSeqIdx = pstQueWorker->nDestSeqIdx;
				if (getSeqInfo (nThreadIdx, nSeqIdx)->enAct == EN_THM_ACT_INIT) {
					/* シーケンスによってはありえる */
					/* リプライ待たずに進むようなシーケンスとか... */
					THM_INNER_FORCE_LOG_N ("enAct is EN_THM_ACT_INIT(nSeqIdx=%d) @REPLY_QUE  ---> drop\n", nSeqIdx);

					/* この場合キューは引き取る */
					pstQueWorker->isDrop = true;
					releaseRequestId (nThreadIdx, pstQueWorker->nReqId);

				} else if (getSeqInfo (nThreadIdx, nSeqIdx)->enAct == EN_THM_ACT_WAIT) {
					/*
					 * 対象のシーケンスがEN_THM_ACT_WAIT シーケンスの途中
					 * requestIdを確認する
					 */
#ifndef _MULTI_REQUESTING
					if (pstQueWorker->nReqId == getSeqInfo (nThreadIdx,nSeqIdx)->nReqId) {
#else
					/* 複数requestの場合があるので requestIdInfoで照合する そのため ユーザがわでreqIdの判断が必要 */
					if (pstQueWorker->nReqId == getRequestIdInfo (nThreadIdx, pstQueWorker->nReqId)->nId) {
#endif
						/*
						 * requestIdが一致
						 * 実行してok
						 */
						memcpy (&rtn, pstQueWorker, sizeof(ST_QUE_WORKER));
						if (isGetOut) {
							/* dequeするときにrelease */
							releaseRequestId (nThreadIdx, pstQueWorker->nReqId);
						}
						break;

					} else {
						/* シーケンスによってはありえる */
						/* リプライ待たずに進むようなシーケンスとか... */
#ifndef _MULTI_REQUESTING
						THM_INNER_LOG_N (
							"enAct is EN_THM_ACT_WAIT  reqId unmatch:[%d:%d] @REPLY_QUE  ---> drop\n",
							pstQueWorker->nReqId,
							getSeqInfo (nThreadIdx, nSeqIdx)->nReqId
						);
#else
						THM_INNER_LOG_N ("enAct is EN_THM_ACT_WAIT  reqId unmatch @REPLY_QUE  ---> drop\n");
#endif
						/* この場合キューは引き取る */
						pstQueWorker->isDrop = true;
						releaseRequestId (nThreadIdx, pstQueWorker->nReqId);
					}

				} else {
					/* ありえない */
					THM_INNER_LOG_E ("BUG: enAct is [%d] @REPLY_QUE\n", getSeqInfo (nThreadIdx, nSeqIdx)->enAct);
				}



			} else if (pstQueWorker->enQueType == EN_QUE_TYPE_NOTIFY) {
				/* * -------------------- NOTIFY_QUE -------------------- * */

				/* そのまま実行してok */
				memcpy (&rtn, pstQueWorker, sizeof(ST_QUE_WORKER));
				break;



			} else if (pstQueWorker->enQueType == EN_QUE_TYPE_REQ_TIMEOUT) {
				/* * -------------------- REQ_TIMEOUT_QUE -------------------- * */

				/* reqIdのタイムアウト状態を確認する */
				enTimeoutState = getReqTimeoutState (nThreadIdx, pstQueWorker->nReqId);
				if (enTimeoutState == EN_TIMEOUT_STATE_PASSED) {

					nSeqIdx = pstQueWorker->nDestSeqIdx;
					if (getSeqInfo (nThreadIdx, nSeqIdx)->enAct == EN_THM_ACT_WAIT) {
						/*
						 * 対象のシーケンスがEN_THM_ACT_WAIT
						 * reqIdの一致を確認する
						 */
#ifndef _MULTI_REQUESTING
						if (pstQueWorker->nReqId == getSeqInfo (nThreadIdx, nSeqIdx)->nReqId) {
#else
						/* 複数requestの場合があるので requestIdInfoで照合する そのため ユーザがわでreqIdの判断が必要 */
						if (pstQueWorker->nReqId == getRequestIdInfo (nThreadIdx, pstQueWorker->nReqId)->nId) {
#endif
							/*
							 * reqIdが一致した
							 * 実行してok
							 */
							memcpy (&rtn, pstQueWorker, sizeof(ST_QUE_WORKER));
							if (isGetOut) {
								/* dequeするときにrelease */
								releaseRequestId (nThreadIdx, pstQueWorker->nReqId);
							}
							break;

						} else {
							/* シーケンスによってはありえる */
							/* リプライ待たずに進むようなシーケンスとか... */
#ifndef _MULTI_REQUESTING
							THM_INNER_LOG_N (
								"enAct is EN_THM_ACT_WAIT  reqId unmatch:[%d:%d] @REQ_TIMEOUT_QUE  ---> drop\n",
								pstQueWorker->nReqId,
								getSeqInfo (nThreadIdx, nSeqIdx)->nReqId
							);
#else
							THM_INNER_LOG_N ("enAct is EN_THM_ACT_WAIT  reqId unmatch:[%d:%d] @REQ_TIMEOUT_QUE  ---> drop\n");
#endif

							/* この場合キューは引き取る */
							pstQueWorker->isDrop = true;
							releaseRequestId (nThreadIdx, pstQueWorker->nReqId);
						}

					} else {
						/* シーケンスによってはありえる */
						/* リプライ待たずに進むようなシーケンスとか... */
						THM_INNER_FORCE_LOG_N ("enAct is not EN_THM_ACT_WAIT  @REQ_TIMEOUT_QUE  ---> drop\n");

						/* この場合キューは引き取る */
						pstQueWorker->isDrop = true;
						releaseRequestId (nThreadIdx, pstQueWorker->nReqId);
					}

				} else {
					/* ありえないはず */
					THM_INNER_LOG_E ("BUG: reqIdInfo.timeout.enState is unexpected [%d] @REQ_TIMEOUT_QUE\n", enTimeoutState);
				}



			} else if (pstQueWorker->enQueType == EN_QUE_TYPE_SEQ_TIMEOUT) {
				/* * -------------------- SEQ_TIMEOUT_QUE -------------------- * */

				nSeqIdx = pstQueWorker->nDestSeqIdx;
				if (getSeqInfo (nThreadIdx, nSeqIdx)->enAct == EN_THM_ACT_WAIT) {
					/*
					 * 対象のシーケンスがEN_THM_ACT_TIMEOUT
					 * seqInfoのタイムアウト状態を確認する
					 */
					if (getSeqInfo (nThreadIdx, nSeqIdx)->timeout.enState == EN_TIMEOUT_STATE_PASSED) {
						/*
						 * 既にタイムアウトしている
						 * 実行してok
						 */
						memcpy (&rtn, pstQueWorker, sizeof(ST_QUE_WORKER));
						break;

					} else {
						/* ありえるのか? ありえないはず... */
						THM_INNER_LOG_E (
							"BUG: enAct is EN_THM_ACT_WAIT  unexpect timeout.enState:[%d]  @SEQ_TIMEOUT_QUE\n",
							getSeqInfo(nThreadIdx, nSeqIdx)->timeout.enState
						);
					}

				} else {
					/* ありえないはず */
					THM_INNER_LOG_E ("BUG: enAct is [%d] @SEQ_TIMEOUT_QUE\n", getSeqInfo (nThreadIdx, nSeqIdx)->enAct);
				}



			} else {
				THM_INNER_LOG_E ("BUG: unexpected queType.\n");
			}
		} else {
//THM_INNER_LOG_E( "not use\n" );
		}

		pstQueWorker ++;
	}

	if (i == nQueWorkerNum) {
		/* not found */
		THM_INNER_LOG_I( "not found. thIdx:[%d]\n", nThreadIdx );

	} else {
		if (isGetOut) {
			/* 見つかったので並び換える */
			pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;
			for (j = 0; j < nQueWorkerNum; j ++) {
				if (j >= i) {
					if (j < nQueWorkerNum -1) {
						memcpy (
							pstQueWorker +j,
							pstQueWorker +j +1,
							sizeof (ST_QUE_WORKER)
						);

					} else {
						/* 末尾 */
						clearQueWorker (pstQueWorker +j);
					}
				}
			}
		}
	}


	/* isDropフラグ立っているものは消します  逆からみます */
	pstQueWorker = gstInnerInfo [nThreadIdx].pstQueWorker;
	for (k = nQueWorkerNum -1; (k >= 0) && (k < nQueWorkerNum); k --) { // unsignedなので マイナス値にならず一周する...
		if ((pstQueWorker +k)->isDrop) {
			if (k < nQueWorkerNum -1) {
				for (l = k; l < nQueWorkerNum -1; l ++) {
					memcpy (
						pstQueWorker +l,
						pstQueWorker +l +1,
						sizeof (ST_QUE_WORKER)
					);
				}

			} else {
				/* 末尾 */
				clearQueWorker (pstQueWorker +k);
			}
		}
	}

	pthread_mutex_unlock (&gMutexOpeQueWorker [nThreadIdx]);

	return rtn;
}

/**
 * clearQueWorker
 * workerキュークリア
 */
static void clearQueWorker (ST_QUE_WORKER *p)
{
	if (!p) {
		return;
	}

	p->nDestThreadIdx = THREAD_IDX_BLANK;
	p->nDestSeqIdx = SEQ_IDX_BLANK;
	p->isValidSrcInfo = false;
	p->nSrcThreadIdx = THREAD_IDX_BLANK;
	p->nSrcSeqIdx = SEQ_IDX_BLANK;
	p->enQueType = EN_QUE_TYPE_INIT;
	p->nReqId = REQUEST_ID_BLANK;
	p->enRslt = EN_THM_RSLT_IGNORE;
	p->nClientId = NOTIFY_CLIENT_ID_BLANK;
	memset (p->msg.szMsg, 0x00, MSG_SIZE);
	p->msg.isUsed = false;
	p->isUsed = false;
	p->isDrop = false;
}

/**
 * baseThread -- main loop
 */
//TODO workerスレッドのキューの状態
static void *baseThread (void *pArg)
{
	int nRtn = 0;
	ST_QUE_BASE stRtnQue;
	struct timespec stTimeout = {0};
	struct timeval stNowTimeval = {0};

//TODO test
createExternalCp();

	/* set thread name */
	setThreadName (BASE_THREAD_NAME);

	/* スレッド立ち上がり通知 */
	postSem ();
	THM_INNER_FORCE_LOG_I ("----- %s created. -----\n", BASE_THREAD_NAME);


	while (1) {

		/* lock */
		pthread_mutex_lock (&gMutexBase);

		/*
		 * キューに入っているかチェック
		 * なければキューを待つ
		 */
		if (!deQueBase(false).isUsed) {

			getTimeOfDay (&stNowTimeval);
			stTimeout.tv_sec = stNowTimeval.tv_sec + BASE_THREAD_LOOP_TIMEOUT_SEC;
			stTimeout.tv_nsec = stNowTimeval.tv_usec * 1000;

			nRtn = pthread_cond_timedwait (&gCondBase, &gMutexBase, &stTimeout);
			switch (nRtn) {
			case SYS_RETURN_NORMAL:
				/* タイムアウト前に動き出した */
				break;

			case ETIMEDOUT:
				/*
				 * タイムアウトした
				 */
				THM_INNER_LOG_I ("ETIMEDOUT baseThread\n");

				/* 外部スレッドのReqタイムアウトチェック */
				checkReqTimeout (THREAD_IDX_EXTERNAL);

				/* workerスレッドの確認 */
				checkWorkerThread();

				break;

			case EINTR:
				/* シグナルに割り込まれた */
//TODO 現状考慮してない
				break;

			default:
				THM_INNER_LOG_E ("BUG: pthread_cond_timedwait() => unexpected return value [%d]\n", nRtn);
				break;
			}

		}

		/* unlock */
		pthread_mutex_unlock (&gMutexBase);


		/*
		 * キューから取り出す
		 */
		stRtnQue = deQueBase (true);
		if (stRtnQue.isUsed) {
			switch (stRtnQue.enMoniType) {
			case EN_MONI_TYPE_DEBUG:
				dumpRequestIdInfo ();
				dumpExtInfoList ();
				dumpQueAllThread ();
				break;

			default:
				break;
			}
		}

	}


	return NULL;
}

/**
 * sigwaitThread -- main loop
 */
static void *sigwaitThread (void *pArg)
{
	int nSig = 0;

//TODO test
createExternalCp();

	/* set thread name */
	setThreadName (SIGWAIT_THREAD_NAME);

	/* スレッド立ち上がり通知 */
	postSem ();
	THM_INNER_FORCE_LOG_I ("----- %s created. -----\n", SIGWAIT_THREAD_NAME);


	while (1) {
		if (sigwait(&gSigset, &nSig) == SYS_RETURN_NORMAL) {
			switch (nSig) {
			case SIGQUIT:
				THM_INNER_LOG_I ("catch SIGQUIT\n");
				requestBaseThread (EN_MONI_TYPE_DEBUG);
				break;
			default:
				THM_INNER_LOG_E ("BUG: unexpected signal.\n");
				break;
			}
		} else {
			THM_PERROR ("sigwait()");
		}
	}


	return NULL;
}

/**
 * checkWaitWorkerThread
 */
static void checkWaitWorkerThread (ST_INNER_INFO *pstInnerInfo)
{
	int nRtn = 0;
	EN_NEAREST_TIMEOUT enTimeout = EN_NEAREST_TIMEOUT_NONE;
	ST_REQUEST_ID_INFO *pstReqIdInfo = NULL;
	ST_SEQ_INFO *pstSeqInfo = NULL;
	struct timespec *pstTimeout = NULL;

	/* lock */
	pthread_mutex_lock (&gMutexWorker [pstInnerInfo->nThreadIdx]);

	/*
	 * キューに入っているかチェック
	 * なければキューを待つ
	 */
//	if (!deQueWorker( pstInnerInfo->nThreadIdx, false ).isUsed) {
	if (!check2deQueWorker (pstInnerInfo->nThreadIdx, false).isUsed) {

		setState (pstInnerInfo->nThreadIdx, EN_STATE_READY);

		enTimeout = searchNearestTimeout (pstInnerInfo->nThreadIdx, &pstReqIdInfo, &pstSeqInfo);
		if (enTimeout == EN_NEAREST_TIMEOUT_NONE) {
			THM_INNER_LOG_I ("normal cond wait\n");

			/*
			 * 通常の cond wait
			 * pthread_cond_signal待ち
			 */
			pthread_cond_wait (
				&gCondWorker [pstInnerInfo->nThreadIdx],
				&gMutexWorker [pstInnerInfo->nThreadIdx]
			);

		} else {
			/* タイムアウト仕掛かり中です */

			if (enTimeout == EN_NEAREST_TIMEOUT_REQ) {
				THM_INNER_LOG_I ("timeout cond wait (reqTimeout)\n");

				if (!pstReqIdInfo) {
					THM_INNER_LOG_E ("BUG: pstReqIdInfo is null !!!\n");
					return;
				}
				pstReqIdInfo->timeout.enState = EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT;
				pstTimeout = &(pstReqIdInfo->timeout.stTime);

			} else {
				THM_INNER_LOG_I ("timeout cond wait (seqTimeout)\n");

				if (!pstSeqInfo) {
					THM_INNER_LOG_E ("BUG: pstSeqInfo is null !!!\n");
					return;
				}
				pstSeqInfo->timeout.enState = EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT;
				pstTimeout = &(pstSeqInfo->timeout.stTime);
			}

			/*
			 * タイムアウト付き cond wait
			 * pthread_cond_signal待ち
			 */
			nRtn = pthread_cond_timedwait (
				&gCondWorker [pstInnerInfo->nThreadIdx],
				&gMutexWorker [pstInnerInfo->nThreadIdx],
				pstTimeout
			);
			switch (nRtn) {
			case SYS_RETURN_NORMAL:
				/* タイムアウト前に動き出した */
				if (enTimeout == EN_NEAREST_TIMEOUT_REQ) {
					if (!pstReqIdInfo) {
						THM_INNER_LOG_E ("BUG: pstReqIdInfo is null !!!\n");
						return;
					}
					pstReqIdInfo->timeout.enState = EN_TIMEOUT_STATE_MEAS;

				} else {
					if (!pstSeqInfo) {
						THM_INNER_LOG_E ("BUG: pstSeqInfo is null !!!\n");
						return;
					}
					pstSeqInfo->timeout.enState = EN_TIMEOUT_STATE_MEAS;
				}
				break;

			case ETIMEDOUT:
				/*
				 * タイムアウトした
				 * 自スレッドにタイムアウトのキューを入れる
				 */
				if (enTimeout == EN_NEAREST_TIMEOUT_REQ) {
					if (!pstReqIdInfo) {
						THM_INNER_LOG_E ("BUG: pstReqIdInfo is null !!!\n");
						return;
					}
					pstReqIdInfo->timeout.enState = EN_TIMEOUT_STATE_PASSED;
					enQueReqTimeout (pstInnerInfo->nThreadIdx, pstReqIdInfo->nId);

				} else {
					if (!pstSeqInfo) {
						THM_INNER_LOG_E ("BUG: pstSeqInfo is null !!!\n");
						return;
					}
					pstSeqInfo->timeout.enState = EN_TIMEOUT_STATE_PASSED;
					enQueSeqTimeout (pstInnerInfo->nThreadIdx, pstSeqInfo->nSeqIdx);
				}
				break;

			case EINTR:
				/* シグナルに割り込まれた */
//TODO 現状考慮してない
				THM_INNER_LOG_W ("interrupt occured.\n");
				break;

			default:
				THM_INNER_LOG_E ("BUG: pthread_cond_timedwait() => unexpected return value [%d]\n", nRtn);
				break;
			}

		}
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexWorker [pstInnerInfo->nThreadIdx]);
}

/**
 * workerThread -- main loop
 */
static void *workerThread (void *pArg)
{
	uint8_t i = 0;
	ST_INNER_INFO *pstInnerInfo = (ST_INNER_INFO*)pArg;
	ST_THM_REG_TBL *pTbl = gpstThmRegTbl [pstInnerInfo->nThreadIdx];
	ST_QUE_WORKER stRtnQue;
	ST_THM_SRC_INFO *pstThmSrcInfo = &gstThmSrcInfo [pstInnerInfo->nThreadIdx];


	/* init thmIf */
	ST_THM_IF stThmIf;
	clearThmIf (&stThmIf);


	/* --- set thread name --- */
	setThreadName ((char*)pTbl->pszName);
	pstInnerInfo->pszName = (char*)pTbl->pszName;

	/* --- pthread_t id --- */
	pstInnerInfo->nPthreadId = pthread_self();

	/* --- init queWorker --- */
	uint8_t nQueWorkerNum = gpstThmRegTbl [pstInnerInfo->nThreadIdx]->nQueNum;
	ST_QUE_WORKER stQueWorker [nQueWorkerNum]; // このスレッドのque実体
	for (i = 0; i < nQueWorkerNum; i ++) {
		clearQueWorker (&stQueWorker [i]);
	}
	pstInnerInfo->nQueWorkerNum = nQueWorkerNum;
	pstInnerInfo->pstQueWorker = &stQueWorker [0];

	/* --- init seqInfo --- */
	uint8_t nSeqNum = gpstThmRegTbl [pstInnerInfo->nThreadIdx]->nSeqNum;
	ST_SEQ_INFO stSeqInfo [nSeqNum]; // seqInfo実体
	for (i = 0; i < nSeqNum; i ++) {
		clearSeqInfo (&stSeqInfo [i]);
		stSeqInfo[i].nSeqIdx = i;
	}
	pstInnerInfo->nSeqNum = nSeqNum;
	pstInnerInfo->pstSeqInfo = &stSeqInfo [0];


	/* 
	 * setup
	 * 登録されていれば行います
	 */
	if (pTbl->pSetup) {
		(void)(pTbl->pSetup)();
	}

	setState (pstInnerInfo->nThreadIdx, EN_STATE_READY);

	/* スレッド立ち上がり通知 */
	postSem ();
	THM_INNER_FORCE_LOG_I ("----- %s created. -----\n", pstInnerInfo->pszName);


	while (1) {

		/*
		 * キューの確認とcondwait
		 */
		checkWaitWorkerThread (pstInnerInfo);

		/* busy */
		setState (pstInnerInfo->nThreadIdx, EN_STATE_BUSY);

		/*
		 * キューから取り出す
		 */	
//		stRtnQue = deQueWorker( pstInnerInfo->nThreadIdx, true );
		stRtnQue = check2deQueWorker (pstInnerInfo->nThreadIdx, true);
		if (stRtnQue.isUsed) {

			switch (stRtnQue.enQueType) {
			case EN_QUE_TYPE_SEQ_TIMEOUT:
				/* Seqタイムアウトが無事終わったので タイムアウト情報クリア */
				clearSeqTimeoutInfo (pstInnerInfo->nThreadIdx, stRtnQue.nDestSeqIdx);

			case EN_QUE_TYPE_REQ_TIMEOUT:
			case EN_QUE_TYPE_REQUEST:
			case EN_QUE_TYPE_REPLY:

				/*
				 * request/ reply/ timeout がきました
				 * 登録されているシーケンスを実行します
				 */

				if (*((pTbl->pSeqArray)+stRtnQue.nDestSeqIdx)) {

					/* sect init のrequestキューを保存 */
					//TODO EN_QUE_TYPE_REQUESTで分けるべきか
					if (((pstInnerInfo->pstSeqInfo)+stRtnQue.nDestSeqIdx)->enAct == EN_THM_ACT_INIT) {
						memcpy (
							&(((pstInnerInfo->pstSeqInfo)+stRtnQue.nDestSeqIdx)->stSeqInitQueWorker),
							&stRtnQue,
							sizeof(ST_QUE_WORKER)
						);
					}

					/* 色々と使うから キューまるまる保存 */
					memcpy (&(pstInnerInfo->stNowExecQueWorker), &stRtnQue, sizeof(ST_QUE_WORKER));

					/* 引数gstThmSrcInfoセット */
					pstThmSrcInfo->nReqId = stRtnQue.nReqId; /* getRequestId で生成した値 */
					pstThmSrcInfo->enRslt = stRtnQue.enRslt; /* reqestの場合EN_THM_RSLT_IGNOREが入る 無効な値
																replyの場合その結果が入る
																Reqタイムアウトの場合はEN_THM_RSLT_REQ_TIMEOUTが入る
																Seqタイムアウトの場合はEN_THM_RSLT_SEQ_TIMEOUTが入る */
					pstThmSrcInfo->nClientId = stRtnQue.nClientId; /* NOTIFY_CLIENT_ID_BLANK が入る 無効な値 */
					if (stRtnQue.msg.isUsed) {
						pstThmSrcInfo->pszMsg = stRtnQue.msg.szMsg;
					}

					/* 引数セット */
					stThmIf.pstSrcInfo = pstThmSrcInfo;
					stThmIf.pReply = reply;
					stThmIf.pRegNotify = registerNotify;
					stThmIf.pUnRegNotify = unregisterNotify;
					stThmIf.pNotify = notify;
					stThmIf.pSetSectId = setSectId;
					stThmIf.pGetSectId = getSectId;
					stThmIf.pSetTimeout = setTimeout;
					stThmIf.pClearTimeout = clearTimeout;


					while (1) { // EN_THM_ACT_CONTINUE の為のloopです
						/*
						 * 関数実行
						 * 主処理
						 * ユーザ側で sectId enActをセットするはず
						 */
						(void)(*((pTbl->pSeqArray)+stRtnQue.nDestSeqIdx)) (&stThmIf);

						if (((pstInnerInfo->pstSeqInfo)+stRtnQue.nDestSeqIdx)->enAct == EN_THM_ACT_CONTINUE) {
//TODO
//							/* Seqタイムアウト関係ないとこでは一応クリアする */
//							clearSeqTimeoutInfo (pstInnerInfo->nThreadIdx, stRtnQue.nDestSeqIdx);
							continue;

						} else if (((pstInnerInfo->pstSeqInfo)+stRtnQue.nDestSeqIdx)->enAct == EN_THM_ACT_WAIT) {

							if (((pstInnerInfo->pstSeqInfo)+stRtnQue.nDestSeqIdx)->timeout.nVal == SEQ_TIMEOUT_BLANK) {
								/* 一応クリア */
								clearSeqTimeoutInfo (pstInnerInfo->nThreadIdx, stRtnQue.nDestSeqIdx);

								/* this while loop break */
								break;

							} else {
								enableSeqTimeout (pstInnerInfo->nThreadIdx, stRtnQue.nDestSeqIdx);

								/* this while loop break */
								break;
							}

						} else {
							/* Seqタイムアウト関係ないとこでは一応クリアする */
							clearSeqTimeoutInfo (pstInnerInfo->nThreadIdx, stRtnQue.nDestSeqIdx);

							/* this while loop break */
							break;
						}
					}

					/* clear */
					clearQueWorker (&(pstInnerInfo->stNowExecQueWorker));
					clearThmIf (&stThmIf);
					clearThmSrcInfo (pstThmSrcInfo);
					clearSyncReplyInfo (&gstSyncReplyInfo [pstInnerInfo->nThreadIdx]);
				}

				break;

#if 0 //廃止
			case EN_QUE_TYPE_REPLY:
				/* ここに来るのは非同期のreply */

				if (pTbl->pRecvAsyncReply) {

					//TODO pstInnerInfo->stNowExecQueWorker の保存必要かどうか

					/* 引数gstThmSrcInfoセット */
					pstThmSrcInfo->nReqId = stRtnQue.nReqId; /* requestの時に生成したもの */
					pstThmSrcInfo->enRslt = stRtnQue.enRslt; /* 結果が入る */
					pstThmSrcInfo->nClientId = stRtnQue.nClientId; /* NOTIFY_CLIENT_ID_BLANK が入る 無効な値 */
					if (stRtnQue.msg.isUsed) {
						pstThmSrcInfo->pszMsg = stRtnQue.msg.szMsg;
					}

					/* 引数セット */
					stThmIf.pstSrcInfo = pstThmSrcInfo;
					stThmIf.pReply = NULL;
					stThmIf.pRegNotify = NULL;
					stThmIf.pUnRegNotify = NULL;
					stThmIf.pNotify = notify;
					stThmIf.pSetSectId = NULL;
					stThmIf.pGetSectId = NULL;
					stThmIf.pSetTimeout = NULL;

					/*
					 * 主処理
					 */
					(void)(pTbl->pRecvAsyncReply) (&stThmIf);

					/* clear */
					clearThmIf (&stThmIf);
					clearThmSrcInfo (pstThmSrcInfo);
				}

				break;
#endif

			case EN_QUE_TYPE_NOTIFY:
				/* notifyがきました */

				if (pTbl->pRecvNotify) {

					/* 引数gstThmSrcInfoセット */
					pstThmSrcInfo->nReqId = stRtnQue.nReqId; /* REQUEST_ID_BLANKが入る 無効な値 */
					pstThmSrcInfo->enRslt = stRtnQue.enRslt; /* EN_THM_RSLT_IGNOREが入る 無効な値 */
					pstThmSrcInfo->nClientId = stRtnQue.nClientId; /* notify時に登録した値 */
					if (stRtnQue.msg.isUsed) {
						pstThmSrcInfo->pszMsg = stRtnQue.msg.szMsg;
					}

					/* 引数セット */
					stThmIf.pstSrcInfo = pstThmSrcInfo;
					stThmIf.pReply = NULL;
					stThmIf.pRegNotify = NULL;
					stThmIf.pUnRegNotify = NULL;
					stThmIf.pNotify = notify;
					stThmIf.pSetSectId = NULL;
					stThmIf.pGetSectId = NULL;
					stThmIf.pSetTimeout = NULL;

					/*
					 * 主処理
					 */
					(void)(pTbl->pRecvNotify) (&stThmIf);

					/* clear */
					clearThmIf (&stThmIf);
					clearThmSrcInfo (pstThmSrcInfo);
				}

				break;

			default:
				THM_INNER_LOG_E ("BUG: unexpected queType.\n");
				break;
			}

		} /* stRtnQue.isUsed */


		/* Reqタイムアウトチェック */
		checkReqTimeout	(pstInnerInfo->nThreadIdx);
		/* Seqタイムアウトチェック */
		checkSeqTimeout (pstInnerInfo->nThreadIdx);


	} /* main while loop */

	/* ここにはこない */
	setState (pstInnerInfo->nThreadIdx, EN_STATE_INIT);


	return NULL;
}

/**
 * clearThmIf
 */
static void clearThmIf (ST_THM_IF *pIf)
{
	if (!pIf) {
		return;
	}

	pIf->pstSrcInfo = NULL;
	pIf->pReply = NULL;
	pIf->pRegNotify = NULL;
	pIf->pUnRegNotify = NULL;
	pIf->pNotify = NULL;
	pIf->pSetSectId = NULL;
	pIf->pGetSectId = NULL;
	pIf->pSetTimeout = NULL;
	pIf->pClearTimeout = NULL;
}

/**
 * clearThmSrcInfo
 */
static void clearThmSrcInfo (ST_THM_SRC_INFO *p)
{
	if (!p) {
		return;
	}

	p->nReqId = REQUEST_ID_BLANK;
	p->enRslt = EN_THM_RSLT_IGNORE;
	p->nClientId = NOTIFY_CLIENT_ID_BLANK;
	p->pszMsg = NULL;
}

/**
 * requestBaseThread
 */
static bool requestBaseThread (EN_MONI_TYPE enType)
{
	/* lock */
	pthread_mutex_lock (&gMutexBase);

	/* キューに入れる */
	if (!enQueBase(enType)) {
		/* unlock */
		pthread_mutex_unlock (&gMutexBase);

		THM_INNER_LOG_E ("enQuebase() is failure.\n");
		return false;
	}


	/*
	 * Request
	 * cond signal ---> BaseThread
	 */
	pthread_cond_signal (&gCondBase);


	/* unlock */
	pthread_mutex_unlock (&gMutexBase);

	return true;
}

/**
 * requestInner
 *
 * 引数 nThreadIdx, nSeqIdx は宛先です
 */
static bool requestInner (
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	uint32_t nReqId,
	ST_CONTEXT *pstContext,
	uint8_t *pszMsg
)
{
	/* lock */
	pthread_mutex_lock (&gMutexWorker [nThreadIdx]);

	/* キューに入れる */
	if (!enQueWorker (nThreadIdx, nSeqIdx, EN_QUE_TYPE_REQUEST,
							pstContext, nReqId, EN_THM_RSLT_IGNORE, NOTIFY_CLIENT_ID_BLANK, pszMsg)) {
		/* unlock */
		pthread_mutex_unlock (&gMutexWorker [nThreadIdx]);

		THM_INNER_LOG_E ("enQueWorker() is failure.\n");
		return false;
	}


	/*
	 * Request
	 * cond signal ---> workerThread
	 */
	pthread_cond_signal (&gCondWorker [nThreadIdx]);


	/* unlock */
	pthread_mutex_unlock (&gMutexWorker [nThreadIdx]);

	return true;
}

/**
 * Request(同期)
 *
 * 引数 nThreadIdx, nSeqIdx は宛先です
 * RequestしたスレッドはReplyが来るまでここで固まる
 * 自分自身に投げたらデッドロックします
 *
 * 公開用 external_if
 */
bool requestSync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg)
{
	if ((nThreadIdx < 0) || (nThreadIdx >= getTotalWorkerThreadNum())) {
		THM_INNER_LOG_E ("invalid arument\n");
		return false;
	}
	if ((nSeqIdx < 0) || (nSeqIdx >= SEQ_IDX_MAX)) {
		/*
		 * 外部のスレッドから呼ぶこともできるから SEQ_IDX_MAX 未満にする
		 * (内部からのみだったら gstInnerInfo [nThreadIdx].nSeqNum未満でいい)
		 */
		THM_INNER_LOG_E ("invalid arument\n");
		return false;
	}

	uint32_t reqId = REQUEST_ID_BLANK;
	ST_REQUEST_ID_INFO *pstTmpReqIdInfo = NULL;
	ST_SYNC_REPLY_INFO *pstTmpSyncReplyInfo = NULL;
	ST_EXTERNAL_CONTROL_INFO *pstExtInfo = NULL;
	int nRtn = 0;


	ST_CONTEXT stContext = getContext();
	if (!stContext.isValid) {
		/* --- 外部のスレッドからリクエストの場合の処理 --- */

		pstExtInfo = searchExtInfoList (pthread_self());
		if (!pstExtInfo) {
			/* 事前にcreateExternalCpしてない */
			THM_INNER_LOG_E ("extInfo is not found.  not request... (exec createExternalCp() in advance.)\n");
			return false;
		}

		/* requestId */
		reqId = getRequestId (THREAD_IDX_EXTERNAL, SEQ_IDX_BLANK);
		if (reqId == REQUEST_ID_BLANK) {
			return false;
		}

		/* lock */
		pthread_mutex_lock (&(pstExtInfo->mutex));
		pstExtInfo->nReqId = reqId;
		pthread_mutex_unlock (&(pstExtInfo->mutex));

		/* リクエスト投げる */
		if (!requestInner (nThreadIdx, nSeqIdx, reqId, &stContext, pszMsg)) {
			releaseRequestId (THREAD_IDX_EXTERNAL, reqId);
			return false;
		}

		enableReqTimeout (THREAD_IDX_EXTERNAL, reqId);

		return true;
	}


	/* requestId */
	reqId = getRequestId (stContext.nThreadIdx, stContext.nSeqIdx);
	if (reqId == REQUEST_ID_BLANK) {
		return false;
	}


	if (!setRequestIdSyncReply (stContext.nThreadIdx, reqId)) {
		/* まだ使われてる.. ありえないけど一応 */
		releaseRequestId (stContext.nThreadIdx, reqId);
		return false;
	}

	/* 自分はEN_STATE_WAIT_REPLY にセット */
	setState (stContext.nThreadIdx, EN_STATE_WAIT_REPLY);


	/*
	 * !!!ここからgMutexSyncReply を lock !!!
	 * こちらがpthread_cond_waitする前に 相手がreply(pthread_cond_signal)してくるのを防ぎます
	 */
//TODO isRequestAlreadyで判断
	pthread_mutex_lock (&gMutexSyncReply [stContext.nThreadIdx]);

	/* リクエスト投げる */
	if (!requestInner(nThreadIdx, nSeqIdx, reqId, &stContext, pszMsg)) {

		/* gMutexSyncReply unlock */
		pthread_mutex_unlock (&gMutexSyncReply[stContext.nThreadIdx]);

		releaseRequestId (stContext.nThreadIdx, reqId);
		return false;
	}

	enableReqTimeout (stContext.nThreadIdx, reqId);
	pstTmpReqIdInfo = getRequestIdInfo (stContext.nThreadIdx, reqId);
	if (!pstTmpReqIdInfo) {
		/* gMutexSyncReply unlock */
		pthread_mutex_unlock (&gMutexSyncReply[stContext.nThreadIdx]);

		releaseRequestId (stContext.nThreadIdx, reqId);
		return false;
	}


	THM_INNER_LOG_I ("requestSync... cond timedwait\n");

	/* 自分はcond wait して固まる(Reply待ち) */
	nRtn = pthread_cond_timedwait (
		&gCondSyncReply [stContext.nThreadIdx],
		&gMutexSyncReply [stContext.nThreadIdx],
		&(pstTmpReqIdInfo->timeout.stTime)
	);
	pstTmpSyncReplyInfo = getSyncReplyInfo (stContext.nThreadIdx);
	switch (nRtn) {
	case SYS_RETURN_NORMAL:
		THM_INNER_LOG_I ("requestSync... reply come\n");

		/*
		 * リプライが来た
		 * 結果をいれる
		 */
		if (pstTmpSyncReplyInfo) {
			gstThmSrcInfo [stContext.nThreadIdx].nReqId = reqId;
			gstThmSrcInfo [stContext.nThreadIdx].enRslt = pstTmpSyncReplyInfo->enRslt;
			if (pstTmpSyncReplyInfo->msg.isUsed) {
				gstThmSrcInfo [stContext.nThreadIdx].pszMsg = pstTmpSyncReplyInfo->msg.szMsg;
			} else {
				gstThmSrcInfo [stContext.nThreadIdx].pszMsg = NULL;
			}
		}
		break;

	case ETIMEDOUT:
		THM_INNER_LOG_W ("requestSync... timeout\n");

		/*
		 * タイムアウトした
		 */
		if (pstTmpSyncReplyInfo) {
			clearThmSrcInfo (&gstThmSrcInfo [stContext.nThreadIdx]);
		}
		break;

	case EINTR:
		/* シグナルに割り込まれた */
//TODO 現状考慮してない
		break;

	default:
		THM_INNER_LOG_E ("BUG: pthread_cond_timedwait() => unexpected return value [%d]\n", nRtn);
		break;
	}

	/* gMutexSyncReply unlock */
	pthread_mutex_unlock (&gMutexSyncReply[stContext.nThreadIdx]);


	releaseRequestId (stContext.nThreadIdx, reqId);

	/* 再びbusyに */
	setState (stContext.nThreadIdx, EN_STATE_BUSY);


	return true;
}

/**
 * Request(非同期)
 *
 * 引数 nThreadIdx, nSeqIdx は宛先です
 * pReqIdはout引数です
 *
 * 外部のスレッドからリクエストした場合はリプライは来ない
 *
 * 公開用 external_if
 */
bool requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg, uint32_t *pnReqId)
{
	if ((nThreadIdx < 0) || (nThreadIdx >= getTotalWorkerThreadNum())) {
		THM_INNER_LOG_E ("invalid arument\n");
		return false;
	}
	if ((nSeqIdx < 0) || (nSeqIdx >= SEQ_IDX_MAX)) {
		/*
		 * 外部のスレッドから呼ぶこともできるから SEQ_IDX_MAX 未満にする
		 * (内部だったら gstInnerInfo [nThreadIdx].nSeqNum未満でいい)
		 */
		THM_INNER_LOG_E ("invalid arument\n");
		return false;
	}

	uint32_t reqId = REQUEST_ID_BLANK;
	ST_EXTERNAL_CONTROL_INFO *pstExtInfo = NULL;


	ST_CONTEXT stContext = getContext();
	if (!stContext.isValid) {
		/* --- 外部のスレッドからリクエストの場合の処理 --- */

		pstExtInfo = searchExtInfoList (pthread_self());
		if (!pstExtInfo) {
			/* 事前にcreateExternalCpしてない */
			THM_INNER_LOG_E ("extInfo is not found.  not request... (exec createExternalCp() in advance.)\n");
			return false;
		}

		/* requestId */
		reqId = getRequestId (THREAD_IDX_EXTERNAL, SEQ_IDX_BLANK);
		if (reqId == REQUEST_ID_BLANK) {
			return false;
		}

		/* 引数pReqIdに返却 */
		if (pnReqId) {
			*pnReqId = reqId;
		}

		/*
		 * reqIdをextInfoにセット
		 * 上書きなので 外部スレッドからは1つづつのrequestしかできません
		 */
		pstExtInfo->nReqId = reqId;

		/* リクエスト投げる */
		if (!requestInner (nThreadIdx, nSeqIdx, reqId, &stContext, pszMsg)) {
			releaseRequestId (THREAD_IDX_EXTERNAL, reqId);
			return false;
		}

		enableReqTimeout (THREAD_IDX_EXTERNAL, reqId);

		return true;
	}


	/* requestId */
	reqId = getRequestId (stContext.nThreadIdx, stContext.nSeqIdx);
	if (reqId == REQUEST_ID_BLANK) {
		return false;
	}

	/* 引数pReqIdに返却 */
	if (pnReqId) {
		*pnReqId = reqId;
	}

#ifndef _MULTI_REQUESTING
	/*
	 * reqIdを自身のseqInfoに保存
	 * replyが返ってきたとき check2deQueWorker()で照合するため
	 */
	if (stContext.isValid) {
		uint8_t nContextSeqIdx = stContext.nSeqIdx;
		getSeqInfo (stContext.nThreadIdx, nContextSeqIdx)->nReqId = reqId;
	}
#endif

	/* リクエスト投げる */
	if (!requestInner(nThreadIdx, nSeqIdx, reqId, &stContext, pszMsg)) {
		releaseRequestId (stContext.nThreadIdx, reqId);
		return false;
	}

	enableReqTimeout (stContext.nThreadIdx, reqId);


	return true;
}

/**
 * replyInner
 *
 * 引数 nThreadIdx, nSeqIdx は宛先です nReqIdはRequest元の値
 */
static bool replyInner (
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	uint32_t nReqId,
	ST_CONTEXT *pstContext,
	EN_THM_RSLT enRslt,
	uint8_t *pszMsg,
	bool isSync
)
{
	if (isSync) {
		/* sync Reply */
		if (!cashSyncReplyInfo (nThreadIdx, enRslt, pszMsg)) {
			THM_INNER_LOG_E ("cashSyncReplyInfo() is failure.\n");
			return false;
		}

		pthread_mutex_lock (&gMutexSyncReply [nThreadIdx]);

		pthread_cond_signal (&gCondSyncReply [nThreadIdx]);

		setReplyAlreadySyncReplyInfo (nThreadIdx);
		pthread_mutex_unlock (&gMutexSyncReply [nThreadIdx]);

		return true;
	}


	/* lock */
	pthread_mutex_lock (&gMutexWorker [nThreadIdx]);


	/* Reqタイムアウトしてないか確認する */
	EN_TIMEOUT_STATE enState = getReqTimeoutState (nThreadIdx, nReqId);
	if ((enState != EN_TIMEOUT_STATE_MEAS) && (enState != EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT)) {
		THM_INNER_LOG_E ("getReqTimeoutState() is unexpected. [%d]   maybe timeout occured. not reply...\n", enState);

		/* unlock */
		pthread_mutex_unlock (&gMutexWorker [nThreadIdx]);

		return false;
	}

	/* キューに入れる */
	if (!enQueWorker (nThreadIdx, nSeqIdx, EN_QUE_TYPE_REPLY,
							pstContext, nReqId, enRslt, NOTIFY_CLIENT_ID_BLANK, pszMsg)) {
		/* unlock */
		pthread_mutex_unlock (&gMutexWorker [nThreadIdx]);

		THM_INNER_LOG_E ("enQueWorker() is failure.\n");
		return false;
	}


	/* 
	 * Reply
	 * cond signal ---> workerThread
	 */
	pthread_cond_signal (&gCondWorker [nThreadIdx]);


	/* unlock */
	pthread_mutex_unlock (&gMutexWorker [nThreadIdx]);

	return true;
}

/**
 * replyOuter
 * 外部スレッドへのリプライ
 */
static bool replyOuter (
	uint32_t nReqId,
	ST_CONTEXT *pstContext,
	EN_THM_RSLT enRslt,
	uint8_t *pszMsg
)
{
	ST_EXTERNAL_CONTROL_INFO *pstExtInfo = NULL;

	pstExtInfo = searchExtInfoListFromRequestId (nReqId);
	if (!pstExtInfo) {
		/* createExternalCp してない or  別のrequestで書き換わった or タイムアウト */
		THM_INNER_LOG_W ("extInfo is not found.  not reply...\n");
		return false;
	}

	/* lock */
	pthread_mutex_lock (&(pstExtInfo->mutex));

	/* 結果を入れます */
	if (enRslt == EN_THM_RSLT_REQ_TIMEOUT) {
		/* タイムアウトだったら extInfoのreqIdクリア */
		pstExtInfo->nReqId = REQUEST_ID_BLANK;
		pstExtInfo->stThmSrcInfo.nReqId = REQUEST_ID_BLANK;
	} else {
		pstExtInfo->stThmSrcInfo.nReqId = nReqId;
	}
	pstExtInfo->stThmSrcInfo.enRslt = enRslt;
	if (pszMsg) {
		memcpy (pstExtInfo->szMsgEntity, pszMsg, MSG_SIZE);
		pstExtInfo->stThmSrcInfo.pszMsg = pstExtInfo->szMsgEntity;
	} else {
		pstExtInfo->stThmSrcInfo.pszMsg = NULL;
	}

	pthread_cond_signal (&(pstExtInfo->cond));
	pstExtInfo->isReplyAlready = true;

	/* unlock */
	pthread_mutex_unlock (&(pstExtInfo->mutex));

	return true;
}

/**
 * Reply
 * 公開用
 */
static bool reply (EN_THM_RSLT enRslt, uint8_t *pszMsg)
{
	/*
	 * getContext->自分のthreadIdx取得->innerInfoを参照して返送先を得る
	 */

	ST_CONTEXT stContext = getContext();

	if (!stContext.isValid) {
		/* ありえないけど一応 */
		return false;
	}

	uint8_t nThreadIdx = THREAD_IDX_BLANK;
	uint8_t nSeqIdx = SEQ_IDX_BLANK;
	uint32_t nReqId = REQUEST_ID_BLANK;
	bool isSync = false;

	bool isValid = getSeqInfo (stContext.nThreadIdx, stContext.nSeqIdx)->stSeqInitQueWorker.isValidSrcInfo;
	if (!isValid) {
		/* 返送先がわからない --> 外部のスレッドに返します */

		nReqId = getSeqInfo (stContext.nThreadIdx, stContext.nSeqIdx)->stSeqInitQueWorker.nReqId;

		/* リプライ投げる */
		if (!replyOuter (nReqId, &stContext, enRslt, pszMsg)) {
			return false;
		}

	} else {
		/* 内部へ通常リプライ */

		nThreadIdx = getSeqInfo (stContext.nThreadIdx, stContext.nSeqIdx)->stSeqInitQueWorker.nSrcThreadIdx;
		nSeqIdx = getSeqInfo (stContext.nThreadIdx, stContext.nSeqIdx)->stSeqInitQueWorker.nSrcSeqIdx;
		nReqId = getSeqInfo (stContext.nThreadIdx, stContext.nSeqIdx)->stSeqInitQueWorker.nReqId;

		isSync = isSyncReplyFromRequestId (nThreadIdx, nReqId);

		/* リプライ投げる */
		if (!replyInner (nThreadIdx, nSeqIdx, nReqId, &stContext, enRslt, pszMsg, isSync)) {
			return false;
		}
	}

	return true;
}

/**
 * getContext
 * これを実行したコンテキがどのスレッドなのか把握する
 */
static ST_CONTEXT getContext (void)
{
	int i = 0;
	ST_CONTEXT stContext;

	/* pthread_self() */
	pthread_t nCurrentPthreadId = pthread_self();

	/* init clear */
	clearContext (&stContext);

	for (i = 0; i < getTotalWorkerThreadNum(); i ++) {
		if (pthread_equal (gstInnerInfo [i].nPthreadId, nCurrentPthreadId)) {
			break;
		}
	}

	if (i < getTotalWorkerThreadNum()) {
		stContext.isValid = true;
		stContext.nThreadIdx = gstInnerInfo [i].nThreadIdx;
		stContext.nSeqIdx = gstInnerInfo [i].stNowExecQueWorker.nDestSeqIdx;

	} else {
		/* not found */
		/* フレームワーク管理外のスレッドです */
		THM_INNER_LOG_W ("this framework unmanaged thread. pthreadId:[%lu]\n", nCurrentPthreadId);
	}

	return stContext;
}

/**
 * clearContext
 */
static void clearContext (ST_CONTEXT *p)
{
	if (!p) {
		return;
	}

	p->isValid = false;
	p->nThreadIdx = THREAD_IDX_BLANK;
	p->nSeqIdx = SEQ_IDX_BLANK;
}

/**
 * getRequestId
 * (REQUEST_ID_MAX -1 で一周する)
 * Reply時にどのRequestのものか判別したり、Reqタイムアウト判定するためのもの
 * 
 * 引数 nThreadIdx nSeqIdx は リクエスト元の値です
 * REQUEST_ID_BLANKが戻ったらエラーです
 */
static uint32_t getRequestId (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	if ((nThreadIdx < 0) || (nThreadIdx >= getTotalWorkerThreadNum())) {
		/* 外部スレッドを考慮 */
		THM_INNER_LOG_N ("external request\n");
		nThreadIdx = THREAD_IDX_EXTERNAL;
	} else {
		// inner
		if ((nSeqIdx < 0) || (nSeqIdx >= gstInnerInfo [nThreadIdx].nSeqNum)) {
			THM_INNER_LOG_E ("invalid arument\n");
			return REQUEST_ID_BLANK;
		}
	}

	uint32_t nRtnReqId = REQUEST_ID_BLANK;
	uint32_t n = 0;


	/* lock */
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);


	while (n < REQUEST_ID_MAX) {
		if (gstRequestIdInfo [nThreadIdx][gnRequestIdInd[nThreadIdx]].nId != REQUEST_ID_BLANK) {
			/* next */
			gnRequestIdInd [nThreadIdx] ++;
			gnRequestIdInd [nThreadIdx] &= REQUEST_ID_MAX -1;
		} else {
			break;
		}
		n ++;
	}

	if (n == REQUEST_ID_MAX) {
		/* 空きがない エラー */
		THM_INNER_LOG_E ("request id is full.\n");

		/* unlock  */
		pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);

		return REQUEST_ID_BLANK;
	}

	nRtnReqId = gnRequestIdInd [nThreadIdx];
	gstRequestIdInfo [nThreadIdx][nRtnReqId].nId = nRtnReqId;
	gstRequestIdInfo [nThreadIdx][nRtnReqId].nSrcThreadIdx = nThreadIdx;
	gstRequestIdInfo [nThreadIdx][nRtnReqId].nSrcSeqIdx = nSeqIdx;


	/* for next set */
	gnRequestIdInd [nThreadIdx] ++;
	gnRequestIdInd [nThreadIdx] &= REQUEST_ID_MAX -1;


	/* unlock */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);

	return nRtnReqId;
}

/**
 * dumpRequestId
 * デバッグ用
 */
static void dumpRequestIdInfo (void)
{
	int i = 0;
	int j = 0;

//TODO 参照だけだからmutexしない

	THM_INNER_FORCE_LOG_N ("####  dump requestIdInfo  ####\n");

	for (i = 0; i < getTotalWorkerThreadNum(); i ++) {
		THM_INNER_FORCE_LOG_N (" --- thread:[%s]\n", gstInnerInfo [i].pszName);
		for (j = 0; j < REQUEST_ID_MAX; j ++) {
			if (gstRequestIdInfo [i][j].nId != REQUEST_ID_BLANK) {
				THM_INNER_FORCE_LOG_N ("  0x%x\n", gstRequestIdInfo [i][j].nId);
			}
		}
	}

	/* 外部スレッド */
	THM_INNER_FORCE_LOG_N (" --- external\n");
	for (j = 0; j < REQUEST_ID_MAX; j ++) {
		if (gstRequestIdInfo [THREAD_IDX_EXTERNAL][j].nId != REQUEST_ID_BLANK) {
			THM_INNER_FORCE_LOG_N ("  0x%x\n", gstRequestIdInfo [THREAD_IDX_EXTERNAL][j].nId);
		}
	}
}

/**
 * getReqTimeoutState
 * ここは参照のみだが他スレッドからも呼ばれるところなのでmutex reply時のチェック
 */
static EN_TIMEOUT_STATE getReqTimeoutState (uint8_t nThreadIdx, uint32_t nReqId)
{
	EN_TIMEOUT_STATE enState = EN_TIMEOUT_STATE_INIT;

	/* lock */
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);

	enState = gstRequestIdInfo [nThreadIdx][nReqId].timeout.enState;

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);

	return enState;
}

/**
 * getRequestIdInfo
 */
static ST_REQUEST_ID_INFO *getRequestIdInfo (uint8_t nThreadIdx, uint32_t nReqId)
{
	ST_REQUEST_ID_INFO *pInfo = NULL;

	/* lock */
//TODO 同スレッドからのみ呼ばれる 参照のみなのでmutexいらないかも
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);

	pInfo = &gstRequestIdInfo [nThreadIdx][nReqId];

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);

	return pInfo;
}

/**
 * enableReqTimeout
 * Reqタイムアウトを仕掛ける
 */
static void enableReqTimeout (uint8_t nThreadIdx, uint32_t nReqId)
{
	if ((nThreadIdx < 0) || (nThreadIdx >= getTotalWorkerThreadNum())) {
		/* 外部スレッドを考慮 */
		THM_INNER_LOG_N ("external\n");
		nThreadIdx = THREAD_IDX_EXTERNAL;
	}
	if ((nReqId < 0) || (nReqId >= REQUEST_ID_MAX)) {
		THM_INNER_LOG_E ("invalid arg reqId.\n");
		return;
	}

	/* lock */
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);

	if (gstRequestIdInfo [nThreadIdx][nReqId].timeout.enState != EN_TIMEOUT_STATE_INIT) {
		THM_INNER_LOG_E ("BUG: timeout.enState != EN_TIMEOUT_STATE_INIT\n");

		/* unlock */
		pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);

		return;
	}

	struct timeval stNowTimeval = { 0 };
	getTimeOfDay (&stNowTimeval);
	gstRequestIdInfo [nThreadIdx][nReqId].timeout.enState = EN_TIMEOUT_STATE_MEAS;
	gstRequestIdInfo [nThreadIdx][nReqId].timeout.stTime.tv_sec = stNowTimeval.tv_sec + REQUEST_TIMEOUT_SEC;
	gstRequestIdInfo [nThreadIdx][nReqId].timeout.stTime.tv_nsec = stNowTimeval.tv_usec * 1000;

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);
}

/**
 * checkReqTimeout
 * タイムアウトしたかどうかの確認して キュー入れる
 *
 * 一連の登録シーケンス実行後にチェック
 * 外部スレッドは baseThreadでチェック
 */
static void checkReqTimeout (uint8_t nThreadIdx)
{
	ST_CONTEXT stContext;
	clearContext (&stContext);

	/* lock */
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);

	int i = 0;
	for (i = 0; i < REQUEST_ID_MAX; i ++) {
		if (gstRequestIdInfo [nThreadIdx][i].timeout.enState == EN_TIMEOUT_STATE_MEAS) {
			if (isReqTimeoutFromRequestId (nThreadIdx, i)) {
				if (nThreadIdx == THREAD_IDX_EXTERNAL) {
					/*
					 * 外部スレッドのReqタイムアウト
					 * ここでreplyして reqId解放します
					 */
					replyOuter (i, &stContext, EN_THM_RSLT_REQ_TIMEOUT, NULL);
					releaseRequestId (THREAD_IDX_EXTERNAL, i);

				} else {
					/* 自スレッドにReqタイムアウトのキューを入れる */
					gstRequestIdInfo [nThreadIdx][i].timeout.enState = EN_TIMEOUT_STATE_PASSED;
					enQueReqTimeout (nThreadIdx, i);
				}
			}
		}
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);
}

/**
 * isReqTimeoutFromRequestId
 */
static bool isReqTimeoutFromRequestId (uint8_t nThreadIdx, uint32_t nReqId)
{
	// checkReqTimeout から呼ばれるので ここはmutex済み

	bool isTimeout = false;
	struct timeval stNowTimeval = {0};
	getTimeOfDay (&stNowTimeval);
	time_t now_sec = stNowTimeval.tv_sec;
	long now_nsec  = stNowTimeval.tv_usec * 1000;

	time_t timeout_sec = gstRequestIdInfo [nThreadIdx][nReqId].timeout.stTime.tv_sec;
	long timeout_nsec  = gstRequestIdInfo [nThreadIdx][nReqId].timeout.stTime.tv_nsec;

	time_t diff_sec = 0;
	long diff_nsec = timeout_nsec - now_nsec;
	if (diff_nsec >= 0) {
		diff_sec = timeout_sec - now_sec;
	} else {
		diff_sec = timeout_sec -1 - now_sec;
		diff_nsec += 1000000000;
	}

	if (diff_sec > 0) {
		/* まだタイムアウトしてない */
		THM_INNER_LOG_I ("not yet timeout(ReqTimeout) threadIdx:[%d] reqId:[0x%x]\n", nThreadIdx, nReqId);

	} else if (diff_sec == 0) {
		if (diff_nsec > 0) {
			/* まだタイムアウトしてない 1秒切ってる */
			THM_INNER_LOG_I ("not yet timeout - off the one second (SeqTimeout) threadIdx:[%d] reqId:[0x%x]\n", nThreadIdx, nReqId);

		} else if (diff_nsec == 0) {
			/* ちょうどタイムアウト かなりレア... */
			THM_INNER_LOG_N ("just timedout(ReqTimeout) threadIdx:[%d] reqId:[0x%x]\n", nThreadIdx, nReqId);
			isTimeout = true;

		} else {
			/* ありえない */
			THM_INNER_LOG_E ("BUG: diff_nsec is minus value.\n");
		}

	} else {
		/* 既にタイムアウトしてる... */
		THM_INNER_LOG_N ("already timedout(ReqTimeout) threadIdx:[%d] reqId:[0x%x]\n", nThreadIdx, nReqId);
		isTimeout = true;
	}

	return isTimeout;
}

/**
 * enQueReqTimeout
 */
static bool enQueReqTimeout (uint8_t nThreadIdx, uint32_t nReqId)
{
	ST_CONTEXT stContext = getContext();

	/* lock */
	/* 再帰mutexする場合あります */
//TODO 同スレッドからのみ呼ばれる 参照のみなのでmutexいらないかも
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);


	uint8_t nSeqIdx = gstRequestIdInfo [nThreadIdx][nReqId].nSrcSeqIdx;

	if (!enQueWorker (nThreadIdx, nSeqIdx, EN_QUE_TYPE_REQ_TIMEOUT,
							&stContext, nReqId, EN_THM_RSLT_REQ_TIMEOUT, NOTIFY_CLIENT_ID_BLANK, NULL)) {
		THM_INNER_LOG_E ("enQueWorker() is failure.\n");

		/* unlock */
		pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);

		return false;
	}

	THM_INNER_LOG_N ("enQue REQ_TIMEOUT %s reqId:[0x%x]\n", gstInnerInfo [nThreadIdx].pszName, nReqId);

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);

	return true;
}

/**
 * searchNearestReqTimeout
 * Reqタイムアウト仕掛り中のもので 最も近い時間のものを探す
 * ST_REQUEST_ID_INFO で返します
 * タイムアウト仕掛けてなければnullを返します
 *
 * cond_waitする直前にチェックします
 * pthread_cond_wait か pthread_cond_timedwaitにするかの判断
 */
static ST_REQUEST_ID_INFO *searchNearestReqTimeout (uint8_t nThreadIdx)
{
	time_t timeout_sec_min = 0;
	time_t timeout_nsec_min = 0;
	time_t timeout_sec = 0;
	long timeout_nsec  = 0;
	uint8_t nRtnReqId = REQUEST_ID_MAX;

	/* lock */
//TODO 同スレッドからのみ呼ばれる 参照のみなのでmutexいらないかも
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);

	int i = 0;
	for (i = 0; i < REQUEST_ID_MAX; i ++) {
		if (gstRequestIdInfo [nThreadIdx][i].timeout.enState == EN_TIMEOUT_STATE_MEAS) {

			if ((timeout_sec_min == 0) && (timeout_nsec_min == 0)) {
				// 初回
				timeout_sec_min = gstRequestIdInfo [nThreadIdx][i].timeout.stTime.tv_sec;
				timeout_nsec_min = gstRequestIdInfo [nThreadIdx][i].timeout.stTime.tv_nsec;
				nRtnReqId = i;

			} else {
				timeout_sec = gstRequestIdInfo [nThreadIdx][i].timeout.stTime.tv_sec;
				timeout_nsec = gstRequestIdInfo [nThreadIdx][i].timeout.stTime.tv_nsec;

				if (timeout_sec < timeout_sec_min) {
					timeout_sec_min = timeout_sec;
					timeout_nsec_min = timeout_nsec;
					nRtnReqId = i;

				} else if (timeout_sec == timeout_sec_min) {
					if (timeout_nsec < timeout_nsec_min) {
						timeout_sec_min = timeout_sec;
						timeout_nsec_min = timeout_nsec;
						nRtnReqId = i;
					}
				}
			}
		}
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);


	if (nRtnReqId == REQUEST_ID_MAX) {
		/* タイムアウト仕掛かけてない */
		return NULL;
	} else {
		return &gstRequestIdInfo [nThreadIdx][nRtnReqId];
	}
}

/**
 * releaseRequestId
 * reqIdを解放します
 *
 * reply受けた時やReqタイムアウト等の回収したタイミングで実行します
 */
static void releaseRequestId (uint8_t nThreadIdx, uint32_t nReqId)
{
	if ((nThreadIdx < 0) || (nThreadIdx >= getTotalWorkerThreadNum())) {
		/* 外部スレッドを考慮 */
		THM_INNER_LOG_N ("external\n");
		nThreadIdx = THREAD_IDX_EXTERNAL;
	}
	if ((nReqId < 0) || (nReqId >= REQUEST_ID_MAX)) {
		THM_INNER_LOG_E ("invalid arg reqId.\n");
		return;
	}

	/* lock */
	pthread_mutex_lock (&gMutexOpeRequestId [nThreadIdx]);

	if (gstRequestIdInfo [nThreadIdx][nReqId].nId == REQUEST_ID_BLANK) {
		THM_INNER_LOG_E ("reqId:[0x%x] is already released ???\n", nReqId);
	} else {
		clearRequestIdInfo (&gstRequestIdInfo [nThreadIdx][nReqId]);
	}

	/* unlock  */
	pthread_mutex_unlock (&gMutexOpeRequestId [nThreadIdx]);
}

/**
 * clearRequestIdInfo
 */
static void clearRequestIdInfo (ST_REQUEST_ID_INFO *p)
{
	if (!p) {
		return;
	}

	p->nId = REQUEST_ID_BLANK;
	p->nSrcThreadIdx = THREAD_IDX_BLANK;
	p->nSrcSeqIdx = SEQ_IDX_BLANK;
	p->timeout.enState = EN_TIMEOUT_STATE_INIT;
	memset (&(p->timeout.stTime), 0x00, sizeof(struct timespec));
}

/**
 * isSyncReplyFromRequestId
 * 同期reply待ち用
 */
static bool isSyncReplyFromRequestId (uint8_t nThreadIdx, uint32_t nReqId)
{
	if (gstSyncReplyInfo [nThreadIdx].nReqId == nReqId) {
		return true;
	} else {
		return false;
	}
}

/**
 * setRequestIdSyncReply
 * 同期reply待ちを開始する前にreqIdをとっておく
 */
static bool setRequestIdSyncReply (uint8_t nThreadIdx, uint32_t nReqId)
{
	if (gstSyncReplyInfo [nThreadIdx].isUsed) {
		/* 使われてる */
		return false;
	}

	gstSyncReplyInfo [nThreadIdx].isUsed = true;
	gstSyncReplyInfo [nThreadIdx].nReqId = nReqId;
	return true;
}

/**
 * getSyncReplyInfo
 * 同期reply待ち-> reply返って来て動き出した時に結果を取るため用(gstThmSrcInfoにいれる)
 */
static ST_SYNC_REPLY_INFO *getSyncReplyInfo (uint8_t nThreadIdx)
{
	if (
		(!gstSyncReplyInfo [nThreadIdx].isUsed) ||
		(gstSyncReplyInfo [nThreadIdx].nReqId == REQUEST_ID_BLANK)
	) {
		/*
		 * setRequestIdSyncReply()を実行してない
		 * Reply待ちしてれば ここには来ないはず
		 */
		return NULL;
	}

	return &gstSyncReplyInfo [nThreadIdx];
}

/**
 * cashSyncReplyInfo
 * 同期reply待ち中に replyする側が結果をキャッシュ messageも
 *
 * 引数 nThreadIdxは request元スレッドです
 */
static bool cashSyncReplyInfo (uint8_t nThreadIdx, EN_THM_RSLT enRslt, uint8_t *pszMsg)
{
	if (
		(!gstSyncReplyInfo [nThreadIdx].isUsed) ||
		(gstSyncReplyInfo [nThreadIdx].nReqId == REQUEST_ID_BLANK)
	) {
		/*
		 * setRequestIdSyncReply()を実行してない
		 * Reply待ちしてれば ここには来ないはず
		 */
		return false;
	}

	/* result 保存 */
	gstSyncReplyInfo [nThreadIdx].enRslt = enRslt;

	/* message 保存 */
	if (pszMsg) {
		memcpy (gstSyncReplyInfo [nThreadIdx].msg.szMsg, pszMsg, MSG_SIZE);
		gstSyncReplyInfo [nThreadIdx].msg.isUsed = true;
	}

	return true;
}

/**
 * setReplyAlreadySyncReplyInfo
 *
 * 引数 nThreadIdxは request元スレッドです
 */
static void setReplyAlreadySyncReplyInfo (uint8_t nThreadIdx)
{
	if (
		(!gstSyncReplyInfo [nThreadIdx].isUsed) ||
		(gstSyncReplyInfo [nThreadIdx].nReqId == REQUEST_ID_BLANK)
	) {
		/*
		 * setRequestIdSyncReply()を実行してない
		 * Reply待ちしてれば ここには来ないはず
		 */
		return;
	}

	gstSyncReplyInfo [nThreadIdx].isReplyAlready = true;
}

/**
 * clearSyncReplyInfo
 */
static void clearSyncReplyInfo (ST_SYNC_REPLY_INFO *p)
{
	if (!p) {
		return;
	}

	p->nReqId = REQUEST_ID_BLANK;
	p->enRslt = EN_THM_RSLT_IGNORE;
	memset( p->msg.szMsg, 0x00, MSG_SIZE );
	p->msg.isUsed = false;
	p->isReplyAlready = false;
	p->isUsed = false;
}

/**
 * Notify登録
 * 引数 pClientId はout
 *
 * コンテキが登録先スレッド(server側)
 * replyでclientIdを返してください
 * 複数クライアントからの登録は個々のスレッドでid管理しなくていはならない
 */
static bool registerNotify (uint8_t *pnClientId)
{
	/*
	 * getContext->自分のthreadIdx取得->innerInfoを参照して登録先を得る
	 */

	if (!pnClientId) {
		THM_INNER_LOG_E( "invalid argument.\n" );
		return false;
	}

	ST_CONTEXT stContext = getContext();
	if (!stContext.isValid) {
		return false;
	}

	bool isValid = getSeqInfo (stContext.nThreadIdx, stContext.nSeqIdx)->stSeqInitQueWorker.isValidSrcInfo;
	if (!isValid) {
		/* 登録先がわからない */
		THM_INNER_LOG_E( "client is unknown.\n" );
		return false;
	}

	uint8_t nClientThreadIdx = getSeqInfo (stContext.nThreadIdx, stContext.nSeqIdx)->stSeqInitQueWorker.nSrcThreadIdx;


	uint8_t id = setNotifyClientInfo (stContext.nThreadIdx, nClientThreadIdx);
	if (id == NOTIFY_CLIENT_ID_MAX) {
		return false;
	}

	/* clientId 返す */
	if (pnClientId) {
		*pnClientId = id;
	}

	return true;
}

/**
 * Notify登録解除
 *
 * コンテキが登録先スレッド(server側)
 * requestのmsgでclientIdをもらうこと
 */
static bool unregisterNotify (uint8_t nClientId)
{
	if (nClientId >= NOTIFY_CLIENT_ID_MAX) {
		THM_INNER_LOG_E( "invalid argument.\n" );
		return false;
	}

	ST_CONTEXT stContext = getContext();
	if (!stContext.isValid) {
		return false;
	}

	return unsetNotifyClientInfo (stContext.nThreadIdx, nClientId);
}

/**
 * setNotifyClientInfo
 * セットしてクライアントIDを返す
 * NOTIFY_CLIENT_ID_MAX が返ったら空きがない状態 error
 */
static uint8_t setNotifyClientInfo (uint8_t nThreadIdx, uint8_t nClientThreadIdx)
{
	uint8_t id = 0;

	/* lock */
//TODO いらないかも ここは個々のスレッドが処理するから
	pthread_mutex_lock (&gMutexNotifyClientInfo [nThreadIdx]);


	/* 空きを探す */
	for (id = 0; id < NOTIFY_CLIENT_ID_MAX; id ++) {
		if (!gstNotifyClientInfo [nThreadIdx][id].isUsed) {
			break;
		}
	}

	if (id == NOTIFY_CLIENT_ID_MAX) {
		/* 空きがない */
		THM_INNER_LOG_E ("clientId is full.\n");
		return id;
	}

	gstNotifyClientInfo [nThreadIdx][id].nThreadIdx = nClientThreadIdx;
	gstNotifyClientInfo [nThreadIdx][id].isUsed = true;


	/* unlock */
//TODO いらないかも ここは個々のスレッドが処理するから
	pthread_mutex_unlock (&gMutexNotifyClientInfo [nThreadIdx]);

	return id;
}

/**
 * unsetNotifyClientInfo
 */
static bool unsetNotifyClientInfo (uint8_t nThreadIdx, uint8_t nClientId)
{
	/* lock */
//TODO いらないかも ここは個々のスレッドが処理するから
	pthread_mutex_lock (&gMutexNotifyClientInfo [nThreadIdx]);

	if (!gstNotifyClientInfo [nThreadIdx][nClientId].isUsed) {
		THM_INNER_LOG_E ("clientId is not use...?");
		return false;
	}

	clearNotifyClientInfo (&gstNotifyClientInfo [nThreadIdx][nClientId]);

	/* unlock */
//TODO いらないかも ここは個々のスレッドが処理するから
	pthread_mutex_unlock (&gMutexNotifyClientInfo [nThreadIdx]);

	return true;
}

/**
 * notifyInner
 *
 * 引数 nThreadIdx は宛先です
 */
static bool notifyInner (
	uint8_t nThreadIdx,
	uint8_t nClientId,
	ST_CONTEXT *pstContext,
	uint8_t *pszMsg
)
{
	/* lock */
	pthread_mutex_lock (&gMutexWorker [nThreadIdx]);

	/* キューに入れる */
	if (!enQueWorker (nThreadIdx, SEQ_IDX_BLANK, EN_QUE_TYPE_NOTIFY,
							pstContext, REQUEST_ID_BLANK, EN_THM_RSLT_IGNORE, nClientId, pszMsg)) {
		/* unlock */
		pthread_mutex_unlock (&gMutexWorker [nThreadIdx]);

		THM_INNER_LOG_E ("enQueWorker() is failure.\n");
		return false;
	}


	/*
	 * Notify
	 * cond signal ---> workerThread
	 */
	pthread_cond_signal (&gCondWorker [nThreadIdx]);


	/* unlock */
	pthread_mutex_unlock (&gMutexWorker [nThreadIdx]);

	return true;
}

/**
 * Notify送信
 * 公開
 */
static bool notify (uint8_t nClientId, uint8_t *pszMsg)
{
	/*
	 * getContext->自分のthreadIdx取得-> notifyClientInfoをみて trueだったらenque
	 */

	if (nClientId >= NOTIFY_CLIENT_ID_MAX) {
		THM_INNER_LOG_E ("invalid argument.\n");
		return false;
	}

	ST_CONTEXT stContext = getContext();
	if (!stContext.isValid) {
		return false;
	}

	if (!gstNotifyClientInfo [stContext.nThreadIdx][nClientId].isUsed) {
		THM_INNER_LOG_E ("clientId is not use.");
		return false;
	}

	uint8_t nClientThreadIdx = gstNotifyClientInfo [stContext.nThreadIdx][nClientId].nThreadIdx;


	/* Notify投げる */
	if (notifyInner (nClientThreadIdx, nClientId, &stContext, pszMsg)) {
		return false;
	}

	return true;
}

/**
 * clearNotifyClientInfo
 */
static void clearNotifyClientInfo (ST_NOTIFY_CLIENT_INFO *p)
{
	if (!p) {
		return;
	}

	p->nThreadIdx = 0;
	p->isUsed = false;
}

/**
 * setSectId
 * 次のsectIdをセットする
 * 公開用
 */
static void setSectId (uint8_t nSectId, EN_THM_ACT enAct)
{
	if ((nSectId < SECT_ID_INIT) || (nSectId >= SECT_ID_MAX)) {
		THM_INNER_LOG_E ("arg(sectId) is invalid.\n");
		return;
	}

	if ((enAct < EN_THM_ACT_INIT) || (enAct >= EN_THM_ACT_MAX)) {
		THM_INNER_LOG_E ("arg(enAct) is invalid.\n");
		return;
	}

	if (enAct == EN_THM_ACT_INIT) {
		/* 自分でEN_THM_ACT_INITに設定はできない */
		THM_INNER_LOG_E ("set EN_THM_ACT_INIT can not on their own.\n");
		return;
	}

	ST_CONTEXT stContext = getContext();
	if (stContext.isValid) {
		uint8_t nSeqIdx = stContext.nSeqIdx;
		getSeqInfo (stContext.nThreadIdx, nSeqIdx)->nSectId = nSectId;

		if (enAct == EN_THM_ACT_DONE) {
			enAct = EN_THM_ACT_INIT;
		}
		getSeqInfo (stContext.nThreadIdx, nSeqIdx)->enAct = enAct;
	}
}

/**
 * getSectId
 * 公開用
 */
static uint8_t getSectId (void)
{
	ST_CONTEXT stContext = getContext();
	if (stContext.isValid) {
		uint8_t nSeqIdx = stContext.nSeqIdx;
		if (getSeqInfo (stContext.nThreadIdx, nSeqIdx)->enAct == EN_THM_ACT_INIT) {
			getSeqInfo (stContext.nThreadIdx, nSeqIdx)->nSectId = SECT_ID_INIT;
			return SECT_ID_INIT;
		} else {
			return getSeqInfo (stContext.nThreadIdx, nSeqIdx)->nSectId;
		}
	} else {
		return SECT_ID_INIT;
	}
}

/**
 * setTimeout
 * 公開用
 * 登録シーケンスがreturn してから時間計測開始します
 * クリアされるタイミングは 実際にタイムアウトした時と ユーザ自身iがclearTimeoutした時です
 */
static void setTimeout (uint32_t nTimeoutMsec)
{
	setSeqTimeout (nTimeoutMsec);
}

/**
 * setSeqTimeout
 */
static void setSeqTimeout (uint32_t nTimeoutMsec)
{
	if ((nTimeoutMsec < SEQ_TIMEOUT_BLANK) || (nTimeoutMsec >= SEQ_TIMEOUT_MAX)) {
		THM_INNER_LOG_E ("arg is invalid.\n");
		return;
	}

	ST_CONTEXT stContext = getContext();
	if (stContext.isValid) {
		uint8_t nSeqIdx = stContext.nSeqIdx;
		/* ここでは値のみ */
		getSeqInfo (stContext.nThreadIdx, nSeqIdx)->timeout.nVal = nTimeoutMsec;
	}
}

/**
 * enableSeqTimeout
 * Seqタイムアウトを仕掛ける
 */
static void enableSeqTimeout (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	if (getSeqInfo (nThreadIdx, nSeqIdx)->timeout.enState != EN_TIMEOUT_STATE_INIT) {
		/*
		 * setTimeoutしてsectionまたいでWAITしている時とかは
		 * 既に設定済なので何もしない よって上書きされない
		 * 1seq内では seqTimeout1つのみ設定可能です
		 */
		return;
	}

	uint32_t nTimeout = getSeqInfo (nThreadIdx, nSeqIdx)->timeout.nVal;
	time_t add_sec = nTimeout / 1000;
	long add_nsec  = (nTimeout % 1000) * 1000000;
	THM_INNER_LOG_I ("add_sec:%ld  add_nsec:%ld\n", add_sec, add_nsec);

	struct timeval stNowTimeval = {0};
	getTimeOfDay (&stNowTimeval);
	time_t now_sec = stNowTimeval.tv_sec;
	long now_nsec  = stNowTimeval.tv_usec * 1000;

	getSeqInfo (nThreadIdx, nSeqIdx)->timeout.stTime.tv_sec = now_sec + add_sec + ((now_nsec + add_nsec) / 1000000000);
	getSeqInfo (nThreadIdx, nSeqIdx)->timeout.stTime.tv_nsec = (now_nsec + add_nsec) % 1000000000;
	getSeqInfo (nThreadIdx, nSeqIdx)->timeout.enState = EN_TIMEOUT_STATE_MEAS;
	THM_INNER_LOG_I (
		"timeout.stTime.tv_sec:%ld  timeout.stTime.tv_nsec:%ld\n",
		getSeqInfo (nThreadIdx, nSeqIdx)->timeout.stTime.tv_sec,
		getSeqInfo (nThreadIdx, nSeqIdx)->timeout.stTime.tv_nsec
	);
}

/**
 * checkSeqTimeout
 * タイムアウトしたかどうかの確認して キュー入れる
 * 
 * 一連の登録シーケンス実行後にチェック
 */
static void checkSeqTimeout (uint8_t nThreadIdx)
{
	uint8_t nSeqNum = gstInnerInfo [nThreadIdx].nSeqNum;
	int i = 0;
	for (i = 0; i < nSeqNum; i ++) {
		/* EN_TIMEOUT_STATE_PASSEDの場合はすでにqueuing済されてるはず */
		if (getSeqInfo (nThreadIdx, i)->timeout.enState == EN_TIMEOUT_STATE_MEAS) {
			if (isSeqTimeoutFromSeqIdx (nThreadIdx, i)) {
				/* 自スレッドにSeqタイムアウトのキューを入れる */
				getSeqInfo (nThreadIdx, i)->timeout.enState = EN_TIMEOUT_STATE_PASSED;
				enQueSeqTimeout (nThreadIdx, i);
			}
		}
	}
}

/**
 * isSeqTimeoutFromSeqIdx
 */
static bool isSeqTimeoutFromSeqIdx (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	bool isTimeout = false;
	struct timeval stNowTimeval = {0};
	getTimeOfDay (&stNowTimeval);
	time_t now_sec = stNowTimeval.tv_sec;
	long now_nsec  = stNowTimeval.tv_usec * 1000;

	time_t timeout_sec = getSeqInfo (nThreadIdx, nSeqIdx)->timeout.stTime.tv_sec;
	long timeout_nsec  = getSeqInfo (nThreadIdx, nSeqIdx)->timeout.stTime.tv_nsec;

	time_t diff_sec = 0;
	long diff_nsec = timeout_nsec - now_nsec;
	if (diff_nsec >= 0) {
		diff_sec = timeout_sec - now_sec;
	} else {
		diff_sec = timeout_sec -1 - now_sec;
		diff_nsec += 1000000000;
	}

	if (diff_sec > 0) {
		/* まだタイムアウトしてない */
		THM_INNER_LOG_I ("not yet timeout(SeqTimeout) threadIdx:[%d] seqIdx:[%d]\n", nThreadIdx, nSeqIdx);

	} else if (diff_sec == 0) {
		if (diff_nsec > 0) {
			/* まだタイムアウトしてない 1秒切ってる */
			THM_INNER_LOG_I ("not yet timeout - off the one second (SeqTimeout) threadIdx:[%d] seqIdx:[%d]\n", nThreadIdx, nSeqIdx);

		} else if (diff_nsec == 0) {
			/* ちょうどタイムアウト かなりレア... */
			THM_INNER_LOG_N ("just timedout(SeqTimeout) threadIdx:[%d] seqIdx:[%d]\n", nThreadIdx, nSeqIdx);
			isTimeout = true;

		} else {
			/* ありえない */
			THM_INNER_LOG_E ("BUG: diff_nsec is minus value.\n");
		}

	} else {
		/* 既にタイムアウトしてる... */
		THM_INNER_LOG_N ("already timedout(SeqTimeout) threadIdx:[%d] seqIdx:[%d]\n", nThreadIdx, nSeqIdx);
		isTimeout = true;
	}

	return isTimeout;
}

/**
 * enQueSeqTimeout
 */
static bool enQueSeqTimeout (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	ST_CONTEXT stContext = getContext();

	if (!enQueWorker( nThreadIdx, nSeqIdx, EN_QUE_TYPE_SEQ_TIMEOUT,
							&stContext, REQUEST_ID_BLANK, EN_THM_RSLT_SEQ_TIMEOUT, NOTIFY_CLIENT_ID_BLANK, NULL )) {
		THM_INNER_LOG_E( "enQueWorker() is failure.\n" );
		return false;
	}

	return true;
}

/**
 * searchNearestSeqTimeout
 * Seqタイムアウト仕掛り中のもので 最も近い時間のものを探す
 * ST_SEQ_INFO で返します
 * タイムアウト仕掛けてなければnullを返します
 *
 * cond_waitする直前にチェックします
 * pthread_cond_wait か pthread_cond_timedwaitにするかの判断
 */
static ST_SEQ_INFO *searchNearestSeqTimeout (uint8_t nThreadIdx)
{
	time_t timeout_sec_min = 0;
	time_t timeout_nsec_min = 0;
	time_t timeout_sec = 0;
	long timeout_nsec  = 0;
	uint8_t nRtnSeqIdx = SEQ_IDX_MAX;

	uint8_t nSeqNum = gstInnerInfo [nThreadIdx].nSeqNum;
	int i = 0;
	for (i = 0; i < nSeqNum; i ++) {
		if (getSeqInfo (nThreadIdx, i)->timeout.enState == EN_TIMEOUT_STATE_MEAS) {

			if ((timeout_sec_min == 0) && (timeout_nsec_min == 0)) {
				// 初回
				timeout_sec_min = getSeqInfo (nThreadIdx, i)->timeout.stTime.tv_sec;
				timeout_nsec_min = getSeqInfo (nThreadIdx, i)->timeout.stTime.tv_nsec;
				nRtnSeqIdx = i;

			} else {
				timeout_sec = getSeqInfo (nThreadIdx, i)->timeout.stTime.tv_sec;
				timeout_nsec = getSeqInfo (nThreadIdx, i)->timeout.stTime.tv_nsec;

				if (timeout_sec < timeout_sec_min) {
					timeout_sec_min = timeout_sec;
					timeout_nsec_min = timeout_nsec;
					nRtnSeqIdx = i;

				} else if (timeout_sec == timeout_sec_min) {
					if (timeout_nsec < timeout_nsec_min) {
						timeout_sec_min = timeout_sec;
						timeout_nsec_min = timeout_nsec;
						nRtnSeqIdx = i;
					}
				}
			}
		}
	}

	if (nRtnSeqIdx == SEQ_IDX_MAX) {
		/* タイムアウト仕掛かけてない */
		return NULL;
	} else {
		return getSeqInfo (nThreadIdx, nRtnSeqIdx);
	}
}

/**
 * checkSeqTimeoutFromCondTimedwait
 * EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAITの状態をみつけてキューを入れる
 *
 * pthread_cond_timedwaitからタイムアウトで戻った時に使います
 */
static void checkSeqTimeoutFromCondTimedwait (uint8_t nThreadIdx)
{
	uint8_t nSeqNum = gstInnerInfo [nThreadIdx].nSeqNum;
	int i = 0;
	int n = 0;
	for (i = 0; i < nSeqNum; i ++) {
		if (getSeqInfo (nThreadIdx, i)->timeout.enState == EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT) {
			n ++;
		}
	}

	if (n != 1) {
		/* 	複数存在した... バグ */
		THM_INNER_LOG_E ("BUG: state(EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT) is multiple. n=[%d]\n", n);
		return;
	}

	/* 自スレッドにSeqタイムアウトのキューを入れる */
	enQueSeqTimeout (nThreadIdx, i);
}

/**
 * checkSeqTimeoutFromNotCondTimedwait
 * pthread_cond_timedwaitから戻った時 タイムアウトでない場合
 * 状態をEN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT ->  EN_TIMEOUT_STATE_MEASに変える
 */
static void checkSeqTimeoutFromNotCondTimedwait (uint8_t nThreadIdx)
{
	uint8_t nSeqNum = gstInnerInfo [nThreadIdx].nSeqNum;
	int i = 0;
	int n = 0;
	for (i = 0; i < nSeqNum; i ++) {
		if (getSeqInfo (nThreadIdx, i)->timeout.enState == EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT) {
			getSeqInfo (nThreadIdx, i)->timeout.enState = EN_TIMEOUT_STATE_MEAS;
			n ++;
		}
	}

	if (n != 1) {
		/* 	複数存在した... バグ */
		THM_INNER_LOG_E ("BUG: state(EN_TIMEOUT_STATE_MEAS_COND_TIMEDWAIT) is multiple. n=[%d]\n", n);
		return;
	}
}

/**
 * clearTimeout
 * 公開用
 */
static void clearTimeout (void)
{
	clearSeqTimeout ();
}

/**
 * clearSeqTimeout
 */
static void clearSeqTimeout (void)
{
	ST_CONTEXT stContext = getContext();
	if (stContext.isValid) {
		clearSeqTimeoutInfo (stContext.nThreadIdx, stContext.nSeqIdx);
	}
}

/**
 * clearSeqTimeoutInfo
 */
static void clearSeqTimeoutInfo (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	ST_SEQ_INFO *pstSeqInfo = getSeqInfo (nThreadIdx, nSeqIdx);

	pstSeqInfo->timeout.enState = EN_TIMEOUT_STATE_INIT;
	pstSeqInfo->timeout.nVal = SEQ_TIMEOUT_BLANK;
	memset (&(pstSeqInfo->timeout.stTime), 0x00, sizeof(struct timespec));
}

/**
 * getSeqInfo
 *
 * seqInfoは同じスレッドからしか操作しないので特に問題ないはず
 */
static ST_SEQ_INFO *getSeqInfo (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	return (gstInnerInfo [nThreadIdx].pstSeqInfo)+nSeqIdx;
}

/**
 * clearSeqInfo
 */
static void clearSeqInfo (ST_SEQ_INFO *p)
{
	if (!p) {
		return;
	}

	p->nSeqIdx = SEQ_IDX_BLANK;

	p->nSectId = SECT_ID_INIT;
	p->enAct = EN_THM_ACT_INIT;
#ifndef _MULTI_REQUESTING
	p->nReqId = REQUEST_ID_BLANK;
#endif
	clearQueWorker (&(p->stSeqInitQueWorker));

	p->timeout.enState = EN_TIMEOUT_STATE_INIT;
	p->timeout.nVal = SEQ_TIMEOUT_BLANK;
	memset (&(p->timeout.stTime), 0x00, sizeof(struct timespec));
}

/**
 * searchNearestTimeout
 * ReqタイムアウトとSeqタイムアウト仕掛り中のもので 最も近い時間のものを探す
 * タイムアウト仕掛けてなければ EN_NEAREST_TIMEOUT_NONE を返します
 *
 * cond_waitする直前にチェックします
 * pthread_cond_wait か pthread_cond_timedwaitにするかの判断
 */
static EN_NEAREST_TIMEOUT searchNearestTimeout (
	uint8_t nThreadIdx,
	ST_REQUEST_ID_INFO **pstRequestIdInfo,	// out
	ST_SEQ_INFO **pstSeqInfo		// out
)
{
	ST_REQUEST_ID_INFO *pstTmpReqIdInfo = searchNearestReqTimeout (nThreadIdx);
	ST_SEQ_INFO *pstTmpSeqInfo = searchNearestSeqTimeout (nThreadIdx);
    time_t reqTimeout_sec = 0;
    long reqTimeout_nsec  = 0;
    time_t seqTimeout_sec = 0;
    long seqTimeout_nsec  = 0;


	*pstRequestIdInfo = NULL;
	*pstSeqInfo = NULL;

	if (!pstTmpReqIdInfo && !pstTmpSeqInfo) {
		THM_INNER_LOG_I ("EN_NEAREST_TIMEOUT_NONE\n");
		return EN_NEAREST_TIMEOUT_NONE;

	} else if (pstTmpReqIdInfo && !pstTmpSeqInfo) {
		THM_INNER_LOG_I ("EN_NEAREST_TIMEOUT_REQ\n");
		*pstRequestIdInfo = pstTmpReqIdInfo;
		return EN_NEAREST_TIMEOUT_REQ;

	} else if (!pstTmpReqIdInfo && pstTmpSeqInfo) {
		THM_INNER_LOG_I ("EN_NEAREST_TIMEOUT_SEQ\n");
		*pstSeqInfo = pstTmpSeqInfo;
		return EN_NEAREST_TIMEOUT_SEQ;

	} else {
		/* Req,Seqタイムアウト両方とも仕掛かっているので 値を比較します */

		reqTimeout_sec = pstTmpReqIdInfo->timeout.stTime.tv_sec;
		reqTimeout_nsec = pstTmpReqIdInfo->timeout.stTime.tv_nsec;
		seqTimeout_sec = pstTmpSeqInfo->timeout.stTime.tv_sec;
		seqTimeout_nsec = pstTmpSeqInfo->timeout.stTime.tv_nsec;

		if (reqTimeout_sec < seqTimeout_sec) {
			THM_INNER_LOG_I ("both timeout enabled. -> EN_NEAREST_TIMEOUT_REQ\n");
			*pstRequestIdInfo = pstTmpReqIdInfo;
			return EN_NEAREST_TIMEOUT_REQ;

		} else if (reqTimeout_sec > seqTimeout_sec) {
			THM_INNER_LOG_I ("both timeout enabled. -> EN_NEAREST_TIMEOUT_SEQ\n");
			*pstSeqInfo = pstTmpSeqInfo;
			return EN_NEAREST_TIMEOUT_SEQ;

		} else {
			/* 整数値等しい場合 */

			if (reqTimeout_nsec < seqTimeout_nsec) {
				THM_INNER_LOG_I ("both timeout enabled. -> EN_NEAREST_TIMEOUT_REQ (integer value equal)\n");
				*pstRequestIdInfo = pstTmpReqIdInfo;
				return EN_NEAREST_TIMEOUT_REQ;

			} else if (reqTimeout_nsec > seqTimeout_nsec) {
				THM_INNER_LOG_I ("both timeout enabled. -> EN_NEAREST_TIMEOUT_SEQ (integer value equal)\n");
				*pstSeqInfo = pstTmpSeqInfo;
				return EN_NEAREST_TIMEOUT_SEQ;

			} else {
				/* 小数点以下も同じ reqSeqTimeoutを返しておく */
				THM_INNER_LOG_I ("both timeout enabled. -> equal !!!\n");
				*pstRequestIdInfo = pstTmpReqIdInfo;
				return EN_NEAREST_TIMEOUT_REQ;
			}
		}

	}
}

/**
 * addExtInfoList
 * pthread_tをキーとしたリスト
 * キーはuniqなものとして扱う前提 (1threadにつき1つのデータをもつ)
 */
static void addExtInfoList (ST_EXTERNAL_CONTROL_INFO *pstExtInfo)
{
	/* lock */
	pthread_mutex_lock (&gMutexOpeExtInfoList);

	if (!gpstExtInfoListTop) {
		// リストが空
		gpstExtInfoListTop = pstExtInfo;
		gpstExtInfoListBtm = pstExtInfo;

	} else {
		// 末尾につなぎます
		gpstExtInfoListBtm->pNext = pstExtInfo;
		gpstExtInfoListBtm = pstExtInfo;
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeExtInfoList);
}

/**
 * searchExtInfoList
 */
static ST_EXTERNAL_CONTROL_INFO *searchExtInfoList (pthread_t key)
{
	ST_EXTERNAL_CONTROL_INFO *pstExtInfoTmp = NULL;

	/* lock */
	pthread_mutex_lock (&gMutexOpeExtInfoList);

	if (!gpstExtInfoListTop) {
		// リストが空

	} else {
		pstExtInfoTmp = gpstExtInfoListTop;

		while (pstExtInfoTmp) {
			if (pthread_equal (pstExtInfoTmp->nPthreadId, key)) {
				// keyが一致
				break;

			} else {
				// next set
				pstExtInfoTmp = pstExtInfoTmp->pNext;
			}
		}
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeExtInfoList);

	return pstExtInfoTmp;
}

/**
 * searchExtInfoListFromRequestId
 */
static ST_EXTERNAL_CONTROL_INFO *searchExtInfoListFromRequestId (uint32_t nReqId)
{
	ST_EXTERNAL_CONTROL_INFO *pstExtInfoTmp = NULL;

	/* lock */
	pthread_mutex_lock (&gMutexOpeExtInfoList);

	if (!gpstExtInfoListTop) {
		// リストが空

	} else {
		pstExtInfoTmp = gpstExtInfoListTop;

		while (pstExtInfoTmp) {
			if (pstExtInfoTmp->nReqId == nReqId) {
				// reqIdが一致
				break;

			} else {
				// next set
				pstExtInfoTmp = pstExtInfoTmp->pNext;
			}
		}
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeExtInfoList);

	return pstExtInfoTmp;
}

/**
 * deleteExtInfoList
 */
static void deleteExtInfoList (pthread_t key)
{
	ST_EXTERNAL_CONTROL_INFO *pstExtInfoTmp = NULL;
	ST_EXTERNAL_CONTROL_INFO *pstExtInfoBef = NULL;
	ST_EXTERNAL_CONTROL_INFO *pstExtInfoDel = NULL;

	/* lock */
	pthread_mutex_lock (&gMutexOpeExtInfoList);

	if (!gpstExtInfoListTop) {
		// リストが空

	} else {
		pstExtInfoTmp = gpstExtInfoListTop;

		while (pstExtInfoTmp) {
			if (pthread_equal (pstExtInfoTmp->nPthreadId, key)) {
				// keyが一致

				// 後でfreeするため保存
				pstExtInfoDel = pstExtInfoTmp;

				if (pstExtInfoTmp == gpstExtInfoListTop) {
					// topの場合

					if (pstExtInfoTmp->pNext) {
						gpstExtInfoListTop = pstExtInfoTmp->pNext;
						pstExtInfoTmp->pNext = NULL;
					} else {
						// リストが１つ -> リストが空になるはず
						gpstExtInfoListTop = NULL;
						gpstExtInfoListBtm = NULL;
						pstExtInfoTmp->pNext = NULL;
					}

				} else if (pstExtInfoTmp == gpstExtInfoListBtm) {
					// bottomの場合

					if (pstExtInfoBef) {
						// 切り離します
						pstExtInfoBef->pNext = NULL;
						gpstExtInfoListBtm = pstExtInfoBef;
					}

				} else {
					// 切り離します
					pstExtInfoBef->pNext = pstExtInfoTmp->pNext;
					pstExtInfoTmp->pNext = NULL;
				}

				break;

			} else {
				// next set
				pstExtInfoTmp = pstExtInfoTmp->pNext;

				// ひとつ前のアドレス保存
				pstExtInfoBef = pstExtInfoTmp;
			}

		}

		free (pstExtInfoDel);
	}

	/* unlock */
	pthread_mutex_unlock (&gMutexOpeExtInfoList);
}

/**
 * dumpExtInfoList
 * デバッグ用
 */
static void dumpExtInfoList (void)
{
	ST_EXTERNAL_CONTROL_INFO *pstExtInfoTmp = NULL;
	int n = 0;

	/* lock */
	pthread_mutex_lock (&gMutexOpeExtInfoList);

	THM_INNER_FORCE_LOG_N ("####  dump externalInfoList  ####\n");

	pstExtInfoTmp = gpstExtInfoListTop;
	while (pstExtInfoTmp) {

		THM_INNER_FORCE_LOG_N (" %d: %lu 0x%x  (%p -> %p)\n", n, pstExtInfoTmp->nPthreadId, pstExtInfoTmp->nReqId, pstExtInfoTmp, pstExtInfoTmp->pNext);

		// next set
		pstExtInfoTmp = pstExtInfoTmp->pNext;
		n ++;
	}


	/* unlock */
	pthread_mutex_unlock (&gMutexOpeExtInfoList);
}

/**
 * createExternalCp
 * 外部のスレッドからリクエストしてリプライを受けたい場合は これをリクエスト前に実行しておく
 * 1thread につき 1つ内部情報を保持します
 *
 * 公開 external_if
 */
bool createExternalCp (void)
{
	/* 内部でよばれたら実行しないチェック */
	ST_CONTEXT stContext = getContext();
	if (stContext.isValid) {
		return false;
	}


	ST_EXTERNAL_CONTROL_INFO *pstTmp = searchExtInfoList(pthread_self());
	if (pstTmp) {
		/* このスレッドではすでに登録されている 古いものは消します */
		deleteExtInfoList (pthread_self());
	}

	ST_EXTERNAL_CONTROL_INFO *pstExtInfo = (ST_EXTERNAL_CONTROL_INFO*) malloc (sizeof(ST_EXTERNAL_CONTROL_INFO));
	if (!pstExtInfo) {
		return false;
	}
	clearExternalControlInfo (pstExtInfo);

	addExtInfoList (pstExtInfo);


	return true;
}

/**
 * destroyExternalCp
 * 公開 external_if
 */
void destroyExternalCp (void)
{
	/* 内部でよばれたら実行しないチェック */
	ST_CONTEXT stContext = getContext();
	if (stContext.isValid) {
		return;
	}


	deleteExtInfoList (pthread_self());
}

/**
 * receiveExternal
 * 外部のスレッドからリクエストした場合はこれでリプライを待ち 結果をとります
 *
 * 公開 external_if
 */
ST_THM_SRC_INFO *receiveExternal (void)
{
	/* 内部でよばれたら実行しないチェック */
	ST_CONTEXT stContext = getContext();
	if (stContext.isValid) {
		return NULL;
	}


	ST_EXTERNAL_CONTROL_INFO *pstExtInfo = searchExtInfoList (pthread_self());
	if (!pstExtInfo) {
		return NULL;
	}

	/* lock */
	pthread_mutex_lock (&(pstExtInfo->mutex));

	/* isReplyAlreadyチェックして cond で待つかどうか判断 */
	if (!pstExtInfo->isReplyAlready) {
		/* 相手がまだreplyしてないので cond wait */
		pthread_cond_wait (&(pstExtInfo->cond), &(pstExtInfo->mutex));
	}

	pstExtInfo->isReplyAlready = false;

	/* reqId解放して extInfoのreqIdもクリア */
	releaseRequestId (THREAD_IDX_EXTERNAL, pstExtInfo->nReqId);
	pstExtInfo->nReqId = REQUEST_ID_BLANK;

	/* unlock */
	pthread_mutex_unlock (&(pstExtInfo->mutex));


	return &(pstExtInfo->stThmSrcInfo);
}

/**
 * clearExternalControlInfo
 */
static void clearExternalControlInfo (ST_EXTERNAL_CONTROL_INFO *p)
{
	if (!p) {
		return;
	}

	pthread_mutex_init (&(p->mutex), NULL);
	pthread_cond_init (&(p->cond), NULL);

	p->nPthreadId = pthread_self();
	p->nReqId = REQUEST_ID_BLANK;

	memset (p->szMsgEntity, 0x00, sizeof(MSG_SIZE));
	clearThmSrcInfo (&(p->stThmSrcInfo));

	p->isReplyAlready = false;

	p->pNext = NULL;
}

/**
 * finalize
 * 終了処理
 *
 * 公開 external_if
 */
void finalize (void)
{
//TODO	
}

/**
 * isLogEnable
 */
static bool isLogEnable (void)
{
	return gIsLogEnable;
}

/**
 * enableLog
 */
static void enableLog (void)
{
	gIsLogEnable = true;
}

/**
 * disableLog
 */
static void disableLog (void)
{
	gIsLogEnable = false;
}
