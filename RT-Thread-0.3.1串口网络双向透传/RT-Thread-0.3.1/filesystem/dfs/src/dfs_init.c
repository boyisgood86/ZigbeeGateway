/*
+------------------------------------------------------------------------------
| Project   : Device Filesystem
+------------------------------------------------------------------------------
| Copyright 2004, 2005  www.fayfayspace.org.
| All rights reserved.
|------------------------------------------------------------------------------
| File      : dfs.c, the implementation of Device FileSystem
|------------------------------------------------------------------------------
| Chang Logs:
| Date           Author       Notes
| 2002-12-31     ffxz         The first version.                     
| 2005-01-22     ffxz         Clean up the code.                     
+------------------------------------------------------------------------------
*/
#include <dfs_def.h>
#include <dfs_config.h>

#include <dfs_fs.h>
#include <dfs_raw.h>

#include <string.h>

/* Global variables */
const struct dfs_filesystem_operation* filesystem_operation_table[DFS_FILESYSTEM_TYPES_MAX + 1];
struct dfs_filesystem filesystem_table[DFS_FILESYSTEMS_MAX + 1];

/* device filesystem lock */
struct rt_mutex dlock;

#ifdef DFS_USING_WORKDIR
char working_directory[DFS_PATH_MAX + 1];
#endif

#ifdef DFS_USING_STDIO
struct dfs_fd fd_table[3 + DFS_FD_MAX];
#else
struct dfs_fd fd_table[DFS_FD_MAX];
#endif

/*
+------------------------------------------------------------------------------
| Function    : dfs_init
+------------------------------------------------------------------------------
| Description : Inits the Device Filesystem
| Parameters  : null
| Returns     : null
+------------------------------------------------------------------------------
*/
void dfs_init()
{
	/* clear filesystem operations table */
	rt_memset(filesystem_operation_table, 0, sizeof(filesystem_operation_table));

	/* clear filesystem table */
	rt_memset(filesystem_table, 0, sizeof(filesystem_table));

	/* clean fd table */
	rt_memset(fd_table, 0, sizeof(fd_table));

	/* create device filesystem lock */
	rt_mutex_init(&dlock, "fs_lock", RT_IPC_FLAG_FIFO);

#ifdef DFS_USING_WORKDIR	
	/* set current working directory */
	strcpy(working_directory, "/");
#endif
}

void dfs_lock()
{
	rt_err_t result;

	result = rt_mutex_take(&dlock, RT_WAITING_FOREVER);
	RT_ASSERT(result == RT_EOK);
}

void dfs_unlock()
{
	rt_mutex_release(&dlock);
}

