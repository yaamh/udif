#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <panel.h>
#include "diffalgo.h"
#include "filediff.h"
#include "dirdiff.h"
#include "filelist.h"
#include "view.h"
#include "file.h"
#include "key.h"

//刷新属性
void flush_attr(viewbase_s *view)
{
    int i,j;
    attr_t attr;
    short pair;
    win_s *win;

    for(i=0;i<view->winnums;i++)
    {
        win = &view->win[i];
        lineattr_s (*attr_arr)[win->h][win->w] = (void*)win->attr_arr; 
        for(j=0;j<view->linemax;j++)
        {
            attr = (*attr_arr)[j][0].attr;
            pair = (*attr_arr)[j][0].pair;
            if(view->lineindex == j)
                attr |= A_UNDERLINE;
            if(pair == 2) //行差异
            {
                int n;
                for(n=0;n<win->w;n++)
                    mvwchgat(win->window, j, n, 1, attr, (*attr_arr)[j][n].pair, NULL);
            }
            else
                mvwchgat(win->window, j, win->x, win->w, attr, pair, NULL);
        }
    }
}

//清屏
void clear_view(viewbase_s *view)
{
    wclear(view->win[0].window);
    wclear(view->win[1].window);
}

//刷新view
void update_view(view_s *view)
{
    if(view->status == DIRVIEW)
    {
        flush_attr(&view->dirview.base);
        touchwin(view->dirview.base.winpair);
        wnoutrefresh(view->dirview.base.winpair);
    }
    else if(view->status == FILEVIEW)
    {
        flush_attr(&view->fileview.base);
        touchwin(view->fileview.base.winpair);
        wnoutrefresh(view->fileview.base.winpair);
    }
}

int print_func(vfilenode_s* vfn,void* arg,int level)
{
    int i;
    vfile_s *vf;
    int *lines = ((void**)arg)[0];
    dirview_s *view =((void**)arg)[1];
    
    if(*lines < view->base.printstart)
    {
        (*lines)++;
        return 0;
    }
#define PRINTNUMY ({int t=view->base.viewmax - view->base.printstart;t>view->base.linemax?view->base.linemax:t;})
    if(*lines > view->base.printstart + PRINTNUMY)
        return 1;

    for(i=0;i<view->base.winnums;i++)
    {
        vf = &vfn->vfs[i];
        if(vf->file)
        {
            if(vf->file->type == FT_DIR)
                mvwprintw(view->base.win[i].window,*lines-view->base.printstart,SPE*level,"%c%s",vfn->showchild?'-':'+',vf->file->name);
            else
                mvwprintw(view->base.win[i].window,*lines-view->base.printstart,SPE*level,"%s",vf->file->name);
        }
    }
    (*lines)++;
    return 0;
}

//打印文件列表
void print_dirlist(dirview_s *view)
{
    int lines = 0;
    void *args[] = {&lines,view};
    recurs_filelist(&view->file_list,print_func,(void*)args);
}

//打印文件
void print_linearr(fileview_s *view,vfilenode_s *vfn)
{
    int i,j,index=0;
    int linelen;
    vlinesnode_s *lines = &vfn->lines;
    viewbase_s *base = &view->base;
    lineattr_s (*attr_arr1)[base->win[0].w] = (void*)base->win[0].attr_arr; 
    lineattr_s (*attr_arr2)[base->win[1].w] = (void*)base->win[1].attr_arr; 
    for(i=0;i<vfn->childnum;i++)
    {
        if(i<base->printstart)
            continue;
        if(index>=base->linemax)
            break;
        switch(lines->line[i].difftype)
        {
            case 0:
            case 1:
                attr_arr1[index][0].pair = lines->line[i].difftype;
                attr_arr2[index][0].pair = lines->line[i].difftype;
                break;
            case 2:
                for(j=0;j<base->win[0].w;j++)
                    attr_arr1[index][j].pair = 0; //清空行颜色
                attr_arr1[index][0].pair = 2; //这一步主要是后续属性判断时可以区分行差异
                linelen = strlen(lines->line[i].left);
                linelen = linelen < base->win[0].w - 5 ? linelen : base->win[0].w - 5;
                for(j=0;j<linelen;j++)
                    attr_arr1[index][j+5].pair = lines->line[i].r1[j];

                for(j=0;j<base->win[1].w;j++)
                    attr_arr2[index][j].pair = 0; //清空行颜色
                attr_arr2[index][0].pair = 2; //这一步主要是后续属性判断时可以区分行差异
                linelen = strlen(lines->line[i].right);
                linelen = linelen < base->win[1].w - 5 ? linelen : base->win[1].w - 5;
                for(j=0;j<linelen;j++)
                    attr_arr2[index][j+5].pair = lines->line[i].r2[j];
                break;
        }
        if(lines->line[i].left)
            mvwprintw(base->win[0].window,index,0,"%-4d %s",i,lines->line[i].left);
        if(lines->line[i].right)
            mvwprintw(base->win[1].window,index,0,"%-4d %s",i,lines->line[i].right);
        index++;
    }
}


