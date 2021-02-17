#ifndef __FILEDIFF_H__
#define __FILEDIFF_H__

#include "filediff.h"

typedef struct
{
    char* left;
    char* right;
    int difftype;
    char* r1;
    char* r2;
}vlinenode_s;

typedef struct 
{
    vlinenode_s *line;
    int *diffpoint;//差异索引数组
    int diffnum; //差异个数
}vlinesnode_s;



#endif

