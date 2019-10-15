all: server

clean:
	rm server

server: server.c cmds.c rw_utils.c dir_utils.c fd_manager.c rw_utils.h dir_utils.h fd_manager.h cmds.h server.h soc_utils.c soc_utils.h
	gcc -o server -Wall server.c cmds.c rw_utils.c dir_utils.c fd_manager.c soc_utils.c

