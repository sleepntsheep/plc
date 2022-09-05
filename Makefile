CFLAGS := -std=c99 -pedantic -Wall -Wextra -g #-fsanitize=address
LIBS :=
SRCS := *.c

all: plc

plc: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm plc

install: plc
	chmod 755 plc
	sudo cp $< /usr/local/bin
