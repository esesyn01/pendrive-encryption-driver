#!/bin/bash

SOURCE_DEV=$1
SOURCE_MNT_POINT=$2
DISK_IMAGE_NAME="disk-copy.img"
MODE=$3

print_help() {
    echo "Usage: ./cli.sh SOURCE_DEV SOURCE_MOUNT_POINT MODE"
    echo "          SOURCE_DEV - for example /dev/sdb"
    echo "  SOURCE_MOUNT_POINT - mountpoint for device, for example /mount/disk"
    echo "                MODE - 0 for encrypt, 1 for decrypt"
}

error_exit() {
  _error_msg="$1"
  echo "$_error_msg"
  exit 1
}

make_disk_image() {
    dd if=$SOURCE_DEV of=$DISK_IMAGE_NAME
}

load_disk_image() {
    dd if=$DISK_IMAGE_NAME of=$SOURCE_DEV
}

remount_partition() {
    umount $SOURCE_DEV
    mkdir -p $SOURCE_MNT_POINT
    mount $SOURCE_DEV $SOURCE_MNT_POINT
    rm $DISK_IMAGE_NAME
}

run_module() {
    insmod encryption.ko mode=$MODE disk_name=$DISK_IMAGE_NAME
    rmmod encryption
}

if [ $# -ne 3 ]; then
    print_help
    error_exit "You have to pass exactly 3 arguments"
fi

make_disk_image
run_module
load_disk_image
remount_partition


