# thread manager

[![Build Status](https://github.com/ysan/thread_manager/actions/workflows/ci.yml/badge.svg)](https://github.com/ysan/thread_manager/actions/workflows/ci.yml) [![Coverage Status](https://coveralls.io/repos/github/ysan/thread_manager/badge.svg?branch=master)](https://coveralls.io/github/ysan/thread_manager?branch=master)
<a href="https://scan.coverity.com/projects/thread_manager">
<img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/22807/badge.svg"/>
</a>

Asynchronous communication framework during multi threads and section sequence manager.

Communicate threads N-to-N with patterns like request-reply and notify.

<!--
Generally, the number of callback functions that receive events increases in asynchronous processing, and the readability of the source code deteriorates. However, this framework improves the readability of the source code by enabling the description through a series of processing sequences.
-->

## Build and install

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

## Components overview

<!--
![big picture](https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png)
-->
<img src="https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png" width="75%" height="75%">

<!--
## About sequence

The framework first receives queue, identifies the sequence from the sent queue, and calls that message as an argument to the corresponding sequence. After processing the called sequence, we return control to the framework and enter the next message wait. Sequence switching is done every 1 queue, and other sequences can not operate at all. since sequences on the same thread are always exclusive, exclusion control between sequences is not necessary.
-->

<!--
![about sequence](https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png)
-->
<!--
<img src="https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png" width="75%" height="75%">
-->

## How to use

#### Quick start (C++)

Here is an excerpt of [`samples`](/samples).  
reqrep sequence diagram  
<img src="https://github.com/ysan/thread_manager/blob/master/etc/sample_seq_reqrep.png" width="50%" height="50%">

also one can look at the example implementations in the [`testpp`](/testpp) folder to get an idea of how thread manager is integrated into an application.

##### Include library header

```C++
#include <threadmgr/ThreadMgrpp.h>
```

##### Setup and register your class instances

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
	threadmgr::CThreadMgr *p_mgr = threadmgr::CThreadMgr::get_instance();

	// create your class instances (maximum of queue buffer size: 100)
	auto module_a = std::make_shared <module_a>(std::move("module_a"), 10); // (name, queue buffer size)
	auto module_b = std::make_shared <module_b>(std::move("module_b"), 10);
	auto module_c = std::make_shared <module_c>(std::move("module_c"), 10);
	.
	.

	std::vector<std::shared_ptr<threadmgr::CThreadMgrBase>> threads;
	threads.push_back(module_a); // enum _MODULE_A
	threads.push_back(module_b); // enum _MODULE_B
	threads.push_back(module_c); // enum _MODULE_C
	.
	.

	// register your instances (maximum of registration: 32)
	p_mgr->setup(threads);

	// ThreadManager-framework was ready.

	// thread return wait, as daemon process.
	p_mgr->wait();
	p_mgr->teardown();

	return 0;
}
```

##### Implements your class and register sequences

Your class should extend `CThreadMgrBase`. It has one posix thread context.  
Then register your sequences with `CThreadMgrBase::set_sequences` in the constructor.

```C++
enum {
	_SEQ_1 = 0,
	_SEQ_2,
	_SEQ_3,
	.
	.
};

class module_a : public threadmgr::CThreadMgrBase
{
public:
	module_a (std::string name, uint8_t que_max) : CThreadMgrBase (name.c_str(), que_max) {
		std::vector<threadmgr::sequence_t> sequences;
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence1(p_if);}, std::move("sequence1")}); // enum _SEQ_1
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence2(p_if);}, std::move("sequence2")}); // enum _SEQ_2
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence2(p_if);}, std::move("sequence3")}); // enum _SEQ_3
		.
		.

		// register your sequences (maximum of registration: 64)
		set_sequences (sequences);
	}

	virtual ~module_a (void) {
		reset_sequences ();
	}

private:
	// implements your sequences (member functions)
	void sequence1 (threadmgr::CThreadMgrIf *p_if) {
		.
		.
	}

	void sequence2 (threadmgr::CThreadMgrIf *p_if) {
		.
		.
	}

	void sequence3 (threadmgr::CThreadMgrIf *p_if) {
		.
		.
	}
	.
	.

};

class module_b : public threadmgr::CThreadMgrBase
{
	.
	.

class module_c : public threadmgr::CThreadMgrBase
{
	.
	.

```

##### Implements your sequences

```C++
//  1-shot sequence (simple-echo)
void module_a::sequence1 (threadmgr::CThreadMgrIf *p_if) {
	// get request message.
	char *msg = reinterpret_cast<char*>(p_if->get_source().get_message().data());
	size_t msglen = p_if->get_source().get_message().length();

	// send reply (maximum of message size: 256bytes)
	p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(msg), msglen);

 	// at the end of sequence,
	// set threadmgr::section_id::init, threadmgr::action::done with set_section_id().
 	p_if->set_section_id (threadmgr::section_id::init, threadmgr::action::done);
}

// separated section sequence
void module_b::sequence2 (threadmgr::CThreadMgrIf *p_if) {
	enum {
		SECTID_REQ_MODB_SEQ2 = threadmgr::section_id::init,
		SECTID_WAIT_MODB_SEQ2,
		SECTID_END,
	};

	threadmgr::action action;
	threadmgr::section_id::type section_id = p_if->get_sect_id();
	switch (section_id) {
	case SECTID_REQ_MODB_SEQ2: {

		// request to module_b::sequence2
		request_async(_MODULE_B, _SEQ_2);

		std::cout << __PRETTY_FUNCTION__ << " request module_b::sequence1" << std::endl;

		// set next section_id and action
		section_id = SECTID_WAIT_MODB_SEQ2;
		// wait for reply
		// while waiting this module can execute other sequences.
		action = threadmgr::action::wait;

		// If you don't want to execute other sequences, call lock() in advance.
		//p_if->lock();
		}
		break;

	case SECTID_WAIT_MODB_SEQ2: {
		threadmgr::result rslt = p_if->get_source().get_result();

		std::cout << __PRETTY_FUNCTION__ << " reply module_b::sequence1 [" << static_cast<int>(rslt) << "]" << std::endl; // "[1]" --> success

		// set next section_id and action
		section_id = SECTID_END;
		action = threadmgr::action::continue_;

		// don't forget to unlock() when you call lock()
		//p_if->unlock();
		}
		break;

	case SECTID_END:
		// send reply
		p_if->reply (threadmgr::result::success);

		// at the end of sequence,
		// set threadmgr::section_id::init, threadmgr::action::done with set_section_id().
		section_id = threadmgr::section_id::init;
		action = threadmgr::action::done;
		break;

	default:
		break;
	}

	p_if->set_section_id (section_id, action);
}
```

#### Reference

Experimental implementation using thread manager.  
[`auto_rec_mini`](https://github.com/ysan/auto_rec_mini)

## Linking with an Application

Here is a typical gcc link command.

#### C

Include `<threadmgr/threadmgr_if.h>` `<threadmgr/threadmgr_util.h>` in your application and link with `libthreadmgr` and `libpthread`.

    $ gcc myapp.c -o myapp -lthreadmgr -lpthread

#### C++

Include `<threadmgr/ThreadMgrpp.h>` in your application and link with `libthreadmgr` `libthreadmgrpp` and `libpthread`.

    $ g++ myapp.cpp -o myapp -lthreadmgr -lthreadmgrpp -lpthread -std=c++11

## Platforms

Will work on generic linux destributions. (confirmed worked on Ubuntu, Fedora, Raspberry Pi OS)

## License

MIT
