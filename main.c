#include <stdio.h>
#include <pthread.h>
#include "filelist.h"
#include "view.h"


int main()
{
    pthread_t pid;
    pthread_create(&pid,NULL,show_view,NULL);


    pthread_join(pid,NULL);
}
