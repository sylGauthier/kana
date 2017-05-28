#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "load_image.h"

unsigned int compute_signature(struct Image* image, int depth, unsigned char* sig);
float distance(unsigned char* sig1, unsigned char* sig2, unsigned int len);

#endif
