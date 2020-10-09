src = $(wildcard src/*.c)
obj = $(src:.c=.o)
headers = $(wildcard src/*.h)
llvm_include := $(shell llvm-config --includedir)
llvm_libs := $(shell llvm-config --libs)

cc: $(obj) 
	$(CC) $(CFLAGS) -g -I$(llvm_include)  $(llvm_libs) -lgc  -o $@ $^ $(LDFLAGS)

$(obj): $(headers)

.PHONY: test
test: cc
	./run_tests
.PHONY: clean
clean:
	rm -f $(obj) cc
