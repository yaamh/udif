#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "filelist.h"
#include "view.h"
#include "file.h"

//根据文件结构体定位屏幕中的行号
#if 0
int index_file(win_s *win,filenode_s *file)
{
    int i;
    for(i=0;i<win->printnum;i++)
    {
        if(file == win->file_arr[win->printstart + i].file)
            return i;
    }
    //如果在界面上找不到父文件，则向上查找
    i = 0;
    while(win->printstart + i >= 0)
    {
        if(file == win->file_arr[win->printstart + i].file)
            return i;
        i--;
    }
    return 0;
}
#endif


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

//update view
void update_view(view_s *view)
{
    flush_attr(view);
    wrefresh(view->win[0].window);
    wrefresh(view->win[1].window);
}

int filelist_eql(vfile_s *file1,vfile_s *file2)
{
    filenode_s *f1 = file1->file;
    filenode_s *f2 = file2->file;

	if(f1->type != f2->type)
		return FALSE;
	if(!strncmp(f1->name,f2->name,sizeof(f1->name)))
		return TRUE;
	return FALSE;
}

//对比文件列表
int diff_filelist(vfile_s *vf1,int vf1num, vfile_s *vf2,int vf2num, list_node* out)
{
	int i,j;
	int index;
	char *r1,*r2;
	
	vfile_s *f1arr[vf1num+1];
	vfile_s *f2arr[vf2num+1];
	
	for(i=0;i<vf1num;i++)
		f1arr[i] = &vf1[i];
	f1arr[i] = NULL;
	for(i=0;i<vf2num;i++)
		f2arr[i] = &vf2[i];
	f2arr[i] = NULL;
	
	//获取文件比较结果，相同行
	int comlines = lcs_data(f1arr,f2arr,&r1,&r2,(void*)filelist_eql);
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
        node->vfs[0] = *f1arr[i];
        node->vfs[1] = *f2arr[i];
        list_add_prev(out, &node->node);
    }

	for(i=comlines;i<vf1num;i++)
    {
        node = malloc(sizeof(vfilenode_s)); 
        node->vfs[0] = *f1arr[i];
        node->vfs[1].file = NULL;
        list_add_prev(out, &node->node);
    }

	for(i=comlines;i<vf2num;i++)
    {
        node = malloc(sizeof(vfilenode_s)); 
        node->vfs[1] = *f2arr[i];
        node->vfs[0].file = NULL;
        list_add_prev(out, &node->node);
    }

	free(r1);
	free(r2);
    return maxlines;
}

//打印文件
int arr_filelist(void *node,void *args,int level)
{
    int *index = ((void**)args)[0];
    vfile_s *arr = ((void**)args)[1];

    arr[*index].file = node;
    arr[*index].level = level;
    (*index)++;
    return 0;
}

vfile_s* get_filearr(dirnode_s *dir)
{
    int index = 0;
    vfile_s *arr = malloc(dir->childnum*sizeof(vfile_s));
    memset(arr,0,dir->childnum*sizeof(vfile_s));
    
    void *args[] = {&index,arr};
	foreach_file(dir, arr_filelist, (void*)args);
    return arr;
}

//打印文件列表
void print_dirlist(view_s *view)
{
    list_node *pos;
    vfilenode_s *vfn;
    vfile_s *vf;
    int lines = 0;
    int i;

    list_foreach(pos,&view->file_list) 
    {
        if(lines < view->printstart)
        {
            lines++;
            continue;
        }
        if(lines > view->printstart + view->printnum)
            break;

        vfn = list_entry(pos, vfilenode_s, node);
        for(i=0;i<view->winnums;i++)
        {
            vf = &vfn->vfs[i];
            if(vf->file)
            {
                if(vf->file->type == FT_DIR)
                    mvwprintw(view->win[i].window,lines-view->printstart,view->spe*vf->level,"%c%s",((dirnode_s*)vf->file)->showchild?'-':'+',vf->file->name);
                else
                    mvwprintw(view->win[i].window,lines-view->printstart,view->spe*vf->level,"%s",vf->file->name);
            }
        }
        lines++;
    }
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

    vfile_s* arr1 = get_filearr(view->win[0].cwdnode);
    vfile_s* arr2 = get_filearr(view->win[1].cwdnode);
	
#define CHILDNUM(i) (view->win[i].cwdnode->childnum)
	view->filenum = diff_filelist(arr1, CHILDNUM(0), arr2, CHILDNUM(1), &view->file_list);
    view->printnum = view->filenum - view->printstart;
    if(view->printnum > view->linemax)
        view->printnum = view->linemax;
	
    free(arr1);
    free(arr2);

    //刷新窗口
    print_dirlist(view);
    update_view(view);
}

