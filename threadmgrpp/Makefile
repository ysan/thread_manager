#
#   Defines
#
BASEDIR		:=	../

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)/threadmgr \

LIBS		:= \
	-lpthread \
	-rdynamic \
	-L$(BASEDIR)/threadmgr -lthreadmgr \

USERDEFS	:= \
	-D_LOG_SIMPLE \

#
#   Target object
#
TARGET_NAME	:=	threadmgrpp

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	SHARED

#
#   Compile sources
#
SRCS_CPP	:= \
	ThreadMgrIf.cpp \
	ThreadMgrExternalIf.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak
