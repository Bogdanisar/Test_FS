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
	int numcpu, i, cpu_of_root = -2000;
	const int root = 0;

	INIT_LIST_HEAD(ret);

	rcu_read_lock();
    for_each_process(g) {
        if (g->pid == root) {
        	cpu_of_root = g->cpu;
        }

    	new_element = kmalloc(sizeof(struct edge), GFP_KERNEL);
    	new_element->son_id = g->pid;
    	new_element->parent_id = g->parent->pid;
    	new_element->son_cpu = g->cpu;
    	new_element->parent_cpu = g->parent->cpu;

    	list_add(&(new_element->list), ret);
    }
    rcu_read_unlock();

	numcpu = getNumCPU();
	for (i = 0; i < numcpu; ++i) {
		new_element = kmalloc(sizeof(struct edge), GFP_KERNEL);
		new_element->parent_id = CPUID_TO_LISTID(i);
		new_element->parent_cpu = i;

		new_element->son_id = root;
		new_element->son_cpu = cpu_of_root;

		list_add(&(new_element->list), ret);
	}

    return ret;
}

void printProcessList(void) {
	struct edge *curr_element;
	struct list_head *curr, *ls = getProcessList();

	list_for_each(curr, ls) {
		curr_element = list_entry(curr, struct edge, list);

		printk(KERN_ALERT "ppid: %i; pid: %i\n", curr_element->parent_id, curr_element->son_id);
	}
}



int getNumCPU(void) {
	return (nr_cpu_ids > 10) ? 10 : nr_cpu_ids;
}

// dfs function for building the dentry and inode tree from the process list
void build_dfs(const int node_id, 
			   const int lowest_ancestor_same_cpu_id, struct dentry *ancestor_dentry,
			   const int current_cpu, const struct list_head * const proc_list) {
	struct list_head *curr;
	struct edge *curr_element;
	struct dentry *son_dentry;
	char *name;

	list_for_each(curr, proc_list) {
		curr_element = list_entry(curr, struct edge, list);

		if (node_id != curr_element->parent_id) {
			continue;
		}

		if (curr_element->son_cpu == current_cpu) {
			name = kmalloc(15 * sizeof(char), GFP_KERNEL);
			if (lowest_ancestor_same_cpu_id < 0) { // the ancestor is the root CPU node
				sprintf(name, "%u", curr_element->son_id);
			}
			else if (node_id == lowest_ancestor_same_cpu_id) { // the ancestor is this node (the parent process)
				sprintf(name, "SON_%u", curr_element->son_id);
			}
			else { // the ancestor is not the parent process
				sprintf(name, "KIN_%u", curr_element->son_id);					
			}

			son_dentry = add_son(ancestor_dentry, name);
			build_dfs(curr_element->son_id, curr_element->son_id, son_dentry, current_cpu, proc_list);
		}
		else {
			build_dfs(curr_element->son_id, lowest_ancestor_same_cpu_id, ancestor_dentry, current_cpu, proc_list);
		}
	}
}
