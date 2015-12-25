#
#   Defines
#
BASEDIR		=	./

INCLUDES	= \
	-I./ \
	-I$(BASEDIR)/../threadmgr \
	-I$(BASEDIR)/main \
	-I$(BASEDIR)/thread_a \
	-I$(BASEDIR)/thread_b \
	-I$(BASEDIR)/thread_c \

LIBS		= \
	-L$(BASEDIR)/main -lmain \
	-L$(BASEDIR)/thread_a -lthread_a \
	-L$(BASEDIR)/thread_b -lthread_b \
	-L$(BASEDIR)/thread_c -lthread_c\
	-L$(BASEDIR)/../threadmgr -lthreadmgr \

LDFLAGS		= \
	-lpthread \
	-Wl,-rpath=$(BASEDIR)/../threadmgr \


#CC			=	/bin/gcc
CC			=	/usr/bin/gcc
CFLAGS		=	-Wall -MD -g
OBJDIR		=	./objs/
MKDIR		=	/bin/mkdir
RM			=	/bin/rm

SUBDIRS		= \
	thread_a \
	thread_b \
	thread_c \
	main\


#
#   Objects
#
TARGET		=	sample_thm


#
#   Conditions
#

.PHONY: all
all:
	@for d in $(SUBDIRS) ; do \
		(cd $$d && make) ;\
	done
	$(CC) -o $(TARGET) $^ $(CFLAGS) $(INCLUDES) $(LIBS) $(LDFLAGS)


.PHONY: clean
clean:
	@for d in $(SUBDIRS) ; do \
		(cd $$d && make clean) ;\
	done
	$(RM) -f $(TARGET)
