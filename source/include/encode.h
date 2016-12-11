#ifndef _ENCODE_H
#define _ENCODE_H

int base64Encode(const char *in, unsigned int sz, char *out);
int base64Decode(const char *encodedIn, char *out);

#endif