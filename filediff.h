#ifndef __FILEDIFF_H__
#define __FILEDIFF_H__

#include "dirdiff.h"

typedef struct
{
    char* left;
    char* right;
}vlinenode_s;

typedef struct
{
    vlinenode_s *line;
}vlinesnode_s;



#endif

