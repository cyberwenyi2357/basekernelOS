//
// Created by WENYi on 2024/4/12.
//

#ifndef BASEKERNELOS_NAMED_PIPE_H
#define BASEKERNELOS_NAMED_PIPE_H
struct named_pipe *namedPipe_create();
struct named_pipe *namedPipe_addref( struct named_pipe *p );
void namedPipe_delete(struct named_pipe *p);
void namedPipe_flush(struct named_pipe *p);

int namedPipe_write(struct named_pipe *p, char *buffer, int size);
int namedPipe_write_nonblock(struct named_pipe *p, char *buffer, int size);
int namedPipe_read(struct named_pipe *p, char *buffer, int size);
int namedPipe_read_nonblock(struct named_pipe *p, char *buffer, int size);
int namedPipe_size( struct named_pipe *p);
#endif //BASEKERNELOS_NAMED_PIPE_H
#include "kernel/types.h"

