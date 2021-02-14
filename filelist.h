#ifndef __FILELIST_H__
#define __FILELIST_H__

#include <dirent.h>
#include "list.h"

struct _dirnodes_s;

typedef struct _filenode_s
{
    int type;
    list_node node;
    struct _dirnode_s *father;
    char name[256];
}filenode_s;

typedef struct _dirnode_s
{
    filenode_s file; //目录也是文件

    int showchild;
    int addchild;
    list_node dirchild;
    list_node filechild;
}dirnode_s;

enum
{
    FT_REG = DT_REG,
    FT_DIR = DT_DIR
};


dirnode_s* create_rootnode(const char* path);
void destroy_filelist(dirnode_s *file);
dirnode_s* get_filelist(dirnode_s *dir);
int foreach_file(dirnode_s *dirnode,int (*filter)(void*,void*,int),void*arg);
void* delete_filenode(filenode_s *node);
void insert_filenode(dirnode_s *dirnode,filenode_s *node,int bcpy);
int insert_checkfile(dirnode_s *dirnode,filenode_s *node);
char* get_path(filenode_s *file);

#endif
