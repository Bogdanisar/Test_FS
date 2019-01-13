#include "test_fs.h"

static struct inode* tfs_root_inode;
static int Inode_Id_Counter = 100;

static struct file_system_type fst = {
	.owner = THIS_MODULE,
	.name = FILE_SYSTEM_NAME,
	.mount = tfs_get_sb,
	.kill_sb = tfs_kill_sb
};

static struct inode_operations tfs_iops = {
	.lookup = tfs_lookup,
};

static struct file_operations tfs_fops = {
	.owner = THIS_MODULE,
	.iterate = tfs_iterate,
	.read = tfs_read
};



// init, exit, mount, fill_sb, kill_sb;

int init_module(void) {

	int err = register_filesystem(&fst);
	if (err != 0) {
		printk(KERN_ALERT "Can't register filesystem\n");
	}
	else {
		printk(KERN_ALERT "Can register filesystem!! :)\n");
	}
	return err;

	return 0;
}

struct dentry* tfs_get_sb (struct file_system_type * type, int flags, const char * dev, void * data) {
	struct dentry * const entry = mount_bdev(type, flags, dev,
                                                  data, tfs_fill_sb);
	if (IS_ERR(entry))
		printk(KERN_ALERT "Cannot mount!! :(\n");
	else
		printk(KERN_ALERT "Can mount!! :)\n");
	return entry;
}

int tfs_fill_sb (struct super_block * sb, void * data, int silent) {

	struct dentry *root_dentry, *zero_node;
	struct list_head *proc_list;

	sb->s_blocksize = 1024;
	sb->s_blocksize_bits = 10;
	sb->s_magic = TFS_MAGIC_NUMBER;
	sb->s_type = &fst;

	tfs_root_inode = new_inode(sb);
	if (!tfs_root_inode) {
		pr_err("inode allocation failed\n");
		return -ENOMEM;
	}

	tfs_root_inode->i_op = &tfs_iops;
    tfs_root_inode->i_mode = S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    tfs_root_inode->i_fop = &tfs_fops;
    tfs_root_inode->i_ino = Inode_Id_Counter++;

    sb->s_root = d_make_root(tfs_root_inode);
    if (!(sb->s_root)) {
    	pr_err("Allocating dentry for root inode\n");
    	return -ENOMEM;
    }

    root_dentry = sb->s_root;
    // root_dentry->d_name.name = "Root of test_fs"; // THIS SEEMS TO BREAK UNMOUNTING
    root_dentry->d_subdirs.prev = root_dentry->d_subdirs.next = &(root_dentry->d_subdirs);
    root_dentry->d_fsdata = root_dentry;

    printk(KERN_ALERT "root_dentry has pointer %p\n", root_dentry);
    printk(KERN_ALERT "root_dentry has parent with pointer %p\n", root_dentry->d_parent);
    if (root_dentry->d_parent != NULL) {
    	printk(KERN_ALERT "name of dentry parent of root_dentry is %s\n", root_dentry->d_parent->d_name.name);
    }

    proc_list = getProcessList();

    zero_node = add_son(root_dentry, "0");
    build_dfs(0, zero_node, proc_list);

    return 0;
}


void tfs_kill_sb (struct super_block * sb) {
	kill_anon_super(sb);
}


void cleanup_module(void) {
	unregister_filesystem(&fst);
}





// iterate, lookup, read operations:

int tfs_iterate (struct file * file, struct dir_context * ctx) {
	bool ret;
	struct list_head *curr_list_item;
	struct dentry *curr_dentry, *parent_dentry;
	int pos = 2;

	if (ctx->pos == 0) {
		ret = dir_emit_dots(file, ctx);

		if (!ret)
		{
			return -ENOSPC;
		}
	}

	parent_dentry = file->f_path.dentry->d_fsdata;
	list_for_each(curr_list_item, &(parent_dentry->d_subdirs)) {
		curr_dentry = list_entry(curr_list_item, struct dentry, d_child);

		if (curr_dentry->d_inode == NULL) {
			continue;
		}

		if (ctx->pos == pos) {
			printk(KERN_ALERT "iterate, ctxpos=%lli: Son of parent_dentry with name %s  is %s\n", ctx->pos, parent_dentry->d_name.name, curr_dentry->d_name.name);

			ret = dir_emit(ctx, curr_dentry->d_name.name, strlen(curr_dentry->d_name.name), curr_dentry->d_inode->i_ino, DT_DIR);
			if (!ret) {
				return -ENOSPC;
			}

			ctx->pos += 1;
		}

		++pos;
	}

	ctx->pos += 100;
	printk(KERN_ALERT "iterate, ctxpos=%lli: Iterate finished finding sons for %s\n", ctx->pos, parent_dentry->d_name.name);
	printk(KERN_ALERT "\n");
	return 1;
}

