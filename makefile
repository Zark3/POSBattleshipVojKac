OUTPUTS = server client
CC = gcc

all: $(OUTPUTS)

clean:
	rm -f $(OUTPUTS)

.PHONY: all clean


server: server.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_sockets/passive_socket.h
	$(CC) -pthread -o server server.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_sockets/passive_socket.h -lncurses

client: client.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_sockets/passive_socket.h
	$(CC) -pthread -o client client.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_sockets/passive_socket.h -lncurses