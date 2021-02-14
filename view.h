#ifndef __VIEW_H__
#define __VIEW_H__

#include <curses.h>

typedef struct
{
    int level;
    filenode_s *file;
}vfile_s;

typedef struct
{
    WINDOW *fullwin;
    WINDOW *window;
    int x,y,w,h;
    int spe;
    attr_t *attr_arr;
    vfile_s *file_arr;
    int arr_maxnum;
    int file_maxnum;
    int printstarty,printnumy;
    filenode_s  *cutfile;
    int bcpyfile;
} win_s;


typedef struct
{
    int winnums;
    win_s  win[2];
}view_s;

void* show_view(void* arg);


#endif
