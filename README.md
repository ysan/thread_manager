thread manager [![Build Status](https://travis-ci.org/ysan/thread_manager.svg?branch=master)](https://travis-ci.org/ysan/thread_manager)
===============
Synchronous asynchronous communication framework during multi threads and section sequence manager.


How to build
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

	
Linking with an C Application
------------
Include `threadmgr_if.h` `threadmgr_util.h` in your application and link with libthreadmgr.  
Here is a typical gcc link command.

	$ gcc myapp.c -o myapp -lthreadmgr


Linking with an C++ Application
------------
Include `ThreadMgrpp.h` in your application and link with `libthreadmgr` `libthreadmgrpp`.  
Here is a typical g++ link command.

	$ g++ myapp.cpp -o myapp -lthreadmgr -lthreadmgrpp


How to using thread manager
------------
In reference, this is middleware using thread manager.  
[`auto_rec_mini`](https://github.com/ysan/auto_rec_mini)


Big picture
------------
![big picture](https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png)


About sequence
------------
The framework first receives queue, identifies the sequence from the sent queue, and calls that message as
an argument to the corresponding sequence. After processing the called sequence, we return control to
the framework and enter the next message wait.
Sequence switching is done every 1 queue, and other sequences can not operate at all.
since sequences on the same thread are always exclusive, exclusion control between threads is not necessary.

![about sequence](https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png)


Platforms
------------
Generic Linux will be ok. (confirmed worked on Ubuntu, Fedora, Raspbian)

