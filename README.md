thread manager
===============
Synchronous asynchronous communication framework during multi threads and section sequence manager.


How to build
--------

`clone` and `make`.

	$ git clone https://github.com/ysan/thread_manager
	$ cd thread_manager
	$ make

When you execute make install, it will install in the cloned local directory.

	$ make install
	$ tree local_build/
	local_build/
	├── include
	│   └── threadmgr
	│       ├── ThreadMgr.h
	│       ├── ThreadMgrBase.h
	│       ├── ThreadMgrExternalIf.h
	│       ├── ThreadMgrIf.h
	│       ├── threadmgr_if.h
	│       └── threadmgr_util.h
	└── lib
	    └── threadmgr
	        ├── libthreadmgr.so
	        └── libthreadmgrpp.so
	
	4 directories, 8 files

For example, if you want to specify the installation destination, please add `INSTALLDIR`.  
And you will need to run `ldconfig`.

	$ sudo make install INSTALLDIR=/usr/local/
	$ sudo ldconfig /usr/local/lib/threadmgr


How to using thread manager
------------
In reference, this is middleware using thread manager.
[`atpp`](https://github.com/ysan/atpp)


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
Generic Linux will be ok. (confirmed worked on Ubuntu, Fedora)

