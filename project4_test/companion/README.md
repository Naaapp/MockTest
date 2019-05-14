# How to use

## Create a file whose content is random.

```
$ ./test -f FILE_NAME -l FILE_SIZE
```

## Modify some parts of a file

```
$ ./test -f FILE_NAME
```

# What does the test script?

```
$ ./test -f file1 -l 100000000 # Create a random file
$ cp file1 file2
$ ./test -f file2 # Modify one of two files
$ cmp -l file1 file2 > cmp.txt
$ YOUR_cmp -l file1 file2 > YOUR_cmp.txt
$ diff cmp.txt YOUR_cmp.txt
```

If the outputs of the original cmp command and your cmp command are same,
your program passes the test script.

```file1``` and ```file2``` will be located in the mount point of your file system kernel module.
