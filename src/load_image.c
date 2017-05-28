#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>

#include "load_image.h"

struct Image* load_jpeg(char* filename) {
    struct jpeg_decompress_struct cinfo;
    FILE* inPic;
    JSAMPARRAY buffer;
    int rowStride;
    int i;
    struct jpeg_error_mgr err;
    struct Image* res;

    printf("Reading image: %s\n", filename);

    if (!(inPic = fopen(filename, "r"))) {
        printf("Cannot open picture\n");
        return NULL;
    }

    if (!(res = malloc(sizeof(struct Image)))) {
        fprintf(stderr, "Error: could not allocate image\n");
        return NULL;
    }

    cinfo.err = jpeg_std_error(&err);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inPic);
    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);
    res->width = cinfo.output_width;
    res->height = cinfo.output_height;
    res->nbComp = cinfo.output_components;
    
    if (!(res->buffer = malloc(res->height*sizeof(unsigned char*)))) {
        fprintf(stderr, "Error allocating image\n");
        free(res);
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(inPic);
        return NULL;
    }

    for (i = 0; i < res->height; i++) {
        if (!(res->buffer[i] = malloc(res->nbComp*res->width*sizeof(unsigned char)))) {
            fprintf(stderr, "Error allocating image\n");
            free(res);
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
            fclose(inPic);
            return NULL;
        }
    }

    rowStride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, rowStride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);

        memcpy(res->buffer[cinfo.output_scanline - 1], buffer[0], res->nbComp*res->width);
    }

    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_decompress(&cinfo);
    fclose(inPic);

    return res;
}

void free_image(struct Image* image) {
    int i;

    for (i = 0; i < image->height; i++) {
        free(image->buffer[i]);
    }
    free(image->buffer);
    free(image);
}
