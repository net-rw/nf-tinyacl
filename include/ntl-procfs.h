/* ntl-procfs.h
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#ifndef __NTL_PROCFS_H__
#define __NTL_PROCFS_H__

#if defined(CONFIG_PROC_FS)
# include <linux/proc_fs.h>
# define ntl_fs_dentry proc_dir_entry
# define ntl_proc_mkdir proc_mkdir
# define ntl_proc_remove proc_remove
#elif defined(CONFIG_DEBUG_FS)
# include <linux/debugfs.h>
# define ntl_fs_dentry dentry
# define ntl_proc_mkdir debugfs_create_dir
# define ntl_proc_remove debugfs_remove_recursive
#else
# error "Neither procfs or debugfs are on!"
#endif

static inline void *
ntl_proc_creat(const char *name, umode_t mode,
               void *parent, void *data,
               const struct file_operations *fops)
{
#if defined(CONFIG_PROC_FS)
    (void)data; /* unused */
    return proc_create(name, mode, parent, fops);
#elif defined(CONFIG_DEBUG_FS)
    return debugfs_create_file(name, mode, parent, data, fops);
#else
    return NULL;
#endif
}

#endif
