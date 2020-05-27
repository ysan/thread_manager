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
<!-- 
![big picture](https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png)
-->
<img src="https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png" width="75%" height="75%">


About sequence
------------
The framework first receives queue, identifies the sequence from the sent queue, and calls that message as
an argument to the corresponding sequence. After processing the called sequence, we return control to
the framework and enter the next message wait.
Sequence switching is done every 1 queue, and other sequences can not operate at all.
since sequences on the same thread are always exclusive, exclusion control between sequences is not necessary.

<!--
![about sequence](https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png)
-->
<img src="https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png" width="75%" height="75%">


How to use
------------
#### Quick start (C++) ####
##### include libary header #####
```C++
#include "ThreadMgrpp.h"
```

##### setup ThreadManager and register your class instances #####

Register your instances with `CThreadMgr::setup`.  
Also, your class should extend `CThreadMgrBase`. It has one posix thread context. >> ([here](#jump1))


```C++
enum {
	_MODULE_A = 0,
	_MODULE_B,
	_MODULE_C,
};

using namespace ThreadManager;
int main (void) {
	CThreadMgr *p_mgr = CThreadMgr::getInstance();

	// create your class instances (maximum of queue buffer size: 100)
	CModuleA *p_moduleA = new CModuleA((char*)"ModuleA", 10); // (name, queue buffer size)
	CModuleB *p_moduleB = new CModuleB((char*)"ModuleB", 10);
	CModuleC *p_moduleC = new CModuleC((char*)"ModuleC", 10);
	.
	.

	std::vector<CThreadMgrBase*> threads;
	threads.push_back(p_moduleA); // enum _MODULE_A
	threads.push_back(p_moduleB); // enum _MODULE_B
	threads.push_back(p_moduleC); // enum _MODULE_C
	.
	.

	// register your instances (maximum of registration: 32)
	p_mgr->setup(threads);

	// ThreadManager-framework was ready.

	// thread return wait, as daemon process.
	p_mgr->wait();
	p_mgr->teardown();

	for (const auto &th : threads)
		delete th;
	return 0;
}
```

<a href="#jump1"></a>
##### implements your class and register sequences #####

Your class should extend `CThreadMgrBase`. It has one posix thread context.  
Then register your sequences with `CThreadMgrBase::setSeqs` in the constructor.

```C++
enum {
	_SEQ_1 = 0,
	_SEQ_2,
	_SEQ_3,
};

using namespace ThreadManager;
class CModuleA : public CThreadMgrBase
{
public:
	CModuleA (char *pszName, uint8_t nQueNum)
		:CThreadMgrBase (pszName, nQueNum)
	{
		vector<ST_SEQ_BASE> seqs;
		seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::sequence1, (char*)"sequence1"}); // enum _SEQ_1
		seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::sequence2, (char*)"sequence2"}); // enum _SEQ_2
		seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::sequence3, (char*)"sequence3"}); // enum _SEQ_3

		// register your sequences (maximum of registration: 64)
		setSeqs (seqs);
	}

	virtual ~CModuleA (void) {}

	// implements your sequences (member functions)
	void sequence1 (CThreadMgrIf *pIf);
	void sequence2 (CThreadMgrIf *pIf);
	void sequence3 (CThreadMgrIf *pIf);
};
```

##### implements your sequences #####
```C++
// one shot echo-reply
void CModuleA::sequence1 (CThreadMgrIf *pIf)
{
	// request message is char strings.
	char *msg = (char*)(pIf->getSrcInfo()->msg.pMsg)

	// send reply (maximum of message size: 256bytes)
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

	// set at the end the follow of the sequence.
	pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
}

// section type
void CModuleA::sequence2 (CThreadMgrIf *pIf)
{
	enum {
		SECTID_REQUEST_MODULE_B_FUNC00 = THM_SECT_ID_INIT,
		SECTID_WAIT_REPLY_MODULE_B_FUNC00,
		SECTID_END,
	};

	uint8_t section_id = pIf->getSectId();
	switch (section_id) {
	case SECTID_REQUEST_MODULE_B_FUNC00:

		// request to CModuleB::sequence1
		// add a message to the request (maximum of message size: 256bytes)
		requestAsync (_MODULE_B, _SEQ_1); // (thread idx, sequence idx)

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

