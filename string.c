#include "types.h"
#include "string.h"

void memcpy(void *src, void *dst, size_t size) {
    u64 num_qwords, idx;
    u8 remainder;
    num_qwords = size>>3;
    remainder = size - (num_qwords << 3);

    //copy all quad words
    for (idx = 0; idx < num_qwords; idx++) {
        *(u64*)dst++ = *(u64*)src++;
    }

    //fill in remaining bytes. Should be between 0-7
    for (idx = 0; idx < remainder; idx++) {
        *(u8*)dst++ = *(u8*)src++;
    }
}

int strlen(char *str, u32 size) {
    int idx = 0;
    while ((*str != '\0') && (idx < size)) {
        idx++;
    }
    return idx < size ? idx : -1;
}

char *split_get_next(char *buf, char *end) {
  
  //find the first position where *buf is NULL indicating a split
  while ((buf < (end-1)) && (*buf != '\0')) {
    buf++;
  }

  //find the next non-zero position in case there were multiple spaces in the oken
  while ((buf < (end-1)) && (*buf == '\0')) {
    buf++;
  }

  //if the next non-zero argument at or 1 away from the end, then we cannot parse anymore arguments. return NULL
  if (buf >= (end-1)) //if we are at the very end, there are no more args)
    return NULL;

  //return current position as it holds the non-zero character
  return buf;

}


char *split_str(char *buf, u32 size, char token) {
  char *cur = buf;
  char *end = buf + size;

  while ((*cur != '\0') & (cur < end)) {
    if (*cur == token) {
      *cur = '\0';
    }
    cur++;
  }
  return cur;
}