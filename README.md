# MyFileSystem

A course assignment

a UnixV6-like file system

designed by Mo and Wang

* The normal use of 'tree' command requires 
the terminal to be set to the utf8 character set
instead of gbk or gb2312 character set.

* The inode of the filesystem has 6 direct pointers, 
2 single indirect pointers, and 2 double indirect pointers

* The blocks of the disk 
(or the disk emulated by a large file in the physical machine) 
is managed by **group link method**.

* This filesystem has a caching machanism.


