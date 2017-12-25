LIB = liblogger.so
CC = $(CROSS_COMPILE)gcc
SRCS = logger.c
OBJS = logger.o

all: $(PROGRAM)
	@echo "make done!"

$(LIB): $(OBJS)
	$(CC) $(OBJS) -Wall -fPIC -shared -ldl -o $(LIB)

.c.o:
	$(CC) $< -c

clean:
	@rm -rf *.o $(PROGRAM) $(LIB)
	@echo "clean done!"

.PHONY:all clean
