#ifndef BASE_H
#define BASE_H

#define DISTANCE_THRESHOLD 320.0
#define SIG_LEN 1023
#define SIG_DEPTH 4

#include <stdio.h>

#include "load_image.h"

struct Entry {
    unsigned char signature[SIG_LEN];
    char fileName[FILE_LEN];
    unsigned int sigLen;
    unsigned int fileNameLen;
    unsigned int width;
    unsigned int height;
};

struct Base {
    unsigned int size;
    unsigned int appendPos;
    struct Entry* data;
    FILE* file;
};

int load_base(struct Base* base, char* filename);
int save_base(struct Base* base);
void close_base(struct Base*);

int add_image_to_base(struct Base* base, struct Image* image);

#endif
