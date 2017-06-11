#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "signature.h"

static int powint(int a, int b) {
    if (b > 0) {
        if (b%2 == 0) {
            int tmp = powint(a, b/2);
            return tmp*tmp;
        } else {
            int tmp = powint(a, (b-1)/2);
            return a*tmp*tmp;
        }
    } else
        return (b == 0);
}

static void normalize(unsigned int* buffer, unsigned int size, unsigned int factor) {
    int i;

    for (i = 0; i < size; i++) {
        buffer[i] /= factor;
    }
}

static unsigned char gethex(const char *s, char **endptr) {
    while (isspace(*s)) s++;
    return strtoul(s, endptr, 16);
}

unsigned char* hex_to_bytes(const char *s, int *length) {
    unsigned char *answer = malloc((strlen(s) + 1) / 3);
    unsigned char *p;
    for (p = answer; *s; p++)
        *p = gethex(s, (char **)&s);
    *length = p - answer;
    return answer;
}

unsigned int signature_length(unsigned int depth) {
    return powint(4, depth + 1) - 1;
}

int hexsig_is_valid(char* sig, int len) {
    unsigned int i;

    for (i = 0; i < len; i++) {
        if (!((sig[i] >= '0' && sig[i] <= '9') || (sig[i] >= 'a' && sig[i] <= 'f')))
            return 0;
    }

    for (i = 0; i < 15; i++) {
        if (len == 2*signature_length(i))
            return 1;
    }

    return 0;
}

unsigned int compute_signature(struct Image* image, int depth, unsigned char* sig) {
    unsigned int i, j, k, siglen;
    unsigned int curDepth = depth;

    unsigned int nbSideCells = powint(2, curDepth);

    unsigned int** means;

    if (!(means = malloc((depth+1)*sizeof(unsigned int*)))) {
        fprintf(stderr, "Error allocating means table\n");
        return 0;
    }

    for (i = 0; i <= depth; i++) {
        if (!(means[i] = calloc(image->nbComp*powint(4,i), sizeof(unsigned int)))) {
            fprintf(stderr, "Error allocating means table\n");
            return 0;
        }
    }

    /*First we build the deepest level of means, directly from the image pixels*/
    for (j = 0; j < image->height; j++) {
        for (i = 0; i < image->width; i++) {
            for (k = 0; k < image->nbComp; k++) {
                means[curDepth][image->nbComp*(j*nbSideCells/image->height*nbSideCells + i*nbSideCells/image->width) + k]
                    += image->buffer[j][image->nbComp*i + k];
            }
        }
    }

    normalize(means[curDepth], image->nbComp*powint(4,curDepth), image->width*image->height/powint(4,curDepth));

    /*Then we recursively build up the lower levels of means up to the level 0, each time by averaging the level beneath*/
    for (i = curDepth - 1; i >= 0 && i < depth; i--) {
        for (j = 0; j < powint(4, i+1); j++) {
            for (k = 0; k < image->nbComp; k++) {
                means[i][image->nbComp*(j/powint(2,i+2)*powint(2,i) + (j%powint(2,i+1))/2) + k] += means[i+1][image->nbComp*j + k];
            }
        }
        normalize(means[i], image->nbComp*powint(4,i),4);
    }

    /*Finally we copy all this in the signature, while taking some overflow precaution (we're converting uint to uchar...)*/
    siglen = 0;
    for (i = 0; i <= depth; i++) {
        for (j = 0; j < powint(4,i); j++) {
            for (k = 0; k < image->nbComp; k++) {
                sig[siglen] = means[i][image->nbComp*j + k] <= 255 ? means[i][image->nbComp*j+k] : 255;
                siglen++;
            }
        }
    }

    for (i = 0; i <= depth; i++)
        free(means[i]);

    free(means);

    return siglen;
}

float distance(unsigned char* sig1, unsigned char* sig2, unsigned int len, int depth) {
    int i;
    float res = 0;
    int expo = 0;
    int cpt = 0;

    for (i = 0; i < len; i++) {
        res += ((float)sig1[i] - (float)sig2[i])*((float)sig1[i] - (float)sig2[i])
            / (float)powint(4, expo);
        cpt++;
        if (cpt >= powint(4, expo)) {
            expo++;
            cpt = 0;

            if (expo > depth)
                return res/depth;
        }
    }

    return res/expo;
}
