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
	-L./module_a -lmodule_a \
	-L./module_b -lmodule_b \
	-L./module_c -lmodule_c \


USERDEFS	:= \
	-D_LOG_SIMPLE \

SUBDIRS		:= \
	module_a \
	module_b \
	module_c \



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
	mgr_reg_tbl.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak
