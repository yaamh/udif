#include <stdio.h>
#include <pthread.h>
#include "filelist.h"
#include "view.h"


int main(int argc,char *argv[])
{
#if 1
    pthread_t pid;
    pthread_create(&pid,NULL,show_view,(void*)argv);


    pthread_join(pid,NULL);
#else
    test_file(&argv[1]);
#endif
}