//初始化窗口
void init_win(WINDOW *father,win_s *win, int h, int w, int y, int x)
{
    win->fullwin = derwin(father,h,w,y,x);
    win->window = derwin(win->fullwin,
            h - 2, w - 2, 1, 1);
    getmaxyx(win->window, win->h, win->w);
    win->y = win->x = 0;
    win->attr_arr = malloc(sizeof(lineattr_s)*win->h*win->w);
    memset(win->attr_arr,0,sizeof(lineattr_s)*win->h*win->w);
    keypad(win->fullwin,TRUE);
    keypad(win->window,TRUE);
    box(win->fullwin,0,0);
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
void init_dirview(dirview_s *view, char *argv[])
{
    view->base.winpair = newwin(LINES,COLS,0,0);
    init_win(view->base.winpair,&view->base.win[0],LINES,COLS/2,0,0);
	init_win(view->base.winpair,&view->base.win[1],LINES,COLS/2,0,COLS/2);
    view->base.winnums = 2;

	//生成目录树
	set_winfilelist(&view->base.win[0],argv[1]);
	set_winfilelist(&view->base.win[1],argv[2]);
	
    view->base.printstart = 0;
    view->base.lineindex = 0;
    view->base.linemax = view->base.win[0].h;

	view->base.viewmax = diff_filelist(view->base.win[0].cwdnode, view->base.win[1].cwdnode, &view->file_list);

    //刷新窗口
    print_dirlist(view);
}

//初始化界面
void init_fileview(fileview_s *view)
{
    view->base.winpair = newwin(LINES,COLS,0,0);
    init_win(view->base.winpair,&view->base.win[0],LINES,COLS/2,0,0);
	init_win(view->base.winpair,&view->base.win[1],LINES,COLS/2,0,COLS/2);
    view->base.winnums = 2;

    view->base.linemax = view->base.win[0].h;
}

void init_view(view_s *view, char *argv[])
{
    init_dirview(&view->dirview, argv);
    init_fileview(&view->fileview);
    view->panel[0] = new_panel(view->fileview.base.winpair);
    view->panel[1] = new_panel(view->dirview.base.winpair);
    view->status = DIRVIEW;
    update_panels();
    update_view(view);
    doupdate();
}

//显示界面
void* show_view(void * arg)
{
    int ch;
    view_s view;

    setlocale(LC_ALL,"");
    initscr();
    if(has_colors() == FALSE)
        return NULL;
    start_color();
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(2,COLOR_YELLOW,COLOR_BLACK);
    noecho();
    cbreak();
    curs_set(FALSE);
    refresh();

    init_view(&view,arg);

    int run = TRUE;

    while(run)
    {
        ch = getch();
        if(view.status == DIRVIEW)
        {
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
        }
        else if(view.status == FILEVIEW)
        {
            switch(ch)
            {
                case 'q':
                    view.status = DIRVIEW;
                    show_panel(view.panel[DIRVIEW]);
                    break;
                case 'j':
                case 'k':
                    move_line(&view,ch);
                    break;
                case 'h':
                case 'l':
                    move_char(&view.fileview,ch);
                    break;
                case 'n':
                case 'N':
                    tab_diff(&view.fileview,ch);
                    break;
                case '\n':
                    break;
            }
        }
        update_view(&view);
        doupdate();
    }
    endwin();
}



