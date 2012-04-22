#include <dlfcn.h>
#include <stdarg.h>
#include <mach-o/dyld.h>
#include "src/tests.h"

int
main()
{
    load_tests(TEST_DIR);
    return 0;
}

void
load_tests(char *dir_name)
{
    DIR *dir;

    if ((dir = opendir(dir_name)) == NULL) {
        perror("opendir");
        exit(1);
    }

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        char *fn;
        asprintf(&fn, "%s/%s", dir_name, dp->d_name);

        if (strcmp(dp->d_name, ".") == 0)
            continue;

        if (strcmp(dp->d_name, "..") == 0)
            continue;

        struct stat st;
        if (stat(fn, &st) == -1) {
            free(fn);
            continue;
        }

        if (st.st_mode & S_IFDIR)
            load_tests(fn);

        char *match = ".dylib";
        char *ext = fn + strlen(fn) - strlen(match);
        if (strlen(fn) > strlen(match) && strcmp(ext, match) == 0) {
            void *dl;
            if ((dl = dlopen(fn, RTLD_LAZY)) == NULL) {
                printf("%s\n", dlerror());
                continue;
            }

            printf("%s\n", fn);

            // TODO iterate over symbols and run each one

            test_t *tests;
            if ((tests = (test_t *)dlsym(dl, "tests")) == NULL) {
                printf("\tcan't find test_suite\n");
                dlclose(dl);
                continue;
            }

            setup_t setup = (setup_t)dlsym(dl, "setup");
            teardown_t teardown = (teardown_t)dlsym(dl, "teardown");

            test_t *test;
            while (*(test = tests++) != NULL) {
                Dl_info info;
                dladdr((void *)*test, &info);
                printf("\t%s... ", info.dli_sname);

                if (setup)
                    setup();

                if (teardown)
                    teardown();

                if ((*test)())
                    printf("ok\n");
                else
                    printf("fail\n");
            }
            printf("\n");
            
            dlclose(dl);
        }

        free(fn);
    }
}

