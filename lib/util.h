#ifndef UTIL
#define UTIL
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define NT(x) ((x)^1)
#ifdef DEBUG
#define LOG(x,...) fprintf(stderr,"%s @ L%d:\n\t " x, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
#define LOG(x,...) do {} while(0)
#endif
#endif
