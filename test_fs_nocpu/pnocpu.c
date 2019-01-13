#include "test_fs.h"

// int init_module(void) {
// 	printk("proc_tree_no_cpu module is initialized\n");

// 	printProcessList();
// 	return 0;
// }

// void cleanup_module(void) {
// 	printk("proc_tree_no_cpu module is cleanedup\n");
// }

struct list_head* getProcessList(void) {
	struct task_struct * g;
	struct edge * new_element;
	struct list_head * ret = kmalloc(sizeof(struct list_head), GFP_KERNEL);

	INIT_LIST_HEAD(ret);

	rcu_read_lock();
    for_each_process(g) {

    	new_element = kmalloc(sizeof(struct edge), GFP_KERNEL);
    	new_element->pid = g->pid;
    	new_element->ppid = g->parent->pid;
    	new_element->isDirect = true;

    	list_add(&(new_element->list), ret);
    }
    rcu_read_unlock();


	new_element = kmalloc(sizeof(struct edge), GFP_KERNEL);
	new_element->pid = 0;
	new_element->ppid = -1;
	new_element->isDirect = true;

	list_add(&(new_element->list), ret);

    return ret;
}

void printProcessList(void) {
	struct edge *curr_element;
	struct list_head *curr, *ls = getProcessList();

	list_for_each(curr, ls) {
		curr_element = list_entry(curr, struct edge, list);

		printk(KERN_ALERT "ppid: %i; pid: %i\n", curr_element->ppid, curr_element->pid);
	}
}




// dfs function for building the dentry and inode tree from the process list
void build_dfs(int node_id, struct dentry *node_dentry, const struct list_head *proc_list) {
	struct list_head *curr;
	struct edge *curr_element;
	struct dentry *son_dentry;
	char *name;

	list_for_each(curr, proc_list) {
		curr_element = list_entry(curr, struct edge, list);

		if (node_id != curr_element->ppid) {
			continue;
		}

		name = kmalloc(15 * sizeof(char), GFP_KERNEL);
		sprintf(name, "%u", curr_element->pid);

		son_dentry = add_son(node_dentry, name);
		build_dfs(curr_element->pid, son_dentry, proc_list);
	}
}

