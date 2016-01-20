#ifndef _config_manage_h__
#define _config_manage_h__

typedef enum _tyep {  
    TYPE_INT = 0,  
    TYPE_STRING  
}CAST_TYPE;


/* 获取配置项的值*/
int getConfValue(char *confFile, char *conf_name, char *value, CAST_TYPE type);

#endif
