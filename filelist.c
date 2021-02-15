#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include "filelist.h"

//复制文件节点
filenode_s* copy_filenode(filenode_s *file)
{
    filenode_s* tfile;
    dirnode_s *dir;
    
    if(file->type == DT_DIR)
    {
        dir = malloc(sizeof(dirnode_s));
        memset(dir,0,sizeof(dirnode_s));
        list_init(&dir->file.node);
        list_init(&dir->dirchild);
        list_init(&dir->filechild);
        strncpy(dir->file.name,file->name,sizeof(dir->file.name));
        dir->file.type = FT_DIR;
        return &dir->file;
    }
    else
    {
        tfile = malloc(sizeof(filenode_s));
        memset(tfile,0,sizeof(filenode_s));
        list_init(&tfile->node);
        strncpy(tfile->name,file->name,sizeof(tfile->name));
        tfile->type = FT_REG;
        return tfile;
    }
}

//创建文件节点
filenode_s* create_filenode(struct dirent *dit)
{
    filenode_s* file;
    dirnode_s *dir;
    
    if(dit->d_type == DT_DIR)
    {
        dir = malloc(sizeof(dirnode_s));
        memset(dir,0,sizeof(dirnode_s));
        list_init(&dir->file.node);
        list_init(&dir->dirchild);
        list_init(&dir->filechild);
        strncpy(dir->file.name,dit->d_name,sizeof(dir->file.name));
        dir->file.type = FT_DIR;
        return &dir->file;
    }
    else
    {
        file = malloc(sizeof(filenode_s));
        memset(file,0,sizeof(filenode_s));
        list_init(&file->node);
        strncpy(file->name,dit->d_name,sizeof(file->name));
        file->type = FT_REG;
        return file;
    }
}


//创建根目录节点
dirnode_s* create_rootnode(const char* path)
{
    dirnode_s *rootdir;

    rootdir = malloc(sizeof(dirnode_s));
    memset(rootdir,0,sizeof(dirnode_s));
    list_init(&rootdir->file.node);
    list_init(&rootdir->dirchild);
    list_init(&rootdir->filechild);
    strncpy(rootdir->file.name,path,sizeof(rootdir->file.name));
    rootdir->file.type = FT_DIR;

    return rootdir;
}

//递归释放目录下所有文件
void destroy_filelist(dirnode_s *dir)
{
    list_node *pos,*n;
    filenode_s  *pfile;

    //释放子文件
    list_foreach_safe(pos,n,&dir->filechild)
    {
        pfile = list_entry(pos,filenode_s,node);
        list_del(&pfile->node);
        free(pfile);
    }
    //目录文件，递归释放
    list_foreach_safe(pos,n,&dir->dirchild)
        destroy_filelist((dirnode_s*)list_entry(pos,filenode_s,node));
    list_del(&dir->file.node);
    free(dir);

}

//获取路径
char* get_path(filenode_s *file)
{
    static char path[1024];
    static char* cpath[16];
    memset(path,0,sizeof(path));
    memset(cpath,0,sizeof(cpath));

    int i = 0;
    filenode_s *pfile = file;
    while(pfile)
    {
        cpath[i++] = pfile->name;
        pfile = (filenode_s*)pfile->father;
    }

    while(--i)
    {
        strncat(path,cpath[i],sizeof(path));
        strncat(path,"/",sizeof(path));
    }

    strncat(path,cpath[i],sizeof(path));
    if(file->type == FT_DIR)
        strncat(path,"/",sizeof(path));

    return path;
}

//获取目录下所有文件
dirnode_s* get_filelist(dirnode_s *dir)
{
    filenode_s *pfilenode;
    dirnode_s *pdirnode;
    struct  dirent **namelist;
    char *path;
    int n,i;

    if(dir->addchild)
        return dir;

    path = get_path((filenode_s*)dir);
    n = scandir(path, &namelist, NULL, alphasort);
    if(n<0)
        return NULL;
    /* 遍历子文件*/
    for(i=0;i<n;i++)
    {
        if(strcmp(namelist[i]->d_name,".") && strcmp(namelist[i]->d_name,".."))
        {
            if(namelist[i]->d_type == DT_DIR)
            {
                pdirnode = (dirnode_s*)create_filenode(namelist[i]);
                list_add_next(&dir->dirchild, &pdirnode->file.node);
                pdirnode->file.father = dir;
            }
            else
            {
                pfilenode = create_filenode(namelist[i]);
                list_add_next(&dir->filechild, &pfilenode->node);
                pfilenode->father = dir;
            }
            dir->childnum++;
        }
        free(namelist[i]);
    }
    free(namelist);
    dir->addchild = 1;

    return dir;
}

//遍历目录节点
int foreach_file(dirnode_s* dirnode,int (*filter)(void*,void*,int),void*arg,int recurs)
{
    static int level = 0;
    list_node *pos;
    filenode_s *file;
    dirnode_s  *dir;

    level++;
    list_foreach_resv(pos, &dirnode->dirchild)
    {
        dir = (dirnode_s*)list_entry(pos,filenode_s,node);
        if(filter)
            if(filter(dir,arg,level))
                goto OUT;
        if(recurs)
            if(foreach_file(dir,filter,arg,recurs))
                goto OUT;
    }

    list_foreach_resv(pos, &dirnode->filechild)
    {
        file = list_entry(pos, filenode_s,node);
        if(filter)
            if(filter(file,arg,level))
                goto OUT;
    }
    level--;
    return 0;
OUT:
    level--;
    return 1;
}

//删除文件节点
void* delete_filenode(filenode_s *file)
{
    list_del(&file->node);
    file->father = NULL;

    return file;
}

//检测文件拷贝是否合法
int insert_checkfile(dirnode_s *dirnode, filenode_s *file)
{
    filenode_s *pfile;
    list_node *pos;

    list_foreach_resv(pos, &dirnode->dirchild)
    {
        pfile = list_entry(pos,filenode_s,node);
        if(!strcoll(file->name,pfile->name))
            return -1;
    }
    list_foreach_resv(pos, &dirnode->filechild)
    {
        pfile = list_entry(pos,filenode_s,node);
            if(!strcoll(file->name,pfile->name))
                return -1;
    }
    return 0;
}

//目录下插入文件
void insert_filenode(dirnode_s *dirnode,filenode_s *file,int bcpy)
{
    filenode_s *pfile,*tfile;
    list_node *pos,*target;
    
    if(bcpy)
        tfile = copy_filenode(file);
    else
        tfile = file;

    if(tfile->type == DT_DIR)
        target = &dirnode->dirchild;
    else
        target = &dirnode->filechild;

    list_foreach_resv(pos,target)
    {
        pfile = list_entry(pos,filenode_s,node);
        if(strcoll(tfile->name,pfile->name)<0)
        {
            tfile->father = dirnode;
            list_add_next(&pfile->node,&tfile->node);
            return;
        }
    }
    tfile->father = dirnode;
    list_add_next(target,&tfile->node);
}











