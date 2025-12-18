#include <stdlib.h>
#include <stdio.h>

#include "./helpers.h"

void handle_error(char *message) {
    perror(message);
    exit(1);
}

