#
#   Defines
#
BASEDIR		:=	../../../

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)/src \
	-I$(BASEDIR)/example/main \
	-I$(BASEDIR)/example/modules/thread_a \
	-I$(BASEDIR)/example/modules/thread_b \
	-I$(BASEDIR)/example/modules/thread_c \

LIBS		:= \
	-lpthread \
	-L$(BASEDIR)/src -lthreadmgr \

USERDEFS	:= \
	-D_LOG_SIMPLE \


#
#   Target object
#
TARGET_NAME	:=	thread_a

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	STATIC

#
#   Compile sources
#
SRCS		:= \
	thread_a.c \


#
#   Configurations
#
include $(BASEDIR)/Config.mak

