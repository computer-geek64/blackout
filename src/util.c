// util.c
// Ashish D'Souza

#include "util.h"


int replaceString(char **out, const char *string, const char *substring, const char *replacement) {
    // Replace all occurrences of substring in string with replacement
    size_t originalLength = strlen(string);
    size_t len = strlen(string);
    size_t n = strlen(substring);
    size_t delta = strlen(replacement) - n;

    *out = (char*) malloc((len + 1) * sizeof(char));
    if(out == NULL) {
        handleMemoryAllocationError();

        // Cleanup
        return 1;
    }

    for(unsigned int i = 0, j = 0; i <= originalLength; i++, j++) {
        if(i <= originalLength - n && strncmp(string + i, substring, n) == 0) {
            len += delta;
            char* outRealloc = (char*) realloc(*out, (len + 1) * sizeof(char));
            if(outRealloc == NULL) {
                handleMemoryAllocationError();

                // Cleanup
                free(*out);
                return 1;
            }
            *out = outRealloc;
            strcpy(*out + j, replacement);
            i += n - 1;
            j += strlen(replacement) - 1;
        }
        else {
            *(*out + j) = *(string + i);
        }
    }
    return 0;
}
