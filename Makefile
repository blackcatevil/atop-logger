PROGRAM = atop_logd
LIB = liblogger.so
CC = $(CROSS_COMPILE)gcc
SRCS1 = atop_logd.c
OBJS1 = atop_logd.o
SRCS2 = logger.c
OBJS2 = logger.o

all: $(PROGRAM)
	@echo "make done!"

$(PROGRAM): $(OBJS1)
	$(CC) $(OBJS1) -o $(PROGRAM)

$(LIB): $(OBJS2)
	$(CC) $(OBJS2) -Wall -fPIC -shared -ldl -o $(LIB)

.c.o:
	$(CC) $< -c

clean:
	@rm -rf *.o $(PROGRAM) $(LIB)
	@echo "clean done!"

.PHONY:all clean
