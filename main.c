#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "include/nunsc.h"

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "nunsc:%s:%d: error: no input file\n", __FILE__, __LINE__);
        exit(1);
    }

    struct stat statbuf;
    int status = stat(argv[1], &statbuf);
    if (status < 0) {
        fprintf(stderr, "nunsc:%s:%d: error: %s\n", __FILE__, __LINE__, strerror(errno));
        exit(errno);
    }

    size_t len;
    const char *source;
    {
        FILE *fsrc = fopen(argv[1], "r");
        char *src = malloc(statbuf.st_size);
        len = fread(src, 1, statbuf.st_size, fsrc);
        if (len != (size_t) statbuf.st_size) {
            int err = errno;
            fclose(fsrc);
            fprintf(stderr, "nunsc:%s:%d: error: %s\n", __FILE__, __LINE__, strerror(err));
            exit(errno);
        }
        fclose(fsrc);
        source = src;
    }
    
    struct NunTokens tokens;

    if ((status = nun_lex(&tokens, source, len))) {
        exit(status);
    }

    NunGc gc;
    nun_new_gc(&gc, 1024 * 1024);

    NunValue *program = NULL;
    size_t proglen;
    if ((status = nun_parse(&program, &proglen, &gc, &tokens))) {
        free(program);
        nun_del_tokens(&tokens);
        free((void *) source);
        exit(status);
    }
    nun_del_tokens(&tokens);

    for (size_t i = 0; i < proglen; i++) {
        const int cap = 1024;
        char buffer[cap];
        nun_display(buffer, cap, program + i);
        fprintf(stdout, "%.*s\n", cap, buffer);
    }

    NunHlbuilder b;
    nun_new_hlbuilder(&b, &gc);
    nun_hlb_make_prelude(&b);

    NunHlprog prog;
    if ((status = nun_to_hlbc(&prog, &b, program, proglen))) {
        nun_del_gc(&gc);
        free(program);
        free((void *) source);
        exit(1);
    }

    const int cap = 4096;
    char buffer[cap];
    nun_hlbc_display(buffer, cap, &prog);
    fprintf(stdout, "%.*s", cap, buffer);
    
    free(program);

    nun_del_gc(&gc);
    free((void *) source);

    return 0;
}
