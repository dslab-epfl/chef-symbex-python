/*
 * test.c
 *
 *  Created on: Jul 15, 2014
 *      Author: stefan
 */


#include <stdio.h>


void chef_fn_begin(const char *fn_name) {
    printf("Function start: %s\n", fn_name);
}


void chef_fn_end(const char *fn_name) {
    printf("Function end: %s\n", fn_name);
}

void chef_bb(int bb) {
    printf("BB: %d\n", bb);
}


int main(int argc, char **argv) {
    if (argc > 3) {
        printf("1\n");
    } else if (argc < 2) {
        printf("2\n");
    } else {
        printf("3\n");
    }
    return 0;
}
