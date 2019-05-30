thread manager
===============
Synchronous asynchronous communication framework during multi threads and section sequence manager.


How to build
--------

Clone and make.

	$ git clone https://github.com/ysan/thread_manager
	$ cd thread_manager
	$ make

When you execute make install, it will install in the cloned local directory.

	$ make install
	$ ls -R local_build/
	local_build/:
	include  lib
	
	local_build/include:
	threadmgr
	
	local_build/include/threadmgr:
	ThreadMgr.h  ThreadMgrBase.h  ThreadMgrExternalIf.h  ThreadMgrIf.h  threadmgr_if.h  threadmgr_util.h
	
	local_build/lib:
	threadmgr
	
	local_build/lib/threadmgr:
	libthreadmgr.so  libthreadmgrpp.so

If you want to specify the installation destination, please add INSTALLDIR.

	$ sudo make install INSTALLDIR=/usr/local/


big picture
------------
![big picture](https://github.com/ysan/thread_manager/blob/master/etc/big_picture.png)


about sequence
------------
The framework first receives queue, identifies the sequence from the sent queue, and calls that message as
an argument to the corresponding sequence. After processing the called sequence, we return control to
the framework and enter the next message wait.
Sequence switching is done every 1 queue, and other sequences can not operate at all.
since sequences on the same thread are always exclusive, exclusion control between threads is not necessary.

![about sequence](https://github.com/ysan/thread_manager/blob/master/etc/about_sequence.png)


repository using this library
------------
https://github.com/ysan/notes/tree/master/atpp


Platforms
------------
Generic Linux will be ok. (confirmed worked on Ubuntu, Fedora)

