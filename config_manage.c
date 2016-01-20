#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "config_manage.h"
/*   删除左边的空格   */
static char * del_left_trim(char *str) {
    assert(str != NULL);
    for (;*str != '\0' && isblank(*str) && *str != '\n'; ++str);
    return str;
}
/*   删除两边的空格   */
static char * del_both_trim(char * str) {
    char *p;
    char * szOutput;
    szOutput = del_left_trim(str);
    for (p = szOutput + strlen(szOutput) - 1; p >= szOutput && (isblank(*p)||*p =='\n');
            --p);
    *(++p) = '\0';
     
    return szOutput;
}

//confFile 配置文件路径，请使用绝对路径。相对路径再开机自启程序中会导致程序启动失败
int getConfValue(char *confFile , char *conf_name, char *value, CAST_TYPE type)  
{
    FILE * fp = NULL;
   /*打开配置文件*/
    fp = fopen(confFile, "r");
   /*缓冲区*/
    char buf[64];
    char s[64];
    /*分割符*/
    char * delim = "=";
    char * p;
    char ch;
    int ret =0 ;
    //printf("%s\n",conf_name);   
    while (!feof(fp)) 
    {
        if ((p = fgets(buf, sizeof(buf), fp)) != NULL) 
        {
            strcpy(s, p);
            ch=del_left_trim(s)[0];
            /*判断注释 空行，如果是就直接下次循环*/
            if (ch == '#' || isblank(ch) || ch=='\n')
                continue;
            /*分割字符串*/
            p=strtok(s, delim);
            if(strcmp(del_both_trim(p),conf_name) == 0)
            {
                p = strtok(NULL, delim);
                if(p != NULL)
                {
                    if(type == TYPE_STRING) 
                    {
                        //printf("test\n"); 
                        strcpy(value,del_both_trim(p)) ; 
                        //printf("%s\n",del_both_trim(p));
                    } 
                    if(type == TYPE_INT) 
                    {
                        ret = atoi(del_both_trim(p));
                    }
                    return ret ;
                }
                else
                    return -1;
            }
            else
                continue;
        }
    }
    return -1;
} 

