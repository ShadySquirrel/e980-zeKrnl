#!/bin/bash

#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

## Let me introduce myself...
echo -e "#############################################################################################################"
echo -e "###### zeKrnl building script"
echo -e "######"
echo -e "###### Version 2.2, 30/10/2015 - Ultimate Madness (tm) edition."
echo -e "######"
echo -e "###### Written by ShadySquirrel @ github (https://github.com/ShadySquirrel/) AKA ShadySquirrel @ XDA"
echo -e "###### Big thanks to my Uni, for my lack of sleep and increased desire to do anything else but study."
echo -e "######"
echo -e "###### Licenced as GPL, so you can share & edit, just don't make it forget who original author is."
echo -e "#############################################################################################################"
echo -e " "
echo -e " "

## Configuration part

# For Arch linux users (I don't know if anyone else has this problem)
# Our precious distro uses python3 as default, but for successful Android building we need python2.7
# Since we don't have a function like set-defaults on Ubuntu/Mint, we have to do it the old way.
# Make sure you have instaled everything described here:
# ++++ https://wiki.archlinux.org/index.php/Android#Building_Android
# 
# This version of script will check do you have python27 virtual env, activate if you have it, and install if you don't.
# Only two things this script needs are
# a) proper installation of build-tools, as described on link above
# b) a path where you installed/want to install python27 virtual env.

venv_path="/media/data/py27_venv/"

# Now we need some informations about your toolchain.
# Legend:
# - toolchains_dir -> location where toolchains are located
# - toolchain_name -> name of the directory toolchain is located

toolchains_dir="/media/data/toolchains"
toolchain_name="UBER_arm-eabi-4.9"

# Now, name of the configuration file...
defconfig_name="zeKrnl_e980_defconfig"

# Set number of threads/cores. If you have 4 cores, set it to 4+1=5.
# General rule: n cores => value is n+1. Set it to 1 if you want to watch line by line and hunt for errors
jobs=2

# initrd template folder
template_bootimg="initrd"

# Kernel name
KERNEL_NAME=$(sed -n '/DEVEL_NAME/p' Makefile | head -1 | cut -d'=' -f 2)

# Mainline kernel version
MAINLINE_VERSION=$(sed -n '/VERSION/p' Makefile | head -1 | cut -d'=' -f 2)
MAINLINE_PATCHLV=$(sed -n '/PATCHLEVEL/p' Makefile | head -1 | cut -d'=' -f 2)
MAINLINE_SUBLVLV=$(sed -n '/SUBLEVEL/p' Makefile | head -1 | cut -d'=' -f 2)

MAINLINE="$MAINLINE_VERSION.$MAINLINE_PATCHLV.$MAINLINE_SUBLVLV"

# Custom kernel version
KERNEL_VERSION=$(sed -n '/EXTRAVERSION/p' Makefile | head -1 | cut -d'=' -f 2)
KERNEL_BRANCH=$(echo $KERNEL_VERSION | cut -d'_' -f 2)
REAL_VERSION=$(echo $KERNEL_VERSION | cut -d'_' -f 1)


# Build timestamp
TIMESTAMP=$(date +"%d%m%y-%H%M")
BUILD_START_TIME=$(date +"%H:%M:%S")
BUILD_START_DATE=$(date +"%d.%m.%Y")

# Build number prefix
BUILD_NUM_PREFIX="_build_"

# And finaly, information about architecture you're building for. 
# OFC, if you're using this for OGPro, you gonna need arm...
device_arch="arm"

## It's fun time! Don't edit this if you really don't have a need for that.

# Variables needed for command line options. 0 = no, 1=yes
# - Create new config
NEW_CONFIG=0
# - Clean up
CLEAN_UP=0
# - Generate boot.img
BOOT_IMG_GEN=0
# - Generate flashable zip
ZIP_GEN=0
# - Sign flashable zip
ZIP_SIGN=0


