#
#   Defines
#
BASEDIR		:=	../

LIBS		:= \
	-lpthread \
	-L$(BASEDIR)/test/modules/thread_a -lthread_a \
	-L$(BASEDIR)/test/modules/thread_b -lthread_b \
	-L$(BASEDIR)/test/modules/thread_c -lthread_c\
	-L$(BASEDIR)/threadmgr -lthreadmgr \

LDFLAGS		:= \
#	-Wl,-rpath=`pwd`/$(BASEDIR)/threadmgr \

APPEND_OBJS	:= \
	$(BASEDIR)/test/main/main.o \


SUBDIRS		:= \
	modules \
	main\


#
#   Target object
#
TARGET_NAME	:=	test
NOT_INSTALL :=	YES

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	EXEC


#
#   Configurations
#
include $(BASEDIR)/Config.mak

