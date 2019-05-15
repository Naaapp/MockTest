cd ~/MockTest/project4_test/src
make
sudo umount /mnt
sudo rmmod ext42
sudo insmod ext42.ko
sudo mount -t ext42 /dev/sdb /mnt
sudo chmod -R 777 /mnt
cd /mnt
python3