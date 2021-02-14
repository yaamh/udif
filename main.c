#include <stdio.h>
#include <pthread.h>
#include "filelist.h"
#include "view.h"


int main(int argc,char *argv[])
{
    pthread_t pid;
    pthread_create(&pid,NULL,show_view,(void*)argv);


    pthread_join(pid,NULL);
}
