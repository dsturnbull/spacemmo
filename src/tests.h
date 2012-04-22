#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdarg.h>

#define TEST_DIR "tests"

#define assert_equals(a, b) do {                                            \
    if (a != b) {                                                           \
        printf("%i != %i (%s:%i)\n", a, b, __FILE__, __LINE__);             \
        return false;                                                       \
    }                                                                       \
} while (0)

typedef bool (*test_t)(void);
typedef void (*setup_t)(void);
typedef void (*teardown_t)(void);
void load_tests(char *);

