#ifndef test_fs_header_guard
#define test_fs_header_guard

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/threads.h>
#include <linux/cpumask.h>

#define FILE_SYSTEM_NAME "test_fs"
#define TFS_MAGIC_NUMBER 100 


#define CPUID_TO_LISTID(id) (-id - 1)
#define LISTID_TO_CPUID(id) (-(id + 1))

struct edge {
	int parent_id, son_id;
	int parent_cpu, son_cpu;
	struct list_head list;
};


// from proc.c

struct list_head* getProcessList(void);
void printProcessList(void);
int getNumCPU(void);
void build_dfs(const int, const int, struct dentry *, const int, const struct list_head * const);


// from fs.c

struct dentry* tfs_get_sb (struct file_system_type *, int, const char *, void *);
int tfs_fill_sb (struct super_block *, void *, int);
void tfs_kill_sb (struct super_block *);
int tfs_iterate (struct file *, struct dir_context *);
ssize_t tfs_read (struct file *, char __user *, size_t, loff_t *);
struct dentry * tfs_lookup (struct inode *,struct dentry *, unsigned int);

struct dentry * add_son(struct dentry *, const char * const);



#endif // test_fs_header_guard