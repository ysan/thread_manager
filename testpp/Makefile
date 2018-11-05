#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)/threadmgr \
	-I$(BASEDIR)/threadmgrpp \

LIBS		:= \
	-lpthread \
	-L$(BASEDIR)/threadmgr -lthreadmgr \
	-L$(BASEDIR)/threadmgrpp -lthreadmgrpp \

USERDEFS	:= \
	-D_LOG_SIMPLE \


#
#   Target object
#
TARGET_NAME	:=	testpp

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	EXEC

#
#   Compile sources
#
SRCS_CPP	:= \
	main.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak
