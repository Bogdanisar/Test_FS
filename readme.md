THIS MODULE IS NOT SAFE TO USE ON YOUR RUNNING OPERATING SYSTEM. THIS SHOULD ONLY BE USED ON A VIRTUAL MACHINE WITH BACKUPS. ANY SIDE EFFECTS HAPPENING TO YOUR OPERATING SYSTEM ARE YOUR RESPONSABILITY.

This was tested on Linux Kernel version 4.15.0-43-generic on Ubuntu

uname -a returns:
Linux bogdan-VirtualBox 4.15.0-43-generic #46-Ubuntu SMP Thu Dec 6 14:45:28 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux

test_fs_cpu and test_fs_nocpu both contain a module that build a file system, however they build a different directory tree. They both take the process tree as of the time of mounting and build a directory tree using that information.

test_fs_cpu builds a forest where each root represents a cpu and the nodes represent processes which belong to that cpu.
test_fs_nocpu builds a tree with no cpu nodes, and a father-son edge in the tree is given by a parent-child process relation.

How to use:
1. Navigate with the terminal in test_fs_cpu or test_fs_nocpu
2. run the following commands:

```    
make
sudo insmod test_fs.ko
var= && sudo mount -t test_fs ./image$var ./mount$var -o loop
```

3. Use the terminal and ls, cd commands to navigate ./mount or the file explorer (e.g. nautilus).

4. Unmount using the command:
```
sudo umount -l mount
```

5. Remove the module with:
```
sudo rmmod test_fs
```

Careful: after unmounting a certain directory, you need to mount on a different one afterwards.
So if you want to mount multiple times, you would use commands such as:
```
var= && sudo mount -t test_fs ./image$var ./mount$var -o loop
var=1 && sudo mount -t test_fs ./image$var ./mount$var -o loop
var=2 && sudo mount -t test_fs ./image$var ./mount$var -o loop
var=3 && sudo mount -t test_fs ./image$var ./mount$var -o loop
var=4 && sudo mount -t test_fs ./image$var ./mount$var -o loop
etc
```