# Function to generate boot image and flashable zip
function generate_bootImg {

	echo -e "-> Starting boot.img generation"
	echo -e " "
	# Check if abootimg binary exist	
	echo -e "+ Checking if abootimg is in PATH.."
	abootimg_pwd=$(which abootimg)
	if [ -e "$abootimg_pwd" ]; then
		echo -e "++ abootimg found."
	else
		echo -e "++ abootimg binary not found; aborting"
		exit
	fi
	
	echo -e "+ Checking and creating needed directories..."
	if [ ! -d "$PWD/build_tools/tmp/boot" ]; then
		mkdir -p "$PWD/build_tools/tmp/boot"
		echo "++ tmp directory created"
	fi
	
	if [ ! -d "$PWD/build_tools/out" ]; then
		mkdir -p "$PWD/build_tools/out"
		echo "++ out directory created"
	fi
	
	# Check if there is existing, example boot.img to use as template
	echo -e " "
	echo -e "+ Checking for template initrd folder in build_tools/boot_template/$template_bootimg..."
	if [ -e "$PWD/build_tools/boot_template/bootimg.cfg" ] && [ -d "$PWD/build_tools/boot_template/$template_bootimg" ]; then
		echo -e "++ Template bootimg.cfg and initrd found, using them"
		echo " "
		cp -rvf "$PWD/build_tools/boot_template/bootimg.cfg" "$PWD/build_tools/tmp/boot"
		cp -rvf "$PWD/build_tools/boot_template/$template_bootimg" "$PWD/build_tools/tmp/boot"
		mv "$PWD/build_tools/tmp/boot/$template_bootimg" "$PWD/build_tools/tmp/boot/initrd-tmp"
		
		# Generate new initrd.img based on given folder...
		echo -e " "
		echo -e "++ Generating initrd.img..."
		curPWD=$PWD
		cd "$PWD/build_tools/tmp/boot/initrd-tmp"
		echo -e "+++ Working in $PWD"
		find . | cpio --create --format='newc' > ../initrd
		cd ..
		gzip initrd
		mv initrd.gz initrd.img
		if [ -e "initrd.img" ]; then
			echo -e "+++ initrd.img generated..."
		else
			echo -e "+++ initrd.img generation failed; aborting build."
			exit
		fi
		# return back to working dir...
		cd "$curPWD"
		echo "+++ Back in $PWD"	

	else
		echo -e "++ Templates not found or files missing! Cannot create boot.img without templates. Aborting."
		exit
	fi
	
	# Generate new boot.img
	if [ -e "arch/$ARCH/boot/zImage" ]; then
		echo -e "+ Starting generation of new boot.img"
		echo -e " "
		
		echo -e "++ Copying zImage..."
		cp -rvf "arch/$ARCH/boot/zImage" "build_tools/tmp/boot"
		
		
		
		# Let's check  if zImage is bigger than in config
		# - zImageSize
		zImageSize=$(du -b $PWD/build_tools/tmp/boot/zImage | cut -f 1)
		echo -e "++ zImage size: $zImageSize bytes"
		# - initrd.img size
		rdImageSize=$(du -b $PWD/build_tools/tmp/boot/initrd.img | cut -f 1)
		echo -e "++ initrd size: $rdImageSize bytes"
		# - get page size from boot.cfg
		pageSizeHex=$(cat $PWD/build_tools/tmp/boot/bootimg.cfg | grep pagesize | cut -d'=' -f 2)
		pageSize=$((pageSizeHex))
		echo -e "++ pagesize is $pageSize bytes"		
		# - get image size from boot.cfg
		bImageSizeHex=$(cat $PWD/build_tools/tmp/boot/bootimg.cfg | grep bootsize | cut -d'=' -f 2)
		bImageSize=$((bImageSizeHex))
		echo -e "++ bootimg.cfg allows $bImageSize / $bImageSizeHex in hex"
		
		# - calculate expected size
		newImageSize=$(python $PWD/build_tools/scripts/bootimg_size.py $zImageSize $rdImageSize 0 $pageSize)
		
		
		if [[ $newImageSize -gt $bImageSize ]]; then
			echo -e "++ zImage is bigger than set in bootimg.cfg; changing value in bootimg.cfg"
			sed -ie "s/$bImageSizeHex/$newImageSize/" "$PWD/build_tools/tmp/boot/bootimg.cfg"
		else
			echo -e "++ No need to change bootimg.cfg, zImage size is not over limit"
		fi
		
		
		echo -e "++ Generating boot.img"
		
		abootimg --create "$PWD/build_tools/tmp/boot/boot.img" -f "$PWD/build_tools/tmp/boot/bootimg.cfg" -k "$PWD/build_tools/tmp/boot/zImage" -r "$PWD/build_tools/tmp/boot/initrd.img"
		
		if [ -e "$PWD/build_tools/tmp/boot/boot.img" ]; then
			echo -e "+++ Success, boot.img generated. Checkout build_tools/out/ directory!"
			cp -rvf "$PWD/build_tools/tmp/boot/boot.img" "$PWD/build_tools/out"
			# Silently generate boot.img .md5 file
			curPWD=$PWD
			cd "build_tools/out"
			echo `md5sum boot.img` >> "boot.img.md5sum"
			cd "$curPWD"
		else
			echo -e "+++ Failure, boot.img not generated"
		fi
	else
		echo -e "+ ERROR: zImage not found. Was build a success?"
		exit;
	fi
	
	if [[ $ZIP_GEN -eq 0 ]] && [[ BOOT_IMG_GEN -eq 0 ]]; then
		if [ -e "$PWD/build_tools/out/boot.img"	]; then
			echo -e "+ Do you want to generate flashable zip?"
				select ans in "y" "n"; do
				case $ans in
					y )
						generate_flashableZip
						break;;
					n)
						echo -e "++ Cleaning up..."
						rm -rvf "build_tools/tmp"
						break;;
				esac
			done
		fi
	fi
}

