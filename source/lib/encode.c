#include "b64/cencode.h"
#include "b64/cdecode.h"
#include "encode.h"
#include <string.h>


// Encode the data blocks in the input of size sz into base64 format and
// stored in the outputBuffer out
int base64Encode(const char *in, unsigned int sz, char *out) {

  base64_encodestate state;
  base64_init_encodestate(&state);
  
  int count = base64_encode_block(in,sz,out,&state);
  count += base64_encode_blockend(out+count, &state);
  return count;

}

// Decode the base64 encoded data block into the corresponding data and is
// stored in the outputBuffer out
int base64Decode(const char *encodedIn, char *out){

  base64_decodestate state;
  base64_init_decodestate(&state);

  int count = base64_decode_block(encodedIn, strlen(encodedIn), out, &state);
  // adding zero at the end of the buffer
  out[count] = 0;
  return count;

}
