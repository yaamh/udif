#ifndef __VIEW_H__
#define __VIEW_H__

#include <curses.h>

typedef struct
{
    filenode_s *file;
    int level;
}vfile_s;

typedef struct
{
    list_node node;
    vfile_s vfs[2];
    list_node childnode;
    int showchild;
    int addchild;
    int childnum;
}vfilenode_s;

typedef struct
{
    WINDOW *fullwin;
    WINDOW *window;
    dirnode_s *cwdnode;
    int x,y,w,h;
    attr_t *attr_arr;
} win_s;


typedef struct
{
    int winnums;
    win_s  win[2];
    list_node file_list;
    int filenum;
    int printstart,printnum;
    int lineindex;
    int linemax;
    int spe;
}view_s;

void* show_view(void* arg);


#endif
