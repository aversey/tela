DEBUG  ?= yes

CFLAGS  = -xc -ansi -Wall
ifeq '$(DEBUG)' 'yes'
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

tl_example: tl_example.c tl.c
	gcc $(CFLAGS) $^ -I. -o $@

.PHONY: clean
clean:
	rm -f tl_example
