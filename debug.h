#ifndef _debug_h
#define _debug_h 1

void fatal(char *message);

void *ec_malloc(size_t size);

void *ec_calloc(size_t nmemb, size_t size);

int ec_open(const char *filename, int mode);

void ec_read(const char *filename, uint8_t *buffer, unsigned int bufferlength);

void ec_write(const char *filename, uint8_t *buffer, unsigned int bufferlength);

#endif
