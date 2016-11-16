#
#   Defines
#
BASEDIR		:=	../../../

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)/src \
	-I$(BASEDIR)/sample/main \
	-I$(BASEDIR)/sample/modules/thread_a \
	-I$(BASEDIR)/sample/modules/thread_b \
	-I$(BASEDIR)/sample/modules/thread_c \

LIBS		:= \
	-lpthread \
	-L$(BASEDIR)/src -lthreadmgr \

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
include $(BASEDIR)/Config.make
