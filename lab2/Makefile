SRC = $(wildcard src/*.c)
HEADER = $(wildcard src/*.h)
INPUT ?= $(wildcard inputs/*/*.x)

OPT_FLAG = -O0

.PHONY: all verify clean

all: sim

sim: $(SRC) $(HEADER)
	gcc -Wall -Wextra -Wno-implicit-fallthrough -g $(OPT_FLAG) $^ -o $@

basesim: $(SRC)
	gcc -Wall -Wextra -g -O2 $^ -o $@

run: sim
	@python3 run.py $(INPUT)

clean:
	rm -rf *.o *~ sim
