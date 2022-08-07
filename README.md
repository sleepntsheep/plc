### plc is a cli task manager

inspired by [please](https://github.com/NayamAmarshe/please)
which i find take a bit too long to open
so i made a more minimal version in C

### installation/ building
    sudo make install

or to staticly compile with musl
    CC='musl-gcc -static -Os' make

### notes
before using, create ~/.config/plc.txt or ~/plc.txt depending on your XDG variables, 
i am too lazy to implement `creating the file if not exists` feature

### usage

show all tasks
    plc

add task
    plc add <taskname>

delete task
    plc del <task index>

mask task as done/ undone
    plc do <task index>

clean all done task
    plc clean


