#
#   Defines
#
BASEDIR		=	../

INCLUDES	=\
	-I./ \
	-I$(BASEDIR)/../mycommon \
	-I$(BASEDIR)/../threadmgr \
	-I$(BASEDIR)/main \
	-I$(BASEDIR)/thread_a \
	-I$(BASEDIR)/thread_b \
	-I$(BASEDIR)/thread_c \

LIBDIR		=\
	-L$(BASEDIR)/../mycommon -lmycommon \
	-L$(BASEDIR)/../threadmgr -lthreadmgr \

LDFLAGS		=	-lpthread


#CC			=	/bin/gcc
CC			=	/usr/bin/gcc
AR			=	/usr/bin/ar
RANLIB		=	/usr/bin/ranlib
CFLAGS		=	-Wall -MD -g
OBJDIR		=	./objs/
MKDIR		=	/bin/mkdir
RM			=	/bin/rm


#
#   Objects
#
TARGET		=	libthread_a.a

SRCS		= \
	thread_a.c \

OBJS		=	$(SRCS:%.c=$(OBJDIR)/%.o)
DEPENDS		=	$(OBJS:%.o=%.d)


#
#   Conditions
#
$(TARGET): $(OBJS)
#	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES) $(LIBDIR) $(LDFLAGS)
	$(AR) rv $@ $^
	$(RANLIB) $@

$(OBJDIR)/%.o: %.c
	$(MKDIR) -p -m 775 $(OBJDIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDES)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	$(RM) -f $(TARGET) $(OBJS) $(DEPENDS)

-include $(DEPENDS)