function generate_flashableZip {
	# Generate flashable zip if boot image is created
	echo -e " "
	if [ -e "$PWD/build_tools/out/boot.img"	]; then
		echo -e "+ Generating flashable zip"
		# Create tmp directory if doesn't exist
		if [ ! -d "build_tools/tmp" ]; then
			echo -e "++ Creating tmp directory..."
			mkdir "build_tools/tmp"
		fi
		
		# Create out directory if it doesn't exist
		if [ ! -d "build_tools/out" ]; then
			echo -e "++ Creating out directory..."
			mkdir "build_tools/out"
		fi
		
		# Copy needed files to tmp dir
		echo -e " "
		echo -e "++ Copying needed files..."
		
		cp -rvf "build_tools/zip_file/" "build_tools/tmp"
		cp -rvf "build_tools/out/boot.img" "build_tools/tmp/zip_file/"
		
		# Check if there is modules directory tmp zip_file
		if [ ! -d "build_tools/tmp/zip_file/system/lib/modules" ]; then
			echo -e "++ Creating directory for modules..."
			mkdir -p "build_tools/tmp/zip_file/system/lib/modules"
		fi
		
		# Copy modules
		echo -e " "
		echo -e "++ Copying modules"
		cp -rvf "drivers/crypto/msm/qce40.ko" "build_tools/tmp/zip_file/system/lib/modules"
		cp -rvf "drivers/crypto/msm/qcedev.ko" "build_tools/tmp/zip_file/system/lib/modules"
		cp -rvf "drivers/crypto/msm/qcrypto.ko" "build_tools/tmp/zip_file/system/lib/modules"
		cp -rvf "drivers/scsi/scsi_wait_scan.ko" "build_tools/tmp/zip_file/system/lib/modules"
		cp -rvf "block/test-iosched.ko" "build_tools/tmp/zip_file/system/lib/modules"
		
		echo -e " "
		# Get build number.
		build_num=$(cat .build_no)
		# Generate updater-script and copy
		echo -e "++ Generating updater-script"
		eval python "build_tools/scripts/update_script_gen.py" "$BUILD_START_DATE" "$BUILD_START_TIME" "$build_num" "$REAL_VERSION" "$KERNEL_BRANCH" "build_tools/tmp/zip_file/META-INF/com/google/android/updater-script"
		result=$?
		if [[ $result != 0 ]]; then
			echo -e "++ Generation of updater-script failed... aborting."
			exit
		fi
		echo -e " "
		
		# Generate ZIP
		work_dir=$PWD
		echo -e "++ Changing working dir to tmp/zip_file"
		cd "build_tools/tmp/zip_file/"
		echo -e " "
		echo -e "++ Creating zip file..."
		zip flashable.zip -r .
		echo -e "++ Returning back to work dir"
		cd $work_dir
		
		# Copy zip file to build_tools/out
		echo -e "++ Copying flashable zip from tmp to build_tools/out"
		cp -rvf "$PWD/build_tools/tmp/zip_file/flashable.zip" "$PWD/build_tools/out/flashable.zip"
		
		# Get build num
		
		BUILD_NUM="$BUILD_NUM_PREFIX$build_num"
		
		
		# Renaming zip file
		ZIP_FILE_NAME="$KERNEL_NAME-$MAINLINE$KERNEL_VERSION-$TIMESTAMP$BUILD_NUM.zip"
		echo -e "++ Renaming zip file to $ZIP_FILE_NAME"
		mv "$PWD/build_tools/out/flashable.zip" "$PWD/build_tools/out/$ZIP_FILE_NAME"
		echo -e " "
		
		# Sign zip file
		if [[ ZIP_SIGN -eq 1 ]]; then
			echo -e "++ Signing zip file"
			ZIP_FILE_NAME_SIGNED="$KERNEL_NAME-$MAINLINE$KERNEL_VERSION-$TIMESTAMP$BUILD_NUM-SIGNED.zip"
			java -jar "build_tools/tools/SignApk/signapk.jar" "build_tools/tools/SignApk/testkey.x509.pem" "build_tools/tools/SignApk/testkey.pk8" "build_tools/out/$ZIP_FILE_NAME" "build_tools/out/$ZIP_FILE_NAME_SIGNED"
			echo -e "++ Done."	
		fi
		
		# Generate MD5 sums for zips
		echo -e " "
		echo -e "++ Generating MD5 sums for zip files..."
		cur_dir="$PWD"
		cd "$PWD/build_tools/out/"
		
		md5sum_nosign=$(md5sum $ZIP_FILE_NAME)
		if [[ ZIP_SIGN -eq 1 ]]; then
			md5sum_sign=$(md5sum $ZIP_FILE_NAME_SIGNED)
		fi
		
		cd "$cur_dir"
		
		echo -e "++ $md5sum_nosign"
		if [[ ZIP_SIGN -eq 1 ]]; then
			echo -e "++ $md5sum_sign"
		fi
		echo $md5sum_nosign >> "$PWD/build_tools/out/$ZIP_FILE_NAME.md5sum"
		if [[ ZIP_SIGN -eq 1 ]]; then
			echo $md5sum_sign >> "$PWD/build_tools/out/$ZIP_FILE_NAME_SIGNED.md5sum"
		fi
	
		
		# Cleanup
		echo -e " "
		echo -e "++ Cleaning up..."
		rm -rf "build_tools/tmp"
		
		# Inform
		echo -e " "
		echo -e "++ Your flashable zip can be found in $PWD/build_tools/out"
		echo -e "++ Unsigned flashable zip: $ZIP_FILE_NAME"
		echo -e "++ Signed flashable zip: $ZIP_FILE_NAME_SIGNED"

	else
		echo -e "+ ERROR: no boot.img found. Build failed or -i switch omited."
	fi
}

