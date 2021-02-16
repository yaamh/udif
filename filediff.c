#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "diffalgo.h"
#include "dirdiff.h"
#include "file.h"


int line_eql(void*s1,void*s2)
{
    if(!strcoll(s1, s2))
        return 1;
    else
        return 0;
}

//比对文件差异
int filediff(vfilenode_s *vfn,vlinenode_s **vlnarr)
{
    filenode_s *fn;
    char *data[2];
    char **linearr[2]={0};
    int  lines[2];
    int i;

    for(i=0;i<2;i++)
    {
        fn = vfn->vfs[i].file;
        if(fn)
        {
            char *file = get_path(fn);
            int filelen = dump_file(file,(void**)&data[i]);
            lines[i] = split_file(data[i],filelen,(void***)&linearr[i]);
        }
        else
        {
            lines[i] = 0;
            linearr[i] = NULL;
        }
    }

    char *r1,*r2;
    vlinenode_s *vln;
    int linelen = ld_data((void**)(linearr[0]),(void**)(linearr[1]),&r1,&r2,line_eql);
    vln = malloc(linelen*sizeof(vlinenode_s));
    memset(vln,0,linelen*sizeof(vlinenode_s));
    int r1index,r2index;
    for(i=0,r1index=0,r2index=0;i<linelen;i++)
    {
        while(r1index < lines[0] && r1[r1index] == 1)
        {
            vln[i].left = linearr[0][r1index++];
            vln[i].right = NULL;
            i++;
        }
        while(r2index < lines[1] && r2[r2index] == 1)
        {
            vln[i].left = linearr[1][r2index++];
            vln[i].right = NULL;
            i++;
        }
        if(r1index < lines[0] && r2index < lines[1] && r1[r1index] == r2[r2index])
        {
            vln[i].left = linearr[0][r1index++];
            vln[i].right = linearr[1][r2index++];
        }
    }
    *vlnarr = vln;

    if(r1)
        free(r1);
    if(r2)
        free(r2);
    if(linearr[0])
        free(linearr[0]);
    if(linearr[1])
        free(linearr[1]);
    return  linelen;
}



