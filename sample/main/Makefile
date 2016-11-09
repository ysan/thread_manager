#
#   Defines
#
BASEDIR		:=	../../

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


#
#   Target object
#
TARGET_NAME	:=	main

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	OBJECT

#
#   Compile sources
#
SRCS		:= \
	main.c \
	reg_tbl.c \


#
#   Configurations
#
include $(BASEDIR)/Config.make
