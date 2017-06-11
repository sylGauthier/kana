#ifndef LOAD_IMAGE_H
#define LOAD_IMAGE_H

#define FILE_LEN 512

struct Image {
    char fileName[FILE_LEN];
    unsigned int width, height;
    unsigned int nbComp;
    unsigned char** buffer;
};

struct Image* load_jpeg(char* filename);

void free_image(struct Image* image);

#endif
