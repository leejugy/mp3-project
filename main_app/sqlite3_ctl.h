#ifndef __SQL_CTL__
#define __SQL_CTL__

#include "define.h"
#include <sqlite3.h>

#define SQL_TABLE_NAME_MAX_LEN 128
#define SQL_ARGUMEN_MAX_LEN 128
#define SQL_COMMAN_MAX_LEN 8192

#define SQL_SETTINGS_VOLUME_NAME "volume"
#define SQL_SETTINGS_VOLUME_VALUE "50"

#define SQL_COLUMN_NAME "name"
#define SQL_COLUMN_VALUE "value"

#define SQL_SETTINGS_TABLE "settings"
#define SQL_SETTINGS_TABLE_ARGS \
SQL_COLUMN_NAME " TEXT UNIQUE,"  \
SQL_COLUMN_VALUE " TEXT"

#define SQL_SETTINGS_DB_PATH "/root/settings.db"
#define SQL_VOLUMNE_INIT_VAL \
"'" SQL_SETTINGS_VOLUME_NAME "'," \
"'" SQL_SETTINGS_VOLUME_VALUE "'"

typedef enum
{
    SQL_INSERT,
    SQL_UPDATE,
    SQL_SEARCH,
    SQL_CREATE,
}SQL_COMMAND;

typedef enum
{
    SQL_TABLE_SETTINGS,
    SQL_TABLE_MAX,
}SQL_TABLE;

typedef enum
{
    SQL_SETTINGS_COLUMN_NAME,
    SQL_SETTINGS_COLUMN_VALUE,
}SQL_SETTINGS_COLUMN_MEMBER;

typedef struct
{
    char condition_column[SQL_ARGUMEN_MAX_LEN];
    char condition[SQL_ARGUMEN_MAX_LEN];
    char revised_column[SQL_ARGUMEN_MAX_LEN];
    char set_value[SQL_ARGUMEN_MAX_LEN];
}sql_update_t;

typedef struct
{
    char args[SQL_ARGUMEN_MAX_LEN];
    char condition_column[SQL_ARGUMEN_MAX_LEN];
    char condition[SQL_ARGUMEN_MAX_LEN];
}sql_search_t;

typedef union
{
    char create_args[SQL_ARGUMEN_MAX_LEN];
    char insert_args[SQL_ARGUMEN_MAX_LEN];
    sql_search_t search_args;
    sql_update_t update_args;
}sql_args;

typedef struct
{
    char condition_column[SQL_ARGUMEN_MAX_LEN];
    char condition[SQL_ARGUMEN_MAX_LEN];
    char settings[SQL_ARGUMEN_MAX_LEN];
}sql_args_settings_t;

typedef union
{
    sql_args_settings_t set;
}sql_search_arg_u;

typedef struct
{
    SQL_TABLE table_index;
    sql_search_arg_u args;
}callback_argument_t;

typedef struct 
{
    callback_argument_t callback;
    sqlite3 *handle;
    SQL_COMMAND cmd;
    sem_t sem;
    char table[SQL_TABLE_NAME_MAX_LEN];
    SQL_TABLE table_index;
    sql_args args;
}sql_ctl_t;

typedef struct
{
    char path[PATH_MAX_LEN];
    char table[SQL_TABLE_NAME_MAX_LEN];
    SQL_TABLE table_index;
}sql_init_t;


int sql_init_all();
int sql_insert(sql_ctl_t *sql_ctl, char *insert_argument);
int sql_create_table(sql_ctl_t *sql_ctl, char *table_argument);
int sql_set_settings_integer(char *settings_name, int integer);
int sql_get_settings_integer(char *settings_name);
#endif