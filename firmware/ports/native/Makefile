# Native (PC) simulator build of the BLE Environmental Sensor Node firmware core.
CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -Werror -std=c11 -O2 -D_POSIX_C_SOURCE=199309L
INC      = -I../../core/include
SRC      = ../../core/src/cli.c main.c
OUT      = esn-sim

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(INC) $(SRC) -o $(OUT) -lm

clean:
	rm -f $(OUT)

.PHONY: all clean
