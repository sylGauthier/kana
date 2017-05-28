#include <stdio.h>

#include "load_image.h"
#include "signature.h"

void print_help(char* cmd) {
    printf("Usage: %s <picture_name>\n", cmd);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help(argv[0]);
        return -1;
    }

    if (argc == 2) {
        struct Image* image;
        unsigned char sig[1000];
        unsigned int l;
        int i;

        image = load_jpeg(argv[1]);

        printf("w: %d, j: %d, c: %d\n", image->width, image->height, image->nbComp);

        l = compute_signature(image, 2, sig);

        printf("Signature is %d bytes long\n", l);

        for (i = 0; i < l; i++) {
            printf("%02x", sig[i]);
        }
        printf("\n");

        free_image(image);
    } else if (argc == 3) {
        struct Image* im1;
        struct Image* im2;
        unsigned char sig1[1000];
        unsigned char sig2[1000];
        unsigned int len;

        im1 = load_jpeg(argv[1]);
        im2 = load_jpeg(argv[2]);

        len = compute_signature(im1, 3, sig1);
        compute_signature(im2, 3, sig2);

        printf("Distance: %f\n", distance(sig1, sig2, len));

        free_image(im1);
        free_image(im2);
    }

    return 0;
}
