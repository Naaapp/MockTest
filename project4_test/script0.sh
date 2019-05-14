#!/bin/bash

_disk="/dev/ram0"

if [ "`uname -r | grep 4.4.0`x" = "x" ]; then
	echo "The kernel module is made for Linux 4.4.0"
	exit
fi

sudo modprobe brd rd_nr=1 rd_size=$((1024*1024/2)) &> /dev/null

if [ ! -e companion ]; then
	echo "System Error 1, Please report to TAs"
	exit 1
fi

make clean -C ./companion &> /dev/null
make -C ./companion &> /dev/null
if [ $? -ne 0 ] ; then
	echo "System Error 2, Please report to TAs"
	exit 2
fi

if [ ! -e "./companion/test" ]; then
	echo "System Error 3, Please report to TAs"
	exit 3
fi

/bin/rm -rf src &> /dev/null
tar zxvf src.tar.gz &> /dev/null
if [ $? -ne 0 ] ; then
	echo "System Error 5, Please report to TAs"
	exit 5
fi

if [ ! -e "./src" ]; then
	echo "Could not find expected directory, please compress a directory named src"
	exit 6
fi

echo -n "Compiling kernel module ... "
make clean -C ./src &> /dev/null
make -C ./src &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Could not compile the kernel module using make"
	exit 7
else
	echo "OK"
fi

if [ ! -e ./src/ext42.ko ]; then
	echo "No kernel module binary ext42.ko found after compilation"
	exit 8
fi

make clean -C ./src/apps &> /dev/null
echo -n "Compiling application ... "
make -C ./src/apps &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Could not compile the cmp command using make"
	exit 9
else
	echo "OK"
fi

if [ ! -e ./src/apps/cmp ]; then
	echo "No cmp application binary found after compilation"
	exit 10
fi

sudo umount /mnt &> /dev/null
sudo rmmod ext42 &> /dev/null
echo -n "Installing kernel module ... "
sudo insmod ./src/ext42.ko &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Could not install the kernel module, the name should be ext42.ko"
	exit 11
else
	echo "OK"
fi

sudo mkfs.ext4 -F $_disk &> /dev/null
if [ $? -ne 0 ] ; then
	echo "System Error 14, Please report to TAs"
	exit 14
fi

echo -n "Mounting file system ... "
sudo mount -t ext42 $_disk /mnt &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Failed to mount the file system"
	exit 15
else
	echo "OK"
fi

sudo chmod 777 -R /mnt &> /dev/null
if [ $? -ne 0 ] ; then
	echo "System Error 16, Please report to TAs"
	exit 16
fi

echo -n "Creating a 100KB file ... "
./companion/test -f /mnt/file1 -l 100000000 &> /dev/null
if [ $? -ne 0 ] ; then
	echo "System Error 17, Please report to TAs"
	exit 17
else
	echo "Done"
fi
sync

echo -n "Copying the file ... "
cp /mnt/file1 /mnt/file2 &> /dev/null
if [ $? -ne 0 ] ; then
	echo "System Error 18, Please report to TAs"
	exit 18
else
	echo "Done"
fi

sync

echo -n "Modifying the file ... "
./companion/test -f /mnt/file2 &> /dev/null
if [ $? -ne 0 ] ; then
	echo "System Error 19, Please report to TAs"
	exit 19
else
	echo "Done"
fi

sync

echo -n "Unmounting the file system ... "
sudo umount /mnt &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Failed to unmount the file system"
	exit 15
else
	echo "OK"
fi

echo -n "Uninstalling the kernel module ... "
sudo rmmod ext42 &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Failed to uninstall kernel module"
	exit 15
else
	echo "OK"
fi

sync

echo -n "Installing kernel module ... "
sudo insmod ./src/ext42.ko &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Could not install the kernel module, the name should be ext42.ko"
	exit 11
else
	echo "OK"
fi

sync

echo -n "Mounting the file system ... "
sudo mount -t ext42 $_disk /mnt &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Failed to mount the file system"
	exit 15
else
	echo "OK"
fi

echo -n "Test (cmp command) ... "
cmp -l /mnt/file1 /mnt/file2 > .out.txt
./src/apps/cmp -l /mnt/file1 /mnt/file2 > .out_sh.txt
diff --strip-trailing-cr --ignore-blank-lines --ignore-all-space .out.txt .out_sh.txt &> /dev/null
if [ $? -ne 0 ] ; then
	echo "Wrong Result"
	echo ""
	echo "-- Expected --"
	cat .out.txt
	echo ""
	echo ""
	echo "--  Output  --"
	cat .out_sh.txt
	exit 20
else
	echo "OK"
fi

echo -n "Measuring Performance ... "

sudo sh -c "echo 1 > /proc/sys/vm/drop_caches" &> /dev/null
(time ./src/apps/cmp -l /mnt/file1 /mnt/file2) > .out_sh.txt 2>&1
sudo sh -c "echo 1 > /proc/sys/vm/drop_caches" &> /dev/null
(time cmp -l /mnt/file1 /mnt/file2) > .out.txt 2>&1

_r=(`cat .out.txt |grep real|grep s|sed 's/ //g'|sed 's/\t//g'|sed 's/real//g'|sed 's/s//g'|sed 's/m/\n/g'|xargs`)
_n=(`cat .out_sh.txt |grep real|grep s|sed 's/ //g'|sed 's/\t//g'|sed 's/real//g'|sed 's/s//g'|sed 's/m/\n/g'|xargs`)

_s=`echo "scale=3; (1 - (${_n[0]} * 60 + ${_n[1]}) / (${_r[0]} * 60 + ${_r[1]})) * 100" | bc`

printf "Score: %.1f/100\n" $_s

printf "${SUBMIT_ID}, %.1f" >> private.txt

sudo umount /mnt

echo "Congratulations! Your program has passed all the tests!"

/bin/rm .out.txt
/bin/rm .out_sh.txt

exit $ret
