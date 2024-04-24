//
// Created by WENYi on 2024/4/12.
//
#include "kernel/types.h"
#include "named_pipe.h"
#include "kmalloc.h"
#include "process.h"
#include "page.h"

#define NAMED_PIPE_SIZE 4096  // 假设管道大小为1024字节

struct named_pipe {
    char *buffer;
    int read_pos;
    int write_pos;
    int flushed;
    int refcount;
    struct list queue;
    char *name;
};


struct named_pipe *named_pipe_create(const char *name)
{
    struct named_pipe *np = (struct named_pipe *)kmalloc(sizeof(*np));
    if (!np) return NULL;

    // 创建基本的 pipe 部分
    np->buffer = page_alloc(1);
    if (!np->buffer) {
        kfree(np);
        return NULL;
    }
    np->read_pos = 0;
    np->write_pos = 0;
    np->flushed = 0;
    np->queue.head = NULL;
    np->queue.tail = NULL;
    np->refcount = 1;

    // 复制名称
    np->name = strdup(name);
    if (!np->name) {
        page_free(np->buffer, 1);  // 假设 page_free 是释放 page_alloc 分配的内存的函数
        kfree(np);
        return NULL;
    }
    return np;
}

struct named_pipe *namedPipe_addref( struct named_pipe *np )
{
    np->refcount++;
    return np;
}

void namedPipe_flush(struct named_pipe *np)
{
    if(np) {
        np->flushed = 1;
    }
}
static int namedPipe_write_internal(struct named_pipe *np, char *buffer, int size, int blocking )
{
    if(!np || !buffer) {
        return -1;
    }
    int written = 0;
    if(blocking) {
        for(written = 0; written < size; written++) {
            while((np->write_pos + 1) % PIPE_SIZE == np->read_pos) {
                if(np->flushed) {
                    np->flushed = 0;
                    return written;
                }
                process_wait(&np->queue);
            }
            np->buffer[np->write_pos] = buffer[written];
            np->write_pos = (np->write_pos + 1) % PIPE_SIZE;
        }
        process_wakeup_all(&np->queue);
    } else {
        while(written < size && np->write_pos != (np->read_pos - 1) % PIPE_SIZE) {
            np->buffer[np->write_pos] = buffer[written];
            np->write_pos = (np->write_pos + 1) % PIPE_SIZE;
            written++;
        }
    }
    np->flushed = 0;
    return written;
}

int namedPipe_write(struct named_pipe *np, char *buffer, int size)
{
    return namedPipe_write_internal(np, buffer, size, 1);
}

int NamedPipe_write_nonblock(struct named_pipe *np, char *buffer, int size)
{
    return namedPipe_write_internal(np, buffer, size, 0);
}

static int namedPipe_read_internal(struct named_pipe *np, char *buffer, int size, int blocking)
{
    if(!np || !buffer) {
        return -1;
    }
    int read = 0;
    if(blocking) {
        for(read = 0; read < size; read++) {
            while(np->write_pos == np->read_pos) {
                if(np->flushed) {
                    np->flushed = 0;
                    return read;
                }
                if (blocking == 0) {
                    return -1;
                }
                process_wait(&np->queue);
            }
            buffer[read] = np->buffer[np->read_pos];
            np->read_pos = (np->read_pos + 1) % PIPE_SIZE;
        }
        process_wakeup_all(&np->queue);
    } else {
        while(read < size && np->read_pos != np->write_pos) {
            buffer[read] = np->buffer[np->read_pos];
            np->read_pos = (np->read_pos + 1) % PIPE_SIZE;
            read++;
        }
    }
    np->flushed = 0;
    return read;
}

int namedPipe_read(struct named_pipe *np, char *buffer, int size)
{
    return namedPipe_read_internal(np, buffer, size, 1);
}

int namedPipe_read_nonblock(struct named_pipe *np, char *buffer, int size)
{
    return namedPipe_read_internal(np, buffer, size, 0);
}

int namedPipe_size( struct named_pipe *np)
{
    return NAMED_PIPE_SIZE;
}

