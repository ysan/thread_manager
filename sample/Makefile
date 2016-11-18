#
#   Defines
#
BASEDIR		:=	../

LIBS		:= \
	-lpthread \
	-L$(BASEDIR)/sample/modules/thread_a -lthread_a \
	-L$(BASEDIR)/sample/modules/thread_b -lthread_b \
	-L$(BASEDIR)/sample/modules/thread_c -lthread_c\
	-L$(BASEDIR)/src -lthreadmgr \

LDFLAGS		:= \
	-Wl,-rpath=`pwd`/$(BASEDIR)/src \

APPEND_OBJS	:= \
	$(BASEDIR)/sample/main/main.o \


SUBDIRS		:= \
	modules \
	main\


#
#   Target object
#
TARGET_NAME	:=	sample

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	EXEC


#
#   Configurations
#
include $(BASEDIR)/Config.make