# Build function
function start_build_cmd {
	echo -e "-> Starting build of $KERNEL_NAME-$MAINLINE$KERNEL_VERSION"
	echo -e " "
	
	# Let's check does defconfig file exist
	echo -e "+ Checking if $defconfig_name exists in $PWD/arch/$ARCH/configs/"
	if [ ! -e "$PWD/arch/$ARCH/configs/$defconfig_name" ]; then
		echo -e "++ ERROR: defconfig doesn't exist. Check the filename and file presence and try again"
		exit
	fi
	
	echo -e " "
	
	##### CHECK CONFIG BLOCK #####
	
	# First, let's check if user wants new configuration or to recreate
	if [[ $NEW_CONFIG -eq 1 ]]; then
		echo -e "++ Removing any old configs, cleaning up and making new config"
		# Check if old config is there, delete it and run mrproper
		if [ -e "$PWD/.config" ]; then 
			make mrproper
			rm -rvf "$PWD/.config";
		fi
		
		make $defconfig_name
		
		if [ -e ".build_no" ]; then
			rm -rvf ".build_no"
		fi
		echo -e "1" >> ".build_no"
		
		# Check if configuration is actually made
		if [ -e "$PWD/.config" ]; then
			echo -e " "
			echo -e "+++ New configuration created!"
			echo -e " "
		else
			echo -e " "
			echo -e "+++ ERROR: make defconfig never finished, aborting..."
			exit
		fi	
	else
		# Let's make sure user isn't a complete idiot...
		echo -e "++ Checking if config exists..."
		if [ ! -e "$PWD/.config" ]; then
			echo -e "+++ Config doesn't exist, creating!"
			make $defconfig_name
		else
			echo -e "+++ Config found, resuming normal operation"
			if [ -e ".build_no"  ]; then
				old_build_num=`cat .build_no`
				new_build_num=$((old_build_num+1))
				rm -r ".build_no"
				echo -e "$new_build_num" >> ".build_no"
			else
				echo -e "1" >> ".build_no"
			fi	
		fi
	fi
	
	##### CHECK CLEAN BLOCK #####	
	if [[ $CLEAN_UP -eq 1 ]]; then
		echo -e "++ Checking if there is something to clean..."
		is_dirty=$(find . -name "*.o" | wc -l)
		if [[ $is_dirty -gt 0 ]]; then
			echo -e "+++ Output is dirty, running make clean"
			make clean
			rm -r ".build_no"
			echo -e "1" >> ".build_no"
		else
			echo -e "+++ Output is not dirty, resuming..."
		fi
	else
		echo -e "++ Not cleaning output. If you're running without -g paramter, build will be dirty"
	fi
	
	
	##### RUN THE BUILD #####
	echo -e "+ All set. Starting build"
	echo -e " "
	echo -e "++ Starting build #$(cat .build_no)"
	echo -e " "				
	time make -j$jobs;
	
	# Check if build was a success and if yes, ask user for boot.img generation
	if [ -e "$PWD/arch/$ARCH/boot/zImage" ]; then
		echo -e " "
		echo -e "+ Build successful."
	else
		echo -e " "
		echo -e "+ Build failed; check output for errors and try again after fixing them"
		exit
	fi
	
	##### CREATE BOOT.IMG #####
	echo -e " "
	if [[ $BOOT_IMG_GEN -eq 1 ]]; then
		generate_bootImg
	else 
		echo -e "++ Not generating boot.img"
		# Check if there is an old boot.img, and remove it.
		if [ -e "$PWD/build_tools/boot.img" ]; then
			echo -e "+ Removing boot.img from previous build"
			rm -rvf "$PWD/build_tools/boot.img"
		fi
	fi
	
	##### CREATE FLASHABLE ZIP #####
	if [[ $ZIP_GEN -eq 1 ]]; then
		if [[ ! $BOOT_IMG_GEN -eq 1 ]]; then
			echo -e "+ You have selected to create a new flashable zip, but you don't want to create new boot.img... Sum-ting-wrong"
			echo -e "++ Generating new boot.img for you..."
			generate_bootImg
		fi
		generate_flashableZip
	else
		echo -e "++ Not generating flashable zip"
	fi
	
	echo -e "+++++ DONE +++++"
	
}

