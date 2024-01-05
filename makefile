OUTPUTS = server client
CC = gcc

all: $(OUTPUTS)

clean:
        rm - f $(OUTPUTS)

.PHONY: all clean


server: sever.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_socket/passive_socket
        $(CC) -o server sever.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_sockets/passive_socket.h

client: client.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_socket/passive_socket.h
        $(CC) -o client client.c player.c grid.c pos_sockets/active_socket.c pos_sockets/char_buffer.c pos_sockets/passive_socket.c player.h grid.h buffer.h pos_sockets/active_socket.h pos_sockets/char_buffer.h pos_sockets/linked_list.h pos_sockets/passive_socket.h