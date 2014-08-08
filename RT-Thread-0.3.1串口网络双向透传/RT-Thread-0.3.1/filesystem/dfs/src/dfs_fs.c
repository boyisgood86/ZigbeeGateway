/*
+------------------------------------------------------------------------------
| Project   : Device Filesystem
+------------------------------------------------------------------------------
| Copyright 2004, 2005  www.fayfayspace.org.
| All rights reserved.
|------------------------------------------------------------------------------
| File      : dfs_fs.c, the filesystem related implementations of Device FileSystem
|------------------------------------------------------------------------------
| Chang Logs:
| Date           Author       Notes
| 2005-02-22     ffxz         The first version.
+------------------------------------------------------------------------------
*/
#include <dfs_fs.h>
#include <dfs_raw.h>
#include <dfs_util.h>

#include <string.h>

/*
+------------------------------------------------------------------------------
| Function    : dfs_register
+------------------------------------------------------------------------------
| Description : registers a filesystem
|
| Parameters  : ops, the implementation of filesystem
| Returns     : the status of register
|               0 , successful
|               -1, failed
+------------------------------------------------------------------------------
*/
int dfs_register(const struct dfs_filesystem_operation* ops)
{
    int index, result;

	result = 0;

	/* lock filesystem */
	dfs_lock();

    /* check if this filesystem was already registered */
    for (index = 0; index < DFS_FILESYSTEM_TYPES_MAX; index++)
    {
        if (filesystem_operation_table[index] != RT_NULL &&
                strcmp(filesystem_operation_table[index]->name, ops->name) == 0)
        {
        	result = -1;
            goto err;
        }
    }

    /* find out an empty filesystem type entry */
    for (index = 0; index < DFS_FILESYSTEM_TYPES_MAX && filesystem_operation_table[index] != RT_NULL;
            index++) ;

    /* filesystem type table full */
    if (index == DFS_FILESYSTEM_TYPES_MAX)
	{
		result = -1;
		goto err;
    }

    /* save the filesystem's operations */
    filesystem_operation_table[index] = ops;

err:
	dfs_unlock();
    return result;
}

/* 
+------------------------------------------------------------------------------
| Function    : dfs_filesystem_lookup
+------------------------------------------------------------------------------
| Description : lookup the mounted filesystem on the specified path
|
| Parameters  : path, the specified path string
| Returns     : the found filesystem
+------------------------------------------------------------------------------
*/
struct dfs_filesystem* dfs_filesystem_lookup(const char *path)
{
    struct dfs_filesystem* fs;
    rt_uint32_t index, fspath, prefixlen;

    fs = RT_NULL;
    prefixlen = 0;

    /* lock filesystem */
	dfs_lock();

    /* lookup it in the filesystem table */
    for (index = 0; index < DFS_FILESYSTEMS_MAX + 1; index++)
    {
        fspath = strlen(filesystem_table[index].path);
        if (fspath < prefixlen) continue;

        if ((filesystem_table[index].ops != RT_NULL) &&
                str_is_prefix(filesystem_table[index].path, path) == 0)
        {
            fs = &filesystem_table[index];
            prefixlen = fspath;
        }
    }

	dfs_unlock();

    return fs;
}

/* 
+------------------------------------------------------------------------------
| Function    : dfs_filesystem_get_partition
+------------------------------------------------------------------------------
| Description : get partition in the buffer data
|
| Parameters  : part, the returned partition information
|               buf, the buffer to save partition data
|               pindex, the partion index
| Returns     : RT_EOK on successful, -RT_ERROR on failed
+------------------------------------------------------------------------------
*/
rt_err_t dfs_filesystem_get_partition(struct dfs_partition* part, rt_uint8_t* buf, rt_uint32_t pindex)
{
#define DPT_ADDRESS		0x1be		/* device partition offset in Boot Sector */
#define DPT_ITEM_SIZE	16			/* partition item size */

    rt_uint8_t* dpt;
    rt_uint8_t type;
    rt_err_t result;

    RT_ASSERT(part != RT_NULL);
    RT_ASSERT(buf != RT_NULL);

    result = RT_EOK;

    dpt = buf + DPT_ADDRESS + pindex * DPT_ITEM_SIZE;

    if ((*dpt != 0x80) && (*dpt != 0x00))
    {
        /* which is not a partition table */
        result = -RT_ERROR;
        return result;
    }

    /* get partition type */
    type = *(dpt+4);

    if (type != 0)
    {
        /* set partition type */
        part->type = type;

        /* get partition offset and size */
        part->offset = *(dpt+ 8) | *(dpt+ 9) << 8 |
                       *(dpt+10) << 16 | *(dpt+11) << 24;
        part->size = *(dpt+12) | *(dpt+13) << 8 |
                     *(dpt+14) << 16 | *(dpt+15) << 24;

#ifdef RT_USING_FINSH
        rt_kprintf("part[%d], begin: %d, size: ",
                   pindex, part->offset * 512);
        if ( (part->size>>11) > 0 ) /* MB */
        {
            unsigned int part_size;
            part_size = part->size>>11;/* MB */
            if ( (part_size>>10) > 0) /* GB */
            {
                rt_kprintf("%d.%d%s",part_size>>10,part_size&0x3FF,"GB\r\n");/* GB */
            }
            else
            {
                rt_kprintf("%d.%d%s",part_size,(part->size>>1)&0x3FF,"MB\r\n");/* MB */
            }
        }
        else
        {
            rt_kprintf("%d%s",part->size>>1,"KB\r\n");/* KB */
        }
#endif
    }
    else
    {
        result = -RT_ERROR;
    }

    return result;
}

