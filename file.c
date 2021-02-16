#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "filelist.h"

//删除文件
int delete_file(filenode_s *file)
{
    const char *filename;
    
    filename = get_path(file);
    return remove(filename);
}

//复制剪切文件
int move_file(filenode_s *file,dirnode_s *target)
{
    char dirname[1024];
    const char* filename;

    strncpy(dirname,get_path(&target->file),sizeof(dirname));
    strncat(dirname,file->name,sizeof(dirname));

    filename = get_path(file);
    return rename(filename,dirname);
}

//复制文件
int copy_file(filenode_s *file,dirnode_s *target)
{
    char dirname[1024];
    char cmd[2048];
    const char *filename;
    
    strncpy(dirname,get_path(&target->file),sizeof(dirname));
    strncat(dirname,file->name,sizeof(dirname));
    filename = get_path(file);
    
    snprintf(cmd,sizeof(cmd),"cp %s %s",filename,dirname);
    
    return system(cmd);
}

//dump 文件!remember free data
int dump_file(const char *filepath,void **data)
{
    int filelen;
    char *filedata;
    int fd = open(filepath, O_RDONLY);
    if(fd<0)
        return 0;
    
    filelen = lseek(fd, 0, SEEK_END);
    if(!filelen)  //空文件直接返回
    {
        close(fd);
        return 0;
    }
    filedata = malloc(filelen + 1); //这里多1字节方便后续处理
    
    lseek(fd, 0, SEEK_SET);
    filelen = read(fd, filedata, filelen);
    filedata[filelen] = 0;
    
    *data = filedata;
    close(fd);
    return filelen;
}

//分割文件!remember free outarr
int split_file(char *data,int len,void ***outarr)
{
    int linenum;
    void **line;
    char *p;

    if(!len)
    {
        *outarr = NULL;
        return 0;
    }
    
    //这里考虑极限情况，每一行都是空行
    line = malloc(sizeof(void*)*(len+1));
    memset(line,0,sizeof(void*)*(len+1));
    
    //将文件'\n'分割
    linenum = 0;
    p = data;
    while(1)
    {
        line[linenum] = p;
        p = memchr(p,'\n',data + len - p);
        if(p)
        {
            *p = 0;
            p++;
        }
        else
        {
            //到达文件末尾，计算最后一行
            ((char*)data)[len] = 0;
            break;
        }
        linenum++;
    }
    
    *outarr = line;
    return linenum;
}



          



