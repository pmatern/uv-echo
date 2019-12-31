#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

uv_loop_t *loop;

void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
    buf->base = malloc(size);
    buf->len = size;
}

void free_handle(uv_handle_t *handle) {
    free(handle);
}

void free_write(uv_write_t *req, int status) {
    free(req);
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        }
        uv_close((uv_handle_t *) stream, free_handle);
    } else if (nread > 0) {
        uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
        int r = uv_write(req, stream, buf, 1, free_write);
        if (r) {
            fprintf(stderr, "Write error %s\n", uv_err_name(nread));
        }
    }

    free(buf->base);
}

void on_connection(uv_stream_t *server, int status) {
    if (status) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    if (!client) {
        fprintf(stderr, "Client creation error\n");
        return;
    }

    int r = uv_tcp_init(loop, client);
    if (r) {
        fprintf(stderr, "Client initialization error %s\n", uv_strerror(r));
        free(client);
        return;
    }

    r = uv_accept(server, (uv_stream_t *) client);
    if (r) {
        fprintf(stderr, "Socket accept error %s\n", uv_strerror(r));
        uv_close((uv_handle_t *) client, free_handle);
        return;
    }

    r = uv_read_start((uv_stream_t *) client, alloc_buffer, on_read);
    if (r) {
        fprintf(stderr, "Socket read error %s\n", uv_strerror(r));
        uv_close((uv_handle_t *) client, free_handle);
    }
}

int main(int argc, char **argv) {
    loop = uv_default_loop();
    if (!loop) {
        fprintf(stderr, "UV Loop access error\n");
        return -1;
    }

    struct sockaddr_in addr;
    int r = uv_ip4_addr("0.0.0.0", 3000, &addr);
    if (r) {
        fprintf(stderr, "Address initialization error %s\n", uv_strerror(r));
        return -1;
    }

    uv_tcp_t server;
    r = uv_tcp_init(loop, &server);
    if (r) {
        fprintf(stderr, "Server initialization error %s\n", uv_strerror(r));
        return -1;
    }

    r = uv_tcp_bind(&server, (const struct sockaddr *) &addr, 0);
    if (r) {
        fprintf(stderr, "TCP bind error %s\n", uv_strerror(r));
        return -1;
    }

    r = uv_listen((uv_stream_t *) &server, 128, on_connection);
    if (r) {
        fprintf(stderr, "TCP listen error %s\n", uv_strerror(r));
        return -1;
    }

    if (uv_run(loop, UV_RUN_DEFAULT)) {
        abort();
    }

    uv_loop_close(loop);
    free(loop);
    return 0;
}
