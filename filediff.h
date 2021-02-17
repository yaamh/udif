#ifndef __FILEDIFF_H__
#define __FILEDIFF_H__

#include "dirdiff.h"

typedef struct
{
    char* left;
    char* right;
    int difftype;
}vlinenode_s;

typedef struct
{
    vlinenode_s *line;
    int *diffpoint;//差异索引数组
    int diffnum; //差异个数
}vlinesnode_s;



#endif

