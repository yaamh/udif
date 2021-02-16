#ifndef __VIEW_H__
#define __VIEW_H__

#include <curses.h>
#include <panel.h>
#include "dirdiff.h"

typedef struct
{
    WINDOW *fullwin;
    WINDOW *window;
    dirnode_s *cwdnode;
    int x,y,w,h;
    attr_t *attr_arr;
}win_s;

typedef struct
{
    int winnums;
    win_s  win[2];
    WINDOW *winpair;
    int printstart;
    int lineindex;
    int linemax;
    int viewmax;
}viewbase_s;

typedef struct
{
    viewbase_s base;
    list_node file_list;
}dirview_s;

typedef struct
{
    viewbase_s base;
    vfilenode_s *vfn;
}fileview_s;

enum
{
    FILEVIEW,
    DIRVIEW,
    VIEW_NUM
};

typedef struct
{
    dirview_s dirview;
    fileview_s fileview;
    PANEL* panel[VIEW_NUM];
    int status;
}view_s;

void* show_view(void* arg);
void clear_view(viewbase_s *view);
void print_dirlist(dirview_s *view);

#define SPE 2


#endif
