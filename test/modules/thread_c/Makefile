#
#   Defines
#
BASEDIR		:=	../../../

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)threadmgr \
	-I$(BASEDIR)/test/main \
	-I$(BASEDIR)/test/modules/thread_a \
	-I$(BASEDIR)/test/modules/thread_b \
	-I$(BASEDIR)/test/modules/thread_c \

LIBS		:= \
	-lpthread \
	-L$(BASEDIR)/threadmgr -lthreadmgr \

USERDEFS	:= \
	-D_LOG_SIMPLE \


#
#   Target object
#
TARGET_NAME	:=	thread_c

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	STATIC

#
#   Compile sources
#
SRCS		:= \
	thread_c.c \


#
#   Configurations
#
include $(BASEDIR)/Config.mak