ssize_t tfs_read (struct file * file, char __user * user_buf, size_t length, loff_t * offset) {
	int ret = 0;
	
	printk(KERN_ALERT "read was done\n");

	if (*offset == 0) {
		put_user('t', user_buf);
		*offset = 1;
		ret = 1;
	}
	return ret;
}


struct dentry * tfs_lookup (struct inode * parent_inode, struct dentry * here_dentry, unsigned int flags) {
	struct list_head * curr_list_item;
	struct dentry * curr_dentry, *parent_dentry;

	parent_dentry = here_dentry->d_parent->d_fsdata;

	printk(KERN_ALERT "Lookup was done for dentry with name %s\n", here_dentry->d_name.name);
	printk(KERN_ALERT "Father dentry name of here_dentry in lookup is %s\n", parent_dentry->d_name.name);

	if (strcmp(here_dentry->d_name.name, ".") == 0) {
		d_add(here_dentry, parent_inode);

		goto fin;
	}

	if (strcmp(here_dentry->d_name.name, "..") == 0) {
		d_add(here_dentry, here_dentry->d_parent->d_inode);
		
		goto fin;
	}

	list_for_each(curr_list_item, &(parent_dentry->d_subdirs)) {
		curr_dentry = list_entry(curr_list_item, struct dentry, d_child);

		if (curr_dentry->d_inode == NULL) {
			printk(KERN_ALERT "Son of %s is %s it has no inode\n", 
				parent_dentry->d_name.name, curr_dentry->d_name.name);
		}
		else {
			printk(KERN_ALERT "Son of %s is %s and the inode has an id of %lu\n", 
				parent_dentry->d_name.name, curr_dentry->d_name.name, curr_dentry->d_inode->i_ino);
		}
	}

	list_for_each(curr_list_item, &(parent_dentry->d_subdirs)) {
		curr_dentry = list_entry(curr_list_item, struct dentry, d_child);

		if (curr_dentry->d_inode == NULL) {
			continue;
		}

		if (strcmp(curr_dentry->d_name.name, here_dentry->d_name.name) == 0) {
			d_add(here_dentry, curr_dentry->d_inode);
			here_dentry->d_fsdata = curr_dentry;

			goto fin;
		}
	}

	d_add(here_dentry, NULL);

fin:
	printk(KERN_ALERT "\n");
	return NULL;
}



// returns the new son dentry if successful, otherwise NULL;
struct dentry * add_son(struct dentry * parent, const char * p_name) {
	struct dentry * son_dentry = NULL;
	struct inode * son_inode = NULL;
	struct super_block * sb = NULL;

	if (parent->d_sb == NULL) {
		printk(KERN_ALERT "add_son: super_block of parent is NULL\n");
		return NULL;
	}
	sb = parent->d_sb;

	son_inode = new_inode(sb);
	if (!son_inode) {
		printk(KERN_ALERT "add_son: Can't allocate new inode\n");
		return NULL;
	}

	son_inode->i_op = &tfs_iops;
    son_inode->i_mode = S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    son_inode->i_fop = &tfs_fops;
    son_inode->i_ino = Inode_Id_Counter++;

	son_dentry = d_make_root(son_inode);
	if (!son_dentry) {
		printk(KERN_ALERT "add_son: Can't allocate new dentry\n");
		return NULL;
	}

	son_dentry->d_sb = sb;
	son_dentry->d_name.name = p_name;
	son_dentry->d_parent = (struct dentry *)parent;
    son_dentry->d_subdirs.prev = son_dentry->d_subdirs.next = &(son_dentry->d_subdirs);

	list_add(&(son_dentry->d_child), &(parent->d_subdirs));
	return son_dentry;
}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Burcea Bogdan-Madalin; Manghiuc Teodor Florin;");