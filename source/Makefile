
CC=g++
CFLAGS=-std=c++11 -Wall -Wextra -Werror
LDFLAGS=-lpthread

SRCS=OrderGenerator.cpp TradeEngine.cpp
OBJS=$(SRCS:.cpp=.o)

# Main files
MAIN1=OrderGenerator.cpp
MAIN2=TradeEngine.cpp

# Executables
EXEC1=OrderGenerator
EXEC2=TradeEngine

.PHONY: all clean

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJS)
	$(CC) $(CFLAGS) $(MAIN1:.cpp=.o) -o $@ $(LDFLAGS)

$(EXEC2): $(OBJS)
	$(CC) $(CFLAGS) $(MAIN2:.cpp=.o) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC1) $(EXEC2)
