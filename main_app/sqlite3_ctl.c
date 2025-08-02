#include "sqlite3_ctl.h"
#include "alsa_ctl.h"

sql_ctl_t sql[SQL_TABLE_MAX] = {0, };

static int sql_init(sql_ctl_t *sql_ctl, sql_init_t *sql_it)
{
    if(sqlite3_open(sql_it->path, &sql_ctl->handle) < 0)
    {
        printr("fail to open sqlite3 : %s", sqlite3_errmsg(sql_ctl->handle));
        return -1;
    }

    if(sem_init(&sql_ctl->sem, 0, 1) < 0)
    {
        printr("fail to init sem : %s", strerror(errno));
        return -1;
    }

    strcpy(sql_ctl->table, sql_it->table);
    sql_ctl->table_index = sql_it->table_index;
    return 1;
}

static int sql_call_back(void *argument, int argc, char **argv, char **column)
{
    callback_argument_t *callback_argument = argument;

    switch (callback_argument->table_index)
    {
    case SQL_TABLE_SETTINGS:
        if(strcmp(column[SQL_SETTINGS_COLUMN_NAME], SQL_COLUMN_NAME) == 0)
        {
            if(strcmp(argv[SQL_SETTINGS_COLUMN_NAME], SQL_SETTINGS_VOLUME_NAME) == 0)
            {
                strcpy(callback_argument->args.set.volume, argv[SQL_SETTINGS_COLUMN_VALUE]);
            }
        }
        break;
    
    default:
        break;
    }
}

static int sql_command(sql_ctl_t *sql_ctl)
{
    char sql_command_string[SQL_COMMAN_MAX_LEN] = {0, };
    char *errmsg = NULL;
    switch (sql_ctl->cmd)
    {
    case SQL_CREATE:
        sprintf(sql_command_string, "CREATE TABLE IF NOT EXISTS `%s` (%s);", sql_ctl->table, sql_ctl->args.create_args);
        break;

    case SQL_INSERT:
        sprintf(sql_command_string, "INSERT INTO `%s` VALUES(%s);", sql_ctl->table, sql_ctl->args.insert_args);
        break;

    case SQL_SEARCH:
        sprintf(sql_command_string, "SELECT * FROM `%s` WHERE %s;", sql_ctl->table, sql_ctl->args.search_args);
        if(sql_ctl->table_index == SQL_TABLE_SETTINGS)
        {
            strcpy(sql_ctl->callback.args.set.setup_name, sql_ctl->args.update_args.condition_column);
        }
        sql_ctl->callback.table_index = sql_ctl->table_index;
        break;

    case SQL_UPDATE:
        sprintf(sql_command_string, "UPDATE `%s` SET `%s` = '%s' where `%s` = '%s';",
            sql_ctl->table, sql_ctl->args.update_args.revised_column, sql_ctl->args.update_args.set_value,
            sql_ctl->args.update_args.condition_column, sql_ctl->args.update_args.condition);
        break;
    
    default:
        break;
    }
    
    printb("command : %s", sql_command_string);
    if(sqlite3_exec(sql_ctl->handle, sql_command_string, &sql_call_back, &sql_ctl->callback, &errmsg) < 0)
    {
        printr("fail to exec : %s", errmsg);
        return -1;
    }

    return 1;
}

int sql_get_volume()
{
    int ret = 0;

    sem_wait(&sql[SQL_TABLE_SETTINGS].sem);
    sql[SQL_TABLE_SETTINGS].cmd = SQL_SEARCH;
    sprintf(sql[SQL_TABLE_SETTINGS].args.search_args, "`%s` = '%s'", SQL_COLUMN_NAME, SQL_SETTINGS_VOLUME_NAME);
    ret = sql_command(&sql[SQL_TABLE_SETTINGS]);
    sem_post(&sql[SQL_TABLE_SETTINGS].sem);

    if(ret < 0)
    {
        printr("fail to get volume");
        ret = ALSA_VOLUME_INIT_VAL;
    }
    else
    {
        ret = atoi(sql[SQL_TABLE_SETTINGS].callback.args.set.volume);
    }
    return ret;
}

int sql_set_volume(int volume)
{
    int ret = 0;

    sem_wait(&sql[SQL_TABLE_SETTINGS].sem);
    sql[SQL_TABLE_SETTINGS].cmd = SQL_UPDATE;
    strcpy(sql[SQL_TABLE_SETTINGS].args.update_args.condition_column, SQL_COLUMN_NAME);
    strcpy(sql[SQL_TABLE_SETTINGS].args.update_args.condition, SQL_SETTINGS_VOLUME_NAME);
    strcpy(sql[SQL_TABLE_SETTINGS].args.update_args.revised_column, SQL_COLUMN_VALUE);
    sprintf(sql[SQL_TABLE_SETTINGS].args.update_args.set_value, "%d", volume);
    ret = sql_command(&sql[SQL_TABLE_SETTINGS]);
    sem_post(&sql[SQL_TABLE_SETTINGS].sem);

    if(ret < 0)
    {
        printr("fail to set volume");
    }
    return ret;
}

int sql_create_table(sql_ctl_t *sql_ctl, char *table_argument)
{
    int ret = 0;

    sem_wait(&sql_ctl->sem);
    sql[SQL_TABLE_SETTINGS].cmd = SQL_CREATE;
    strcpy(sql_ctl->args.create_args, table_argument);
    ret = sql_command(sql_ctl);
    sem_post(&sql_ctl->sem);

    if(ret < 0)
    {
        printr("fail to create volume");
    }
    return ret;
}

int sql_insert(sql_ctl_t *sql_ctl, char *insert_argument)
{
    int ret = 0;

    sem_wait(&sql_ctl->sem);
    sql[SQL_TABLE_SETTINGS].cmd = SQL_INSERT;
    strcpy(sql_ctl->args.insert_args, insert_argument);
    ret = sql_command(sql_ctl);
    sem_post(&sql_ctl->sem);

    if(ret < 0)
    {
        printr("fail to insert table");
    }
    return ret;
}

int sql_init_all()
{
    sql_init_t sql_it = {0, };
    
    strcpy(sql_it.path, SQL_SETTINGS_DB_PATH);
    strcpy(sql_it.table, SQL_SETTINGS_TABLE);
    sql_it.table_index = SQL_TABLE_SETTINGS;
    sql_init(&sql[SQL_TABLE_SETTINGS], &sql_it);
    sql_create_table(&sql[SQL_TABLE_SETTINGS], SQL_SETTINGS_TABLE_ARGS);
    sql_insert(&sql[SQL_TABLE_SETTINGS], SQL_VOLUMNE_INIT_VAL);
}