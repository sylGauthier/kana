#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "load_image.h"
#include "signature.h"
#include "base.h"

void print_help(char* cmd) {
    printf("Usage: %s <command> [options]\n", cmd);
    printf("Commands:\n"
           "    sig <image> <depth> : computes signature of <image> at depth <depth>\n"
           "    dist <image1> <image2> <depth> : computes distance between two images\n"
           "    dist <signature1> <signature2> : computes distance between two signatures (shortest depth is taken into account)\n"
           "    add <base> <image> [<image2> <image3> ...] : add an image to a base, detect doubles or close matches\n"
           "    help : prints this help\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help(argv[0]);
        return -1;
    }

    if (!strcmp(argv[1], "sig") && argc == 4) {
        struct Image* im;
        int depth = strtol(argv[3], NULL, 10);
        unsigned char* sig;
        int sigLen;

        if ((im = load_jpeg(argv[2])) && (sig = malloc(signature_length(depth) * sizeof(unsigned char)))) {
            int i;
            sigLen = compute_signature(im, depth, sig);
            printf("Length: %d bytes\n", sigLen);
            for (i = 0; i < sigLen; i++)
                printf("%02x", sig[i]);
            printf("\n");
            free_image(im);
            free(sig);
        } else {
            fprintf(stderr, "Invalid image, aborting\n");
            return -1;
        }
    } else if (!strcmp(argv[1], "dist") && argc == 5) {
        struct Image* im1;
        struct Image* im2;
        int depth = strtol(argv[4], NULL, 10);
        int sigLen;
        unsigned char* sig1;
        unsigned char* sig2;
        float dist;

        if ((im1 = load_jpeg(argv[2])) && (im2 = load_jpeg(argv[3]))
                && (sig1 = malloc(signature_length(depth) * sizeof(unsigned char)))
                && (sig2 = malloc(signature_length(depth) * sizeof(unsigned char)))) {
            sigLen = compute_signature(im1, depth, sig1);
            compute_signature(im2, depth, sig2);
            dist = distance(sig1, sig2, sigLen, depth);

            printf("%f\n", dist);
        } else {
            fprintf(stderr, "Invalid image, aborting\n");
            return -1;
        }
    } else if (!strcmp(argv[1], "dist") && argc == 4) {
        unsigned char* sig1;
        unsigned char* sig2;
        int l1, l2;
        float dist;

        if (hexsig_is_valid(argv[2], strlen(argv[2])) && hexsig_is_valid(argv[3], strlen(argv[3]))) {
            sig1 = hex_to_bytes(argv[2], &l1);
            sig2 = hex_to_bytes(argv[3], &l2);
            l1 = l1 <= l2 ? l1 : l2;

            dist = distance(sig1, sig2, l1, 16);
            printf("len; %d\n%f\n", l1, dist);

            free(sig1);
            free(sig2);
        }
        printf("Distance between signatures not implemented yet\n");
    } else if (!strcmp(argv[1], "add") && argc >= 4) {
        struct Base base;
        struct Image* curImage;
        int i;

        if (load_base(&base, argv[2]) != 0) {
            fprintf(stderr, "Loading base failed\n");
            return -1;
        }
        
        for (i = 3; i < argc; i++) {
            if (!(curImage = load_jpeg(argv[i]))) {
                close_base(&base);
                return -1;
            }

            if (add_image_to_base(&base, curImage) != 0) {
                fprintf(stderr, "Adding image failed\n");
            }

            free_image(curImage);
        }
        save_base(&base);
        close_base(&base);
    } else if (!strcmp(argv[1], "help")) {
        print_help(argv[0]);
    } else {
        print_help(argv[0]);
        return -1;
    }

    return 0;
}
