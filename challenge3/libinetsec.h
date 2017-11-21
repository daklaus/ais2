#ifndef _LIBINETSEC_H_
#define _LIBINETSEC_H_

// User authentication
int auth_user(char *user, char *pass);

typedef unsigned char byte;
// We don't trust the standard canaries. we use our own
// Initialise the canary
void init_canary(byte *canary,char *seed1);
// Check the canary
int check_canary(byte *canary,char *seed1);
#endif 
