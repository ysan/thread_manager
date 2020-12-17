thread manager
===============

[![Build Status](https://travis-ci.org/ysan/thread_manager.svg?branch=master)](https://travis-ci.org/ysan/thread_manager) [![Coverage Status](https://coveralls.io/repos/github/ysan/thread_manager/badge.svg?branch=master)](https://coveralls.io/github/ysan/thread_manager?branch=master)

Asynchronous I/O communication framework during multi threads and section sequence manager.  
  
You can communicate threads N-to-N with patterns like request-reply and notify.  

<!--
Generally, the number of callback functions that receive events increases in asynchronous processing, and the readability of the source code deteriorates. However, this framework improves the readability of the source code by enabling the description through a series of processing sequences.
-->

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


Uninstall:

	$ sudo make clean INSTALLDIR=/usr/local/


Big picture
------------
<!-- 
![big picture](https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png)
-->
<img src="https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png" width="75%" height="75%">


About sequence
------------
The framework first receives queue, identifies the sequence from the sent queue, and calls that message as an argument to the corresponding sequence. After processing the called sequence, we return control to the framework and enter the next message wait. Sequence switching is done every 1 queue, and other sequences can not operate at all. since sequences on the same thread are always exclusive, exclusion control between sequences is not necessary.

<!--
![about sequence](https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png)
-->
<img src="https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png" width="75%" height="75%">


How to use
------------
#### Quick start (C++) ####
Here is an excerpt of [`samples`](/samples).  
also one can look at the example implementations in the [`testpp`](/testpp) folder to get an idea of how thread manager is integrated into an application.


##### include library header #####
```C++
#include <threadmgr/ThreadMgrpp.h>
```

##### setup ThreadManager and register your class instances #####

Register your instances with `CThreadMgr::setup`.  
Also, your class should extend `CThreadMgrBase`. It has one posix thread context.


```C++
enum {
	_MODULE_A = 0,
	_MODULE_B,
	_MODULE_C,
	.
	.
};

int main (void) {
	ThreadManager::CThreadMgr *p_mgr = ThreadManager::CThreadMgr::getInstance();

	// create your class instances (maximum of queue buffer size: 100)
	CModuleA *p_moduleA = new CModuleA("ModuleA", 10); // (name, queue buffer size)
	CModuleB *p_moduleB = new CModuleB("ModuleB", 10);
	CModuleC *p_moduleC = new CModuleC("ModuleC", 10);
	.
	.

	std::vector<ThreadManager::CThreadMgrBase*> threads;
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

##### implements your class and register sequences #####

Your class should extend `CThreadMgrBase`. It has one posix thread context.  
Then register your sequences with `CThreadMgrBase::setSeqs` in the constructor.

```C++
enum {
	_SEQ_1 = 0,
	_SEQ_2,
	_SEQ_3,
	.
	.
};

class CModuleA : public ThreadManager::CThreadMgrBase
{
public:
	CModuleA (char *pszName, uint8_t nQueNum) : CThreadMgrBase (pszName, nQueNum) {
		std::vector<ThreadManager::SEQ_BASE_t> seqs;
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::sequence1, "sequence1"}); // enum _SEQ_1
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::sequence2, "sequence2"}); // enum _SEQ_2
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::sequence3, "sequence3"}); // enum _SEQ_3
		.
		.

		// register your sequences (maximum of registration: 64)
		setSeqs (seqs);
	}

	virtual ~CModuleA (void) {}

private:
	// implements your sequences (member functions)
	void sequence1 (ThreadManager::CThreadMgrIf *pIf);
	void sequence2 (ThreadManager::CThreadMgrIf *pIf);
	void sequence3 (ThreadManager::CThreadMgrIf *pIf);
	.
	.

};

class CModuleB : public ThreadManager::CThreadMgrBase
{
	.
	.

class CModuleC : public ThreadManager::CThreadMgrBase
{
	.
	.

```

##### implements your sequences #####
```C++
//  1-shot sequence (simple-echo)
void CModuleA::sequence1 (ThreadManager::CThreadMgrIf *pIf)
{
	// get request message.
	char *msg = (char*)(pIf->getSrcInfo()->msg.pMsg);
	size_t msglen = pIf->getSrcInfo()->msg.size;

	// send reply (maximum of message size: 256bytes)
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);

	// at the end of sequence,
	// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with setSectId().
	pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
}

// separated section sequence
void CModuleB::sequence1 (ThreadManager::CThreadMgrIf *pIf)
{
	enum {
		SECTID_REQ_SEQ1_MODB = THM_SECT_ID_INIT,
		SECTID_WAIT_SEQ1_MODB,
		SECTID_END,
	};

	EN_THM_ACT action;
	uint8_t section_id = pIf->getSectId();
	switch (section_id) {
	case SECTID_REQ_SEQ1_MODB: {

		// request to CModuleB::sequence1
		requestAsync(_MODULE_B, _SEQ_1);

		// set next section_id and action
		section_id = SECTID_WAIT_SEQ1_MODB;
		// wait for reply
		// while waiting this module can execute other sequences.
		action = EN_THM_ACT_WAIT;

		// If you don't want to execute other sequences, call lock() in advance.
		//pIf->lock();
		}
		break;

	case SECTID_REQ_SEQ1_MODB: {
		EN_THM_RSLT r = pIf->getSrcInfo()->enRslt;

		std::cout << "[" << enRslt << "]" << std::endl; // "[1]" --> EN_THM_RSLT_SUCCESS

		// set next section_id and action
		section_id = SECTID_END;
		action = EN_THM_ACT_CONTINUE;

		// don't forget to unlock() when you call lock()
		//pIf->unlock();
		} break;

	case SECTID_END:
		// send reply
		pIf->reply (EN_THM_RSLT_SUCCESS);

		// at the end of sequence,
		// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with setSectId().
		section_id = THM_SECT_ID_INIT;
		action = EN_THM_ACT_DONE;
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
Include `<threadmgr/threadmgr_if.h>` `<threadmgr/threadmgr_util.h>` in your application and link with `libthreadmgr` and `libpthread`.  

	$ gcc myapp.c -o myapp -lthreadmgr -lpthread

#### C++ ####
Include `<threadmgr/ThreadMgrpp.h>` in your application and link with `libthreadmgr` `libthreadmgrpp` and `libpthread`.  

	$ g++ myapp.cpp -o myapp -lthreadmgr -lthreadmgrpp -lpthread


Platforms
------------
Generic Linux will be ok. (confirmed worked on Ubuntu, Fedora, Raspbian)


License
------------
MIT

