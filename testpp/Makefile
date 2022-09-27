#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)/threadmgr \
	-I$(BASEDIR)/threadmgrpp \
	-I./module_a \
	-I./module_b \
	-I./module_c \

LIBS		:= \
	-lpthread \
	-L$(BASEDIR)/threadmgr -lthreadmgr \
	-L$(BASEDIR)/threadmgrpp -lthreadmgrpp \


USERDEFS	:= \
	-D_LOG_SIMPLE \

SUBDIRS		:= \


#
#   Target object
#
TARGET_NAME	:=	testpp
NOT_INSTALL :=	YES

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
