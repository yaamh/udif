#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "filelist.h"
#include "diffalgo.h"
#include "dirdiff.h"
#include "file.h"

//比较2个文件是否一样
int file_eql(vfile_s *file1,vfile_s *file2)
{
    filenode_s *f1 = file1->file;
    filenode_s *f2 = file2->file;

	if(f1->type != f2->type)
		return 0;
	if(!strncmp(f1->name,f2->name,sizeof(f1->name)))
		return 1;
	return 0;
}

//将文件链表转换为数组
int filearr_func(void *node,void *args,int level)
{
    int *index = ((void**)args)[0];
    vfile_s *arr = ((void**)args)[1];

    arr[*index].file = node;
    arr[*index].level = level;
    (*index)++;
    return 0;
}

//将文件链表转换为数组
vfile_s* get_filearr(dirnode_s *dir)
{
    if(!dir)
        return NULL;
    int index = 0;
    vfile_s *arr = malloc(dir->childnum*sizeof(vfile_s));
    memset(arr,0,dir->childnum*sizeof(vfile_s));
    
    void *args[] = {&index,arr};
	foreach_file(dir, filearr_func, (void*)args, 0);
    return arr;
}

//对比文件列表
int diff_filelist(dirnode_s *dir1, dirnode_s *dir2, list_node* out)
{
    int vf1num = 0;
    int vf2num = 0;
	int i,j;
	int index;
	char *r1,*r2;

    vfile_s* vf1 = get_filearr(dir1);
    vfile_s* vf2 = get_filearr(dir2);
    if(vf1)
        vf1num = dir1->childnum;
    if(vf2)
        vf2num = dir2->childnum;
	
	vfile_s *f1arr[vf1num+1];
	vfile_s *f2arr[vf2num+1];
	
	for(i=0;i<vf1num;i++)
		f1arr[i] = &vf1[i];
	f1arr[i] = NULL;
	for(i=0;i<vf2num;i++)
		f2arr[i] = &vf2[i];
	f2arr[i] = NULL;
	
	//获取文件比较结果，相同行
	int comlines = lcs_data((void**)f1arr,(void**)f2arr,&r1,&r2,(void*)file_eql);
	int maxlines = vf1num + vf2num - comlines;

    //将相同的部分挪到数组前面
	for(i=0,index=0;i<vf1num;i++)
	{
		if(r1[i])
			f1arr[index++] = &vf1[i];
		else
			f1arr[comlines+i-index] = &vf1[i];
	}
	for(i=0,index=0;i<vf2num;i++)
	{
		if(r2[i])
			f2arr[index++] = &vf2[i];
		else
			f2arr[comlines+i-index] = &vf2[i];
	}
    
    //将结果转换为链表
    vfilenode_s *node;
    list_init(out);
	for(i=0;i<comlines;i++)
    {
        node = malloc(sizeof(vfilenode_s)); 
        memset(node,0,sizeof(vfilenode_s));
        node->vfs[0] = *f1arr[i];
        node->vfs[1] = *f2arr[i];
        list_add_prev(out, &node->node);
    }

	for(i=comlines;i<vf1num;i++)
    {
        node = malloc(sizeof(vfilenode_s)); 
        memset(node,0,sizeof(vfilenode_s));
        node->vfs[0] = *f1arr[i];
        node->vfs[1].file = NULL;
        list_add_prev(out, &node->node);
    }

	for(i=comlines;i<vf2num;i++)
    {
        node = malloc(sizeof(vfilenode_s)); 
        memset(node,0,sizeof(vfilenode_s));
        node->vfs[1] = *f2arr[i];
        node->vfs[0].file = NULL;
        list_add_prev(out, &node->node);
    }

	free(r1);
	free(r2);
    free(vf1);
    free(vf2);
    return maxlines;
}

//递归遍历链表
void recurs_filelist(list_node *list,int (*func)(vfilenode_s*,void*,int),void *arg)
{
    static int level = 0;
    vfilenode_s *vfn;
    list_node *pos;

    level++;
    list_foreach(pos,list) 
    {
        vfn = list_entry(pos, vfilenode_s, node);
        if(func(vfn,arg,level))
        {
            level--;
            return;
        }
        if(vfn->showchild)
        {
            recurs_filelist(&vfn->childnode,func,arg);
        }
    }
    level--;
}

//定位文件
int seek_func(vfilenode_s *vfn,void *arg,int level)
{
    vfilenode_s **retvfn = ((void**)arg)[0];
    int *index = ((void**)arg)[1];
    int *seek = ((void**)arg)[2];

    if((*index)++ == *seek)
    {
        *retvfn = vfn;
        return 1;
    }
    return 0;
}

vfilenode_s* seek_file(int seek,list_node *list)
{
    vfilenode_s *vfn;
    int index = 0;
    void *args[] = {&vfn,&index,&seek};
    recurs_filelist(list,seek_func,(void*)args);
    return vfn;
}
























