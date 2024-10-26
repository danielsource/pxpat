#ifndef UTIL_H_
#define UTIL_H_

#define UNUSED(param) (void)(param)

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
char *getenv_alloc(const char *name);

extern const char *arg0;

#endif /* UTIL_H_ */
