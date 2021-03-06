#include "shmipc.h"
#include "mock_k.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
Easy to use ocaml bindings for the 'open and tailing' use case.

OCAMLshmipc_open_and_poll takes a directory and callback function that will get called on each CQ message.
*/

// Store the passed in funptr so we can wrap it
void(*extern_cb_funptr)(uint64_t, const char*, uint64_t);

K kdb_to_ocaml_cb(K x, K y) {
    extern_cb_funptr(x->j, y, y->n);
	return ki(5);
}

int parse_data_raw(unsigned char* base, int lim, uint64_t index, void* userdata) {
    char* c = malloc((lim+1)*sizeof(char));
    c[lim] = '\0';
    memcpy(c, base, lim);
    extern_cb_funptr(index, c, lim);
    free(c);
    return 0;
}
const parsedata_f* parser = &parse_data_raw;

int append_data_raw(unsigned char* base, int lim, int* sz, K msg) {
    return 0;
}
const appenddata_f* appender = &append_data_raw;

K append_check_raw(queue_t* queue, K msg) {
    return msg;
}
const encodecheck_f* encoder = &append_check_raw;



void OCAMLshmipc_open_and_poll(const char* dirpath, void(*cb_funptr)(uint64_t, const char*, uint64_t)) {
    K dir = kss(dirpath);
    uint64_t index = 0;
    per(shmipc_init(dir, parser, appender, encoder));

    extern_cb_funptr = cb_funptr;
    K cb = dl(&kdb_to_ocaml_cb, 2);
    K kindex = kj(index);
    per(shmipc_tailer(dir, cb, kindex));
    per(shmipc_peek(dir));
    while (1) {
		usleep(500*1000);
		per(shmipc_peek(dir));
	}

	per(shmipc_close(dir));

	r0(dir);
	r0(cb);
	r0(index);
}
