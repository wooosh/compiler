src = $(wildcard src/*.c)
obj = $(src:.c=.o)


cc: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) cc