## Function to check does python venv exist
function createVenv {
	pyvenv --system-site-packages --copies "$venv_path"
}

## Print help message if some of script parameters are bad
function print_error_msg {
	echo -e "/*********************** HELP! ***************************/"
	echo -e "To use turn-table mode, don't pass any arguments."
	echo -e " "
	echo -e "To automate script's work, pass needed arguments:"
	echo -e "\t -c || --clean -> Clean up before building"
	echo -e "\t -g || --generate -> Generate new .config"
	echo -e "\t -i || --img -> Generate boot.img"
	echo -e "\t -z || --zip -> Generate flashable zip"
	echo -e "\t -s || --sign -> Sign flashable zip"
	echo -e "\t -h || --help -> displays this message"
	echo -e "\t -j=# || --jobs=# -> number of jobs/threads"
	echo -e "# is a numeric value; 1 for yes, 2 for no"
	echo -e "If some of variables aren't defined, script will let it's"
	echo -e "own free will decide..."
	echo -e "/********************* END HELP! *************************/"
}

# Check-up time! Bend over please...

function check_stuff {
## Check if you're running Arch
echo -e "-> Checking if running on Arch Linux..."

distro=`cat /etc/os-release | grep ID |  cut -d'=' -f 2`

if [ "$distro" == "arch" ]; then
	echo -e "+ Running on Arch Linux"
	if [ -d "$venv_path" ]; then
		echo -e "++ python27 virtual env. installed in $venv_path, activating"
		source "$venv_path/bin/activate"
	else
		echo -e "++ python27 virtual env. not found, checking if installer is present..."
		pyvenv_path=$(which pyvenv)
		if [ -e "$pyvenv_path" ]; then
			echo -e "+++ pyvenv available, installing..."
			createVenv
		else
			echo -e "+++ pyvenv binary not found, aborting..."
			exit
		fi
	fi
fi

echo -e " "

### Check for toolchains
echo -e "-> Checking if toolchain is installed..."

toolchain_path="$toolchains_dir/$toolchain_name/bin/arm-eabi-"
if [ -d "$toolchains_dir/$toolchain_name/bin/" ]; then
	echo -e "+ Toolchain path set to $toolchain_path"
else
	echo -e "+ Toolchain not found on $toolchain_path, aborting..."
	exit
fi

### We're alive, let's create needed variables
echo -e " "
echo -e "-> Setting variables:"
export CROSS_COMPILE=$toolchain_path
echo -e "+ CROSS_COMPILE=$CROSS_COMPILE"
export ARCH=$device_arch
echo -e "+ ARCH=$ARCH"
echo -e "+ building config $defconfig_name"
echo -e "+ Using $jobs threads"

}
########## FUNTIME ###############

