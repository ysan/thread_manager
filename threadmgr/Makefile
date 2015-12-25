#
#   Defines
#
#CC			=	/bin/gcc
CC			=	/usr/bin/gcc
CFLAGS		= \
	-Wall \
	-shared \
	-MD \
	-fPIC \
	-g \
#	-D_THM_LOG_OFF \

INCLUDES	=	-I./
LIBS		=
LDFLAGS		=	-lpthread
OBJDIR		=	./objs/
MKDIR		=	/bin/mkdir
RM			=	/bin/rm

LINKBASEDIR	=	$(shell pwd)/
LINKDIR		=	/usr/lib/

#
#   Objects
#
TARGET		=	libthreadmgr.so

SRCS		=\
	threadmgr.c \
	threadmgr_if.c\
	threadmgr_util.c\

OBJS		=	$(SRCS:%.c=$(OBJDIR)/%.o)
DEPENDS		=	$(OBJS:%.o=%.d)


#
#   Conditions
#
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES) $(LIBS) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(MKDIR) -p -m 775 $(OBJDIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDES)


.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	$(RM) -f $(TARGET) $(OBJS) $(DEPENDS)

-include $(DEPENDS)
