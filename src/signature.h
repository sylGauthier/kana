#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "load_image.h"

unsigned char* hex_to_bytes(const char* s, int* len);
unsigned int signature_length(unsigned int depth);
int hexsig_is_valid(char* sig, int len);
unsigned int compute_signature(struct Image* image, int depth, unsigned char* sig);
float distance(unsigned char* sig1, unsigned char* sig2, unsigned int len, int depth);

#endif
