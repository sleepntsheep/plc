CFLAGS := -std=c99 -g
LIBS :=
SRCS := *.c

all: plc

plc: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install: plc
	sudo cp $< /usr/local/bin
