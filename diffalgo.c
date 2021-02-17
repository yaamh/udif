#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX(x,y) (x>y?x:y)
#define MIN(x,y,z) ((x>y?y:x)>z?z:(x>y?y:x))

void diff_line(const char* s1,const char* s2,char *r1,char *r2)
{
    int s1len = strlen(s1);
    int s2len = strlen(s2);
    int i,j;

    char reg[s1len + 1][s2len + 1];

    for(i=0;i<=s2len;i++)
        reg[0][i] = i;
    for(j=0;j<=s1len;i++)
        reg[j][0] = j;
    for(j=1;j<=s1len;j++)
    {
        for(i=1;i<=s2len;i++)
        {
            if(s1[j-1] == s2[i-1])
                reg[j][i] = reg[j-1][i-1];
            else
                reg[j][i] = MIN(reg[j-1][i],reg[j][i-1],reg[j-1][i-1]) + 1;
        }
    }
    memset(r1,0,s1len);
    memset(r2,0,s2len);
    j = s1len;
    i = s2len;

    while(i||j)
    {
        if(i == 1 && j != 1)
        {
            r1[--j] = 1;
            continue;
        }
        if(i != 1 && j == 1)
        {
            r2[--i] = 1;
            continue;
        }

        if(reg[j-1][i-1] == reg[j][i])
        {
            r1[--j] = 0;
            r2[--i] = 0;
        }
        else
        {
            if(reg[j-1][i] < reg[j][i-1])
            {
                if(reg[j-1][i] < reg[j-1][i-1])
                {
                    r1[--j] = 1;
                    continue;
                }
            }
            else
            {
                if(reg[j][i-1] < reg[j-1][i-1])
                {
                    r2[--i] = 1;
                    continue;
                }
            }
            r1[--j] = 2;
            r2[--i] = 2;
        }
    }
}

//比较2个数据，获取共同，s1,s2以NULL结尾
int lcs_data(void **s1,void **s2,char **r1,char **r2,int (*eql)(void*,void*))
{
    int i,j;
    int s1len,s2len;

    i = 0;s1len=0;
    while(s1[i++])
        s1len++;
    i = 0;s2len=0;
    while(s2[i++])
        s2len++;

    char reg[s1len + 1][s2len + 1];
    memset(reg,0,sizeof(reg));
    for(j=1;j<=s1len;j++)
    {
        for(i=1;i<=s2len;i++)
        {
            if(eql(s1[j-1],s2[i-1]))
                reg[j][i] = reg[j-1][i-1] + 1;
            else
                reg[j][i] = MAX(reg[j-1][i],reg[j][i-1]);
        }
    }

    char* t1 = malloc(s1len);
    char* t2 = malloc(s2len);
    memset(t1,0,s1len);
    memset(t2,0,s2len);

    j = s1len;
    i = s2len;
    while(i&&j)
    {
        if(eql(s1[j-1],s2[i-1]))
        {
            t1[--j] = 1;
            t2[--i] = 1;
        }
        else
        {
            if(reg[j-1][i-1] >= reg[j-1][i] && reg[j-1][i-1] >= reg[j][i-1])
            {
                i--;
                j--;
            }
            else if(reg[j-1][i] < reg[j][i-1])
                i--;
            else
                j--;
        }
    }
    *r1 = t1;
    *r2 = t2;
    return reg[s1len][s2len];
}

//比较2个数据，获取差异，s1,s2以NULL结尾
int ld_data(void **s1,void **s2,char **r1,char **r2,int (*eql)(void*,void*))
{
    int i,j;
    int s1len,s2len;
    char *t1,*t2;

    i = 0;s1len=0;
    if(s1)
    {
        while(s1[i++])
            s1len++;
        t1 = malloc(s1len);
        memset(t1,0,s1len);
    }
    i = 0;s2len=0;
    if(s2)
    {
        while(s2[i++])
            s2len++;
        t2 = malloc(s2len);
        memset(t2,0,s2len);
    }
    
    if(!s1 || !s2)
    {
        if(s1)
        {
            memset(t1,1,s1len);
            *r1 = t1;
            *r2 = NULL;
            return s1len;
        }
        if(s2)
        {
            memset(t2,1,s2len);
            *r2 = t2;
            *r1 = NULL;
            return s2len;
        }
        if(!s1 && !s2)
            return 0;
    }

    char reg[s1len + 1][s2len + 1];

    for(i=0;i<=s2len;i++)
        reg[0][i] = i;
    for(j=0;j<=s1len;j++)
        reg[j][0] = j;
    for(j=1;j<=s1len;j++)
    {
        for(i=1;i<=s2len;i++)
        {
            if(eql(s1[j-1],s2[i-1]))
                reg[j][i] = reg[j-1][i-1];
            else
                reg[j][i] = MIN(reg[j-1][i],reg[j][i-1],reg[j-1][i-1]) + 1;
        }
    }

#if 0
    for(j=0;j<=s1len;j++)
    {
        for(i=0;i<=s2len;i++)
        {
            printf("%d ",reg[j][i]);
        }
        printf("\n");
    }
#endif


    int len = 0;
    j = s1len;
    i = s2len;
    while(i||j)
    {
        len++;
        if(i == 1 && j != 1)
        {
            t1[--j] = 1;
            continue;
        }
        if(i != 1 && j == 1)
        {
            t2[--i] = 1;
            continue;
        }

        if(reg[j-1][i-1] == reg[j][i])
        {
            t1[--j] = 0;
            t2[--i] = 0;
        }
        else
        {
            if(reg[j-1][i] <= reg[j][i-1])
            {
                if(reg[j-1][i] < reg[j-1][i-1])
                {
                    t1[--j] = 1;
                    continue;
                }
            }
            else
            {
                if(reg[j][i-1] < reg[j-1][i-1])
                {
                    t2[--i] = 1;
                    continue;
                }
            }
            t1[--j] = 2;
            t2[--i] = 2;
        }
    }
    *r1 = t1;
    *r2 = t2;
    return len;
}

int test_line_eql(void*s1,void*s2)
{
    if(!strcoll(s1, s2))
        return 1;
    else
        return 0;
}

#if 0
#define C0 "\x1b[0m"
#define C1 "\x1b[31m"
#define C2 "\x1b[31m"
#define C3 "\x1b[31m"
#else
#define C0 
#define C1
#define C2
#define C3
#endif

void test_file(char *file[])
{
    int i;
    char *data[2] = {0};
    int lines[2] = {0};
    char **linearr[2];
    char *r1,*r2;
    for(i=0;i<2;i++)
    {
        int filelen = dump_file(file[i],(void**)&data[i]);
        lines[i] = split_file(data[i],filelen,(void***)&linearr[i]);
    }
    int linelen = ld_data((void**)(linearr[0]),(void**)(linearr[1]),&r1,&r2,test_line_eql);
#if 1 
    for(i=0;i<lines[0];i++)
    {
        if(r1[i] == 0)
            printf(C0"%s\n"C0,linearr[0][i]);
        else if(r1[i] == 1)
            printf(C2"%s++++++++++++++++\n"C0,linearr[0][i]);
        else if(r1[i] == 2)
            printf(C3"%s================\n"C0,linearr[0][i]);
    }
    printf("\n");
    for(i=0;i<lines[1];i++)
    {
        if(r2[i] == 0)
            printf(C0"%s\n"C0,linearr[1][i]);
        else if(r2[i] == 1)
            printf(C2"%s++++++++++++++++\n"C0,linearr[1][i]);
        else if(r2[i] == 2)
            printf(C3"%s================\n"C0,linearr[1][i]);
    }
#endif
}



