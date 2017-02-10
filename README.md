thread manager
===============
Synchronous asynchronous communication framework during multi threads and section sequence manager


Platforms
------------
Linux (confirmed worked on Ubuntu, Fedora)


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

