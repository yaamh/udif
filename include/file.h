#ifndef __FILE_H__
#define __FILE_H__

#include "filelist.h"

int move_file(filenode_s *file,dirnode_s *target);
int delete_file(filenode_s *file);
int copy_file(filenode_s *file,dirnode_s *target);
int dump_file(const char *filepath,void **data);
int split_file(char *data,int len,void ***outarr);


#endif
