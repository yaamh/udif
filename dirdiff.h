#ifndef __DIRDIFF_H__
#define __DIRDIFF_H__

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

int diff_filelist(dirnode_s *dir1, dirnode_s *dir2, list_node* out);
void recurs_filelist(list_node *list,int (*func)(vfilenode_s*,void*,int),void *arg);
vfilenode_s* seek_file(int seek,list_node *list);

#endif