//移动选中窗口
#if 0
void move_win(view_s *view,int type)
{
    view->curwin++;
    if(view->curwin == &view->win[view->winnums])
        view->curwin = &view->win[0];

}
#endif

//移动光标
void move_line(view_s *view,int type)
{
    if(type == 'j')
    {
        if(view->lineindex + 1 < view->printnum)
            //当光标未达到最下方文件，则光标下移
            view->lineindex++;
        else
        {
            if(view->printstart + view->printnum < view->filenum)
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
#if 0
void toggle_dir(view_s *view)
{
    dirnode_s *pdir;
    filenode_s *file;

    file = win->file_arr[win->printstart + win->y].file;
    if(file->type == DT_DIR)
    {
        pdir = (dirnode_s*)file;
        get_filelist(pdir);
        pdir->showchild = !pdir->showchild;
        clear_view(win);
        print_dirlist(win);
    }
    else
    {
    }
}
#endif

//折叠父目录
#if 0
void fold_dir(win_s *win)
{
    filenode_s *file;
    
    if(win->y < win->printnum)
    {
        file = win->file_arr[win->printstart + win->y].file;
        if(!file->father->file.father)
            return;
        ((dirnode_s*)file->father)->showchild = 0;
        win->y = index_file(win,(filenode_s*)file->father);
        if(win->y < 0)
        {
            win->printstart += win->y;
            win->y = 0;
        }
        clear_win(win);
        print_dirlist(win);
    }
}
#endif

//黏贴文件
#if 0
void past_file(win_s *win)
{
    filenode_s *pfile;
    dirnode_s *fatherdir;

    if(!win->cutfile)
        return;

    if(win->y < win->printnum)
    {
        pfile = win->file_arr[win->printstart + win->y].file;
        if(pfile->type == DT_DIR)
        {
            if(((dirnode_s*)pfile)->showchild)
                fatherdir = (dirnode_s*)pfile;
            else
                fatherdir = pfile->father;
        }
        else
            fatherdir = pfile->father;

        if(win->cutfile->type == DT_DIR)
        {
            //目录拷贝要满足2个条件，1.负责文件不能本来就在目标文件夹下 2.复制文件不能是目标文件本身或直系父目录
            if(win->cutfile->father == fatherdir)
                    return;

            dirnode_s *pdir = fatherdir;
            while(pdir)
            {
                if(pdir == (dirnode_s*)win->cutfile)
                    return;
                pdir = pdir->file.father;
            }
        }
        else
        {
            //文件复制需满足1个条件，文件不能本身在目标文件夹下
            pfile = win->cutfile;
            if(pfile->father == fatherdir)
                return;
        }

        //检测文件复制是否成功
        if(insert_checkfile(fatherdir,win->cutfile))
            return;
        if(win->bcpyfile)
        {
            //复制文件
            copy_file(win->cutfile, fatherdir);
            insert_filenode(fatherdir, win->cutfile,TRUE);
        }
        else
        {
            if(move_file(win->cutfile, fatherdir))
                return;
            delete_file(win->cutfile);
            insert_file(fatherdir, win->cutfile,FALSE);
        }
    }
    win->cutfile = NULL;
    clear_win(win);
    print_dirlist(win);
}
#endif

//剪切文件
#if 0
void cut_file(win_s*win)
{
    if(win->y < win->printnum)
        win->cutfile = win->file_arr[win->printstart + win->y].file;
}
#endif

//复制文件
#if 0
void cpy_file(win_s *win)
{
    if(win->y < win->printnum)
    {
        win->cutfile = win->file_arr[win->printstart + win->y].file;
        win->bcpyfile = 1;
    }
}
#endif

//删除文件
#if 0
void del_file(win_s*win)
{
    filenode_s *file;
    if(win->y < win->printnum)
    {
        file = win->file_arr[win->printstart + win->y].file;
        delete_file(file);
        delete_filenode(file);
        clear_win(win);
        print_dirlist(win,cwdnode);
    }
}
#endif

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
            case 'h':
            case 'l':
                //move_win(&view,ch);
                break;
            case '\n':
                //toggle_dir(view);
                break;
            case 'x':
                //fold_dir(view.curwin);
                break;
            case 'd':
                //if('d' == getch())
                    //cut_file(view.curwin);
                break;
            case 'y':
                //if('y' == getch())
                    //cpy_file(view.curwin);
                break;
            case 'D':
                //del_file(view.curwin);
                break;
            case 'p':
                //past_file(view.curwin);
                break;
            case 0x1b:  //撤销复制
                //view.curwin->cutfile = NULL;
                break;
        }
        flush_attr(&view);
        update_view(&view);
    }
    endwin();
}
































