#ifndef __DIFFALGO_H__
#define __DIFFALGO_H__



void diff_line(const char* s1,const char* s2,char *r1,char *r2);
int lcs_data(void **s1,void **s2,char **r1,char **r2,int (*eql)(void*,void*));
int ld_data(void **s1,void **s2,char **r1,char **r2,int (*eql)(void*,void*));



#endif




