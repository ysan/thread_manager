thread manager
===============

[![Build Status](https://travis-ci.org/ysan/thread_manager.svg?branch=master)](https://travis-ci.org/ysan/thread_manager) [![Coverage Status](https://coveralls.io/repos/github/ysan/thread_manager/badge.svg?branch=master)](https://coveralls.io/github/ysan/thread_manager?branch=master)

Synchronous asynchronous communication framework during multi threads and section sequence manager.


Build and install
------------

	$ git clone https://github.com/ysan/thread_manager.git
	$ cd thread_manager
	$ make
	$ sudo make install INSTALLDIR=/usr/local/
	$ sudo ldconfig

Installation files:

	/usr/local/
	├── include
	│   └── threadmgr
	│       ├── threadmgr_if.h
	│       ├── threadmgr_util.h
	│       ├── ThreadMgrpp.h
	│       ├── ThreadMgr.h
	│       ├── ThreadMgrBase.h
	│       ├── ThreadMgrExternalIf.h
	│       └── ThreadMgrIf.h
	└── lib
	    └── libthreadmgr.so
	    └── libthreadmgrpp.so


Uninstall
------------

	$ sudo make clean INSTALLDIR=/usr/local/


Big picture
------------
![big picture](https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png)


About sequence
------------
The framework first receives queue, identifies the sequence from the sent queue, and calls that message as
an argument to the corresponding sequence. After processing the called sequence, we return control to
the framework and enter the next message wait.
Sequence switching is done every 1 queue, and other sequences can not operate at all.
since sequences on the same thread are always exclusive, exclusion control between sequences is not necessary.

![about sequence](https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png)


How to use
------------
#### Quick start (C++) ####
##### register your class #####
```C++
#include "ThreadMgrpp.h"

// Class that inherits CThreadMgrBase has one posix thread context.
class CMyModuleA : public CThreadMgrBase
{
	// implements your class
};
class CMyModuleB : public CThreadMgrBase
{
	// implements your class
};

// create instance (maximum of queue buffer size: 100)
CMyModuleA _moduleA ((char*)"MyModuleA", 10); // (name, queue buffer size)
CMyModuleB _moduleB ((char*)"MyModuleB", 10);

// instance table
CThreadMgrBase *p_modules [] = {
	&_moduleA,
	&_moduleB,
};

int main (void) {
	CThreadMgr *p_mgr = CThreadMgr::getInstance();

	// register instance table (maximum of registration: 32)
	p_mgr->setup(p_modules, 2);

	// thread manager framework was ready.

	// thread return wait, as daemon process.
	p_mgr->wait();
	p_mgr->teardown();
	return 0;
}

```

##### register your sequences #####
```C++
// Class that inherits CThreadMgrBase has one posix thread context.
class CMyModuleA : public CThreadMgrBase
{
public:
	CMyModuleA (char *pszName, uint8_t nQueNum)
		:CThreadMgrBase (pszName, nQueNum)
	{
		mSeqs [0] = {(PFN_SEQ_BASE)&CMyModuleA::func00, (char*)"func00"};
		mSeqs [1] = {(PFN_SEQ_BASE)&CMyModuleA::func01, (char*)"func01"};
		mSeqs [2] = {(PFN_SEQ_BASE)&CMyModuleA::func02, (char*)"func02"};
		setSeqs (mSeqs, 3); // set sequences
	}

	virtual ~CMyModuleA (void) {}

	// implements your sequences (member functions)
	void func00 (CThreadMgrIf *pIf);
	void func01 (CThreadMgrIf *pIf);
	void func02 (CThreadMgrIf *pIf);

private:
	ST_SEQ_BASE mSeqs [2]; // member function pointers entity
};
```

##### implements your sequences #####
```C++
// one shot echo-reply
void CMyModuleA::func00 (CThreadMgrIf *pIf)
{
	// request message is char strings.
	char *msg = (char*)(pIf->getSrcInfo()->msg.pMsg)

	// send reply (maximum of message size: 256bytes)
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

	// set at the end the follow of the sequence.
	pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
}

// section sequence
void CMyModuleA::func01 (CThreadMgrIf *pIf)
{
	enum {
		SECTID_REQUEST_MODULE_B_FUNC00 = THM_SECT_ID_INIT,
		SECTID_WAIT_REPLY_MODULE_B_FUNC00,
		SECTID_END,
	};

	uint8_t section_id = pIf->getSectId();
	switch (section_id) {
	case SECTID_REQUEST_MODULE_B_FUNC00:

		// request to CMyModuleB::func00
		// add a message to the request (maximum of message size: 256bytes)
		requestAsync (1, 0); // (thread idx, sequence idx)

		// set next section id, and action.
		// Return control to framework and wait for queue, by EN_THM_ACT_WAIT.
		// During this time the framework can handle another queue. (func00 or func02)
		pIf->setSectId (SECTID_WAIT_REPLY_MODULE_B_FUNC00, EN_THM_ACT_WAIT);
		break;

	case SECTID_WAIT_REPLY_MODULE_B_FUNC00: {

		EN_THM_RSLT r = pIf->getSrcInfo()->enRslt;
		if (r == EN_THM_RSLT_SUCCESS) {
			// success reply
		} else {	
			// error reply
		}

		// EN_THM_ACT_CONTINUE is context continues.
		pIf->setSectId (SECTID_END, EN_THM_ACT_CONTINUE);

		} break;

	case SECTID_END:

		// send reply
		// reply is required for request-reply model.
		if (xxx) {
			pIf->reply (EN_THM_RSLT_SUCCESS);
		} else {
			pIf->reply (EN_THM_RSLT_ERROR);
		}

		// set at the end the follow of the sequence.
		pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
		break;

	default:
		break:
	}
}
```

#### Reference ####
middleware daemon using thread manager.  
[`auto_rec_mini`](https://github.com/ysan/auto_rec_mini)


Linking with an Application
------------
Here is a typical gcc link command.  
  
#### C ####
Include `threadmgr_if.h` `threadmgr_util.h` in your application and link with `libthreadmgr` and `libpthread`.  

	$ gcc myapp.c -o myapp -lthreadmgr -lpthread

#### C++ ####
Include `ThreadMgrpp.h` in your application and link with `libthreadmgr` `libthreadmgrpp` and `libpthread`.  

	$ g++ myapp.cpp -o myapp -lthreadmgr -lthreadmgrpp -lpthread


Platforms
------------
Generic Linux will be ok. (confirmed worked on Ubuntu, Fedora, Raspbian)


License
------------
MIT

