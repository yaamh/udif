#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "filelist.h"
#include "diffalgo.h"
#include "dirdiff.h"
#include "filediff.h"
#include "view.h"

//移动光标
void move_line(view_s *view,int type)
{
#define PRINTNUM ({int t=base->viewmax - base->printstart;t>base->linemax?base->linemax:t;})
    viewbase_s *base;
    
    if(view->status == DIRVIEW)
        base = &view->dirview.base;
    else if(view->status == FILEVIEW)
        base = &view->fileview.base;

    if(type == 'j')
    {
        if(base->lineindex + 1 < PRINTNUM)
            //当光标未达到最下方文件，则光标下移
            base->lineindex++;
        else
        {
            if(base->printstart + PRINTNUM < base->viewmax)
            {
                //当光标达到最下方，但下方还有文件时，文件上移                
                base->printstart++;
                goto UPDATEVIEW;
            }
        }
    }
    else if(type == 'k')
    {
        if(base->lineindex > 0)
            //当光标未达最上方文件，光标上移
            base->lineindex--;
        else
        {
            if(base->printstart)
            {
                //若已达到最上方,但是上方还有文件,则文件下移
                base->printstart--;
                goto UPDATEVIEW;
            }
        }
    }
    return;

UPDATEVIEW:
    clear_view(base);
    if(view->status == DIRVIEW)
        print_dirlist(&view->dirview);
    else if(view->status == FILEVIEW)
        print_linearr(&view->fileview,view->fileview.vfn);
}

//张开目录
void toggle_dir(view_s *view)
{
    vfilenode_s *vfn;

    vfn = seek_file(view->dirview.base.lineindex + view->dirview.base.printstart,&view->dirview.file_list);
    dirnode_s* pdir[view->dirview.base.winnums];
    filenode_s *file;
    int i;
    int type;

    for(i=0;i<view->dirview.base.winnums;i++)
    {
        file = vfn->vfs[i].file;
        if(!file)
        {
            pdir[i] = NULL;
            continue;
        }
        if(file->type == FT_DIR)
        {
            //目录
            pdir[i] = (dirnode_s*)file;
            get_filelist(pdir[i]);
            type = FT_DIR;
        }
        else
            //文件
            type = FT_REG;
    }

    if(type == FT_DIR)
    {
        //目录
        vfn->showchild = !vfn->showchild;
        if(!vfn->addchild)
        {
            int filenum = diff_filelist(pdir[0], pdir[1], &vfn->childnode);
            vfn->addchild = 1;
            vfn->childnum = filenum;
        }
        //将链表插入vfilenode_s
        if(vfn->showchild)
            view->dirview.base.viewmax += vfn->childnum;
        else
            view->dirview.base.viewmax -= vfn->childnum;
        clear_view(&view->dirview.base);
        print_dirlist(&view->dirview);
    }
    else
    {
        //文件
        hide_panel(view->panel[DIRVIEW]);
        update_panels();
        if(!vfn->lines.line)
        {
            int filediff(vfilenode_s *vfn);
            vfn->childnum = filediff(vfn);
        }
        view->fileview.vfn = vfn;
        view->fileview.base.viewmax = vfn->childnum;
        view->fileview.base.lineindex = 0;
        view->fileview.base.printstart = 0;
        clear_view(&view->fileview.base);
        print_linearr(&view->fileview,vfn);
        view->status = FILEVIEW;
    }
}

void tab_diff(fileview_s *view,int type)
{
    int i;
    vfilenode_s *vfn = view->vfn;
    viewbase_s *base = &view->base;
    base->lineindex = 10;
    int lineindex = base->printstart + base->lineindex;
    int *diffpoint = vfn->lines.diffpoint;
    int diffnum = vfn->lines.diffnum;

    if(type == 'n')
    {
        for(i=0;i<diffnum;i++)
        {
            if(diffpoint[i] > lineindex)
            {
                //找到下一个差一点
                base->printstart = diffpoint[i] - base->lineindex;
                break;
            }
        }

    }
    else if(type == 'p')
    {
        for(i=diffnum-1;i>=0;i--)
        {
            if(diffpoint[i] < lineindex)
            {
                //找到上一个差一点
                if(diffpoint[i] > base->lineindex)
                    base->printstart = diffpoint[i] - base->lineindex;
                else
                {
                    base->lineindex = diffpoint[i];
                    base->printstart = 0;
                }
                break;
            }
        }
    }
    clear_view(base);
    print_linearr(view,vfn);
}

void move_char(fileview_s *view,int type)
{

}
















