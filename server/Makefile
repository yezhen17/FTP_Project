all: server

clean:
	rm server

server: server.c cmds.c rw.c utils.c fd_manager.c rw.h utils.h fd_manager.h cmds.h
	gcc -o server -Wall server.c cmds.c rw.c utils.c fd_manager.c

