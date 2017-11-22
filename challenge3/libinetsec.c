/*
 * libinetsec.c
 *
 *  Created on: Nov 21, 2017
 *      Author: klaus
 */

#include <unistd.h>
#include <string.h>

#include "libinetsec.h"

#define USER "inetsec024"
#define PASSWORD "zebughai"

int auth_user(char *user, char *pass) {
    if (strncmp(user, USER, sizeof(USER) - 1))
        return 0;
    if (strncmp(pass, PASSWORD, sizeof(PASSWORD) - 1))
        return 0;

    return getuid();
}

void init_canary(byte *canary, char *seed) {
    *canary = seed[0];
}

int check_canary(byte *canary, char *seed) {
    return *canary == *seed;
}

