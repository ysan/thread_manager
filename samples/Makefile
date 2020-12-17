TARGET		=	sample
SRCS		=	sample.cpp

$(TARGET): $(SRCS)
	g++ -o $@ $^ -lthreadmgr -lthreadmgrpp -lpthread -D_LOG_SIMPLE

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET)
