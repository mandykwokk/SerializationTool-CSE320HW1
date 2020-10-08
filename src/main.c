#include <stdio.h>
#include <stdlib.h>


#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    int ret;
    if((ret=validargs(argc, argv))){
        USAGE(*argv, EXIT_FAILURE);
        fflush(stdout);
        return EXIT_FAILURE;
    }
    debug("Options: 0x%x", global_options);
    if(global_options & 1){ // h
        USAGE(*argv, EXIT_SUCCESS);
        fflush(stdout);
        return EXIT_SUCCESS;
    }
    else if((global_options&0x2)!=0){
        int s = serialize();
        //printf("serialize: %d\n",s);
        fflush(stdout);
        return s;
    }
    else if((global_options&0X4)!=0){
        int d = deserialize();
        //printf("deserialize: %d\n",d);
        fflush(stdout);
        return d;
    }
    else{
        fflush(stdout);
        return EXIT_FAILURE;
    }
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
