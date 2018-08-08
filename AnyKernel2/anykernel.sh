# By KingOfMezi @XDA Developers
# begin properties
properties() {
kernel.string=DirtyV by bsmitty83 @ xda-developers
do.devicecheck=0
do.modules=0
do.cleanup=1
do.cleanuponabort=0
device.name1=hi6250
device.name2=VNS
device.name3=vns
device.name4=venus
device.name5=VENUS
device.name6=nem
device.name7=NEM
device.name8=NEMO
device.name9=nemo
device.name10=NMO
device.name11=nmo
device.name12=PRA
device.name14=pra
device.name15=BLN
device.name16=bln
device.name17=BLL
device.name18=bll
device.name19=RNE
device.name20=rne
} # end properties

# shell variables
block=/dev/block/bootdevice/by-name/boot;
is_slot_device=0;


## AnyKernel methods (DO NOT CHANGE)
# import patching functions/variables - see for reference
. /tmp/anykernel/tools/ak2-core.sh;

## AnyKernel install
dump_boot;

# begin ramdisk changes
write_boot;

## end install

