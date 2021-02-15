#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "filelist.h"
#include "diffalgo.h"
#include "dirdiff.h"
#include "view.h"
#include "file.h"

#define PRINTNUM ({int t=view->filenum - view->printstart;t>view->linemax?view->linemax:t;})
//刷新属性
void flush_attr(view_s *view)
{
    int i,j;
    attr_t attr;
    win_s *win;

    for(i=0;i<view->winnums;i++)
    {
        win = &view->win[i];
        for(j=0;j<view->linemax;j++)
        {
            attr=win->attr_arr[j];
            if(view->lineindex == j)
                attr |= A_UNDERLINE;
            mvwchgat(win->window, j, win->x, win->w, attr, 0, NULL);
        }
    }
}

//清屏
void clear_view(view_s *view)
{
    wclear(view->win[0].window);
    wclear(view->win[1].window);
}

//刷新view
void update_view(view_s *view)
{
    flush_attr(view);
    wrefresh(view->win[0].window);
    wrefresh(view->win[1].window);
}

int print_func(vfilenode_s* vfn,void* arg,int level)
{
    int i;
    vfile_s *vf;
    int *lines = ((void**)arg)[0];
    view_s *view =((void**)arg)[1];
    
    if(*lines < view->printstart)
    {
        (*lines)++;
        return 0;
    }
    if(*lines > view->printstart + PRINTNUM)
        return 1;

    for(i=0;i<view->winnums;i++)
    {
        vf = &vfn->vfs[i];
        if(vf->file)
        {
            if(vf->file->type == FT_DIR)
                mvwprintw(view->win[i].window,*lines-view->printstart,view->spe*level,"%c%s",vfn->showchild?'-':'+',vf->file->name);
            else
                mvwprintw(view->win[i].window,*lines-view->printstart,view->spe*level,"%s",vf->file->name);
        }
    }
    (*lines)++;
    return 0;
}

//打印文件列表
void print_dirlist(view_s *view)
{
    int lines = 0;
    void *args[] = {&lines,view};
    recurs_filelist(&view->file_list,print_func,(void*)args);
}


//初始化窗口
void init_win(win_s *win, int h, int w, int y, int x)
{
    win->fullwin = newwin(h,w,y,x);
    win->window = derwin(win->fullwin,
            h - 2, w - 2, 1, 1);
    getmaxyx(win->window, win->h, win->w);
    win->y = win->x = 0;
    win->attr_arr = malloc(sizeof(attr_t)*win->h);
    memset(win->attr_arr,0,sizeof(attr_t)*win->h);
    keypad(win->window,TRUE);
    box(win->fullwin,0,0);
    wrefresh(win->fullwin);
}

//设置窗口对于目录树
void set_winfilelist(win_s *win,const char *path)
{
	//创建当前目录节点
	win->cwdnode = create_rootnode(path);
	win->cwdnode = get_filelist(win->cwdnode);
	win->cwdnode->showchild = 1;
}

//初始化界面
void init_view(view_s *view, char *argv[])
{
	int i;
	
    init_win(&view->win[0],LINES,COLS/2,0,0);
	init_win(&view->win[1],LINES,COLS/2,0,COLS/2);
    view->winnums = 2;

	//生成目录树
	set_winfilelist(&view->win[0],argv[1]);
	set_winfilelist(&view->win[1],argv[2]);
	
    view->printstart = 0;
    view->printnum = 0;
    view->lineindex = 0;
    view->linemax = view->win[0].h;
    view->spe = 2;

	view->filenum = diff_filelist(view->win[0].cwdnode, view->win[1].cwdnode, &view->file_list);

    view->printnum = view->filenum - view->printstart;
    if(view->printnum > view->linemax)
        view->printnum = view->linemax;
	

    //刷新窗口
    print_dirlist(view);
    update_view(view);
}

//移动光标
void move_line(view_s *view,int type)
{
    if(type == 'j')
    {
        if(view->lineindex + 1 < PRINTNUM)
            //当光标未达到最下方文件，则光标下移
            view->lineindex++;
        else
        {
            if(view->printstart + PRINTNUM < view->filenum)
            {
                //当光标达到最下方，但下方还有文件时，文件上移                
                view->printstart++;
                clear_view(view);
                print_dirlist(view);
            }
        }
    }
    else if(type == 'k')
    {
        if(view->lineindex > 0)
            //当光标未达最上方文件，光标上移
            view->lineindex--;
        else
        {
            if(view->printstart)
            {
                //若已达到最上方,但是上方还有文件,则文件下移
                view->printstart--;
                clear_view(view);
                print_dirlist(view);
            }
        }
    }
}

//张开目录
void toggle_dir(view_s *view)
{
    vfilenode_s *vfn;

    vfn = seek_file(view->lineindex + view->printstart,&view->file_list);
    dirnode_s* pdir[view->winnums];
    filenode_s *file;
    int i;

    for(i=0;i<view->winnums;i++)
    {
        file = vfn->vfs[i].file;
        if(!file)
        {
            pdir[i] = NULL;
            continue;
        }
        if(file->type == DT_DIR)
        {
            //目录
            pdir[i] = (dirnode_s*)file;
            get_filelist(pdir[i]);
        }
        else
        {
            //文件
            return;
        }
    }
    vfn->showchild = !vfn->showchild;
    if(!vfn->addchild)
    {
        int filenum = diff_filelist(pdir[0], pdir[1], &vfn->childnode);
        vfn->addchild = 1;
        vfn->childnum = filenum;
    }
    //将链表插入vfilenode_s
    if(vfn->showchild)
        view->filenum += vfn->childnum;
    else
        view->filenum -= vfn->childnum;
    clear_view(view);
    print_dirlist(view);
}

//显示界面
void* show_view(void * arg)
{
    int ch;
    view_s view;

    initscr();
    start_color();
    noecho();
    cbreak();
    curs_set(FALSE);
    refresh();

    init_view(&view,arg);

    int run = TRUE;

    while(run)
    {
        ch = getch();
        switch(ch)
        {
            case 'q':
                run = FALSE;
                break;
            case 'j':
            case 'k':
                move_line(&view,ch);
                break;
            case '\n':
                toggle_dir(&view);
                break;
        }
        flush_attr(&view);
        update_view(&view);
    }
    endwin();
}
































