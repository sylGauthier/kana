#ifndef LOAD_IMAGE_H
#define LOAD_IMAGE_H

struct Image {
    unsigned int width, height;
    unsigned int nbComp;
    unsigned char** buffer;
};

struct Image* load_jpeg(char* filename);

void free_image(struct Image* image);

#endif
