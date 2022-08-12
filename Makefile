CFLAGS := -std=c99
LIBS :=
SRCS := *.c

all: plc

plc: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install: plc
	chmod 755 plc
	sudo cp $< /usr/local/bin
