


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
        for(j=1;j<=s1len;j++)
        {
            if(eql(s1[j-1],s2[i-1]))
                reg[j][i] = reg[j-1][i-1] + 1;
            else
                reg[j][i] = MAX(reg[j-1][i],reg[j][i-1]);
        }
    }

    char* r1 = malloc(s1len);
    char* r2 = malloc(s2len);
    memset(r1,0,s1len);
    memset(r2,0,s2len);

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
            if(reg[j-1][i-1] >= reg[j-1][i] && reg[j-1][i-1] >= reg[1][i-1])
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
void ld_data(void **s1,void **s2,char **r1,char **r2,int (*eql)(void*,void*))
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

    for(i=0;i<=s2len;i++)
        reg[0][i] = i;
    for(j=0;j<=s1len;i++)
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