echo -e " "
if [[ $# -gt 0 ]]; then
	echo "-> Script run parameters:"
	for i in $@; do
		params+=" $i "
		case $i in
			-h|--help)
				print_error_msg;
				break;;
			-c|--clean)
				echo -e "+ Clean before building"
				CLEAN_UP=1
				shift # past argument
				;;
			-g|--generate)
				echo -e "+ Generate new config and clean"
				NEW_CONFIG=1
				shift # past argument
				;;
			-i|--img)
				echo -e "+ Generate boot.img"
				BOOT_IMG_GEN=1
				shift # past argument
				;;
			-z|--zip)
				echo -e "+ Generate flashable zip"
				ZIP_GEN=1
				shift # past argument
				;;
			-s|--sign)
				echo -e "+ Sign flashable zip"
				ZIP_SIGN=1
				shift # past argument
				;;
			-j=* | --jobs=*)
				nJobs="${i#*=}"
				if [ $nJobs -gt 0 ]; then
					jobs=$nJobs
					echo -e "+ Running with $jobs threads"
				fi
				shift
				;;
			* )
				print_error_msg;
				break;;
		esac
		shift # past argument or value
	done
	echo " "
	echo "-> Running checks..."
	echo " "
	check_stuff
	
	start_build_cmd;
else
	check_stuff;
	start_build;
fi