/*
+------------------------------------------------------------------------------
| Function    : dfs_mount
+------------------------------------------------------------------------------
| Description : mount a filesystem to specified path
|
| Parameters  : device_name, the implementation of filesystem
|               path,
|               filesystemtype,
|               rwflag,
|               data,
| Returns     : the status of register
|               0 , successful
|               -1, failed
+------------------------------------------------------------------------------
*/
int dfs_mount(const char* device_name, const char* path,
              const char* type, unsigned long rwflag, const
              void* data)
{
    const struct dfs_filesystem_operation* ops;
    struct dfs_filesystem* fs;
    char *fullpath=RT_NULL;
#ifdef DFS_USING_WORKDIR
    char full_path[DFS_PATH_MAX + 1];
#endif
    rt_device_t dev_id;
    int index;

    /* open specific device */
    dev_id = rt_device_find(device_name);
    if (dev_id == RT_NULL)
    {
        /* no this device */
        rt_set_errno(-DFS_STATUS_ENODEV);
        return -1;
    }

    /* find out specific filesystem */
	dfs_lock();
    for ( index = 0; index < DFS_FILESYSTEM_TYPES_MAX; index++ )
    {
        if (strcmp(filesystem_operation_table[index]->name, type) == 0)break;
    }

    /* can't find filesystem */
    if ( index == DFS_FILESYSTEM_TYPES_MAX )
    {
        rt_set_errno(-DFS_STATUS_ENODEV);
        dfs_unlock();
        return -1;
    }
    ops = filesystem_operation_table[index];
    dfs_unlock();

    /* make full path for special file */
    fullpath = (char*)path;
    if ( fullpath[0] != '/') /* not an abstract path */
    {
#ifdef DFS_USING_WORKDIR
        /* build full path */
        fullpath = full_path;
        dfs_lock();
        build_fullpath(working_directory, path, fullpath);
        dfs_unlock();
#else
        rt_set_errno(-DFS_STATUS_ENOTDIR);
        return -1;
#endif
    }

    /* Check if the path exists or not, raw APIs call, fixme */
    if ( (strcmp(fullpath, "/") != 0) && (strcmp(fullpath, "/dev") != 0) )
    {
        struct dfs_fd fd;

        if ( dfile_raw_open(&fd, fullpath, DFS_O_RDONLY | DFS_O_DIRECTORY) < 0 )
        {
            rt_set_errno(-DFS_STATUS_ENOTDIR);
            return -1;
        }
        dfile_raw_close(&fd);
    }

    /* check whether the file system mounted or not */
	dfs_lock();
    for (index =0; index < DFS_FILESYSTEMS_MAX; index++)
    {
        if ( filesystem_table[index].ops != RT_NULL &&
                strcmp(filesystem_table[index].path, path) == 0 )
        {
            rt_set_errno(-DFS_STATUS_EINVAL);
            goto err1;
        }
    }

    /* find out en empty filesystem table entry */
    for (index = 0; index < DFS_FILESYSTEMS_MAX && filesystem_table[index].ops != RT_NULL;
            index++) ;
    if ( index == DFS_FILESYSTEMS_MAX )	/* can't find en empty filesystem table entry */
    {
        rt_set_errno(-DFS_STATUS_EMMOUNT);
        goto err1;
    }

    /* register file system */
    fs = &(filesystem_table[index]);
    strncpy(fs->path, fullpath, strlen(fullpath));
    fs->ops = ops;
    fs->dev_id = dev_id;
    /* release filesystem_table lock */
	dfs_unlock();

    /* open device, but do not check the status of device */
    rt_device_open(fs->dev_id, RT_DEVICE_OFLAG_RDWR);

    if ( ops->mount == RT_NULL ) /* there is no mount implementation */
    {
    	dfs_lock();
        /* clear filesystem table entry */
        rt_memset(fs, 0, sizeof(struct dfs_filesystem));
		dfs_unlock();

        rt_set_errno(-DFS_STATUS_ENOSYS);
        return -1;
    }
    /* call mount of this filesystem */
    else if ( ops->mount(fs) < 0 )
    {
        /* mount failed */
		dfs_lock();

        /* close device */
        rt_device_close(fs->dev_id);

        /* clear filesystem table entry */
        rt_memset(fs, 0, sizeof(struct dfs_filesystem));

		dfs_unlock();
        return -1;
    }

    return 0;

err1:
    dfs_unlock();
    return -1;
}

/* 
+------------------------------------------------------------------------------
| Function    : dfs_unmount
+------------------------------------------------------------------------------
| Description : unmount a filesystem
|
| Parameters  :
| Returns     : the status of register
|               0 , successful
|               -1, failed
+------------------------------------------------------------------------------
*/
int dfs_unmount(const char *specialfile)
{
    char *fullpath;
#ifdef DFS_USING_WORKDIR
    char full_path[DFS_PATH_MAX + 1];
#endif
    struct dfs_filesystem* fs = RT_NULL;

    fullpath = (char*)specialfile;
    if ( fullpath[0] != '/')
    {
#ifdef DFS_USING_WORKDIR
        /* build full path */
        fullpath = full_path;
		dfs_lock();
        build_fullpath(working_directory, specialfile, fullpath);
        dfs_unlock();
#else
        rt_set_errno(-DFS_STATUS_ENOTDIR);
        return -1;
#endif
    }

    /* lock filesystem */
    dfs_lock();

    fs = dfs_filesystem_lookup(fullpath);
    if (fs != RT_NULL && fs->ops->unmount != RT_NULL && fs->ops->unmount(fs) < 0)
    {
        goto err1;
    }

    /* close device, but do not check the status of device */
    rt_device_close(fs->dev_id);

    /* clear this filesystem table entry */
    rt_memset(fs, 0, sizeof(struct dfs_filesystem));

	dfs_unlock();

    return 0;

err1:
	dfs_unlock();

    return -1;
}

