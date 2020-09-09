src = $(wildcard src/*.c)
obj = $(src:.c=.o)
llvm_include := $(shell llvm-config --includedir)
llvm_libs := $(shell llvm-config --libs)
cc: $(obj)
	$(CC) -g -I$(llvm_include)  $(llvm_libs)  -o $@ $^ $(LDFLAGS)

.PHONY: test
test: cc
	./run_tests
.PHONY: clean
clean:
	rm -f $(obj) cc
