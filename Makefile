CC=gcc
MCC=smpcc --cc=$(CC)

OPT=3
CPPFLAGS=-DNDEBUG
CFLAGS=-Wall -O$(OPT) -std=gnu11 -fno-optimize-sibling-calls -funroll-all-loops -DDOUBLE_PRECISION $(OMPSSFLAGS)
OMPSSFLAGS=-Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses --keep-all-files --ompss
LDFLAGS=
LDLIBS=-lm

OUTPUT = synthetic
EXECUTABLES = $(OUTPUT) $(OUTPUT)_instr $(OUTPUT)_debug $(OUTPUT)_seq

all: perf instr debug seq
perf:  $(OUTPUT)
instr: $(OUTPUT)_instr
debug: $(OUTPUT)_debug
seq:   $(OUTPUT)_seq


$(OUTPUT):OMPSSFLAGS += --output-dir=.build_perf
$(OUTPUT)_instr:OMPSSFLAGS += --output-dir=.build_instr --instrument
$(OUTPUT)_debug:OMPSSFLAGS += --output-dir=.build_debug --debug
$(OUTPUT)_debug:OPT=0
$(OUTPUT)_debug:CFLAGS+=-g
$(OUTPUT)_seq:OMPSSFLAGS = -Wno-unknown-pragmas
$(OUTPUT)_seq:MCC=$(CC)

$(EXECUTABLES): $(wildcard *.c) | dirs
	$(MCC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^  -o $@ $(LDLIBS)


dirs:
	@mkdir -p .build_perf .build_instr .build_debug

clean:
	@rm -rvf $(EXECUTABLES)
	@rm -rf .build_perf .build_instr .build_debug *.o

.PHONY: all dirs clean instr perf debug seq
