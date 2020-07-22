# Irradiation-Test-Scripts

This repo contains the scripts for the testing of ABCStar V1 chips at TRIUMF (August 2020)

## Installation

The installation is assumuing you are on an Ubuntu OS and has been done on Ubuntu 20.04. The itsdaq repo also recommends ROOT 6 as a dependancy.
To start, open a new terminal and make a directory for itsdaq-sw and the irrad. scripts.

```
mkdir <workspace>

cd <workspace>
```

### CERN ROOT Install

To install CERN ROOT first a number of dependancies will be needed: 

```
sudo apt-get install dpkg-dev cmake g++ gcc binutils libx11-dev libxpm-dev \
libxft-dev libxext-dev python gfortran libssl-dev libpcre3-dev \
xlibmesa-glu-dev libglew1.5-dev libftgl-dev \
libmysqlclient-dev libfftw3-dev libcfitsio-dev \
graphviz-dev libavahi-compat-libdnssd-dev \
libldap2-dev python-dev libxml2-dev libkrb5-dev \
libgsl0-dev g++-multilib gcc-multilib
```

Next, download and unpack ROOT:

```
$ wget https://root.cern/download/root_v6.20.04.Linux-ubuntu19-x86_64-gcc9.2.tar.gz
$ tar -xzvf root_v6.20.04.Linux-ubuntu19-x86_64-gcc9.2.tar.gz
```
Finally, ROOT will need to be sourced every time a new terminal is opened. This is easiest done by edidting your .bashrc. 

```
cd ~
```

Open the .bashrc with the editor of choice and add the following line:

```
cd ~/<pathToRoot>/root/bin/; source thisroot.sh; cd -
```

### ITSDAQ Install

If you have left the work area to install root or edit your .bashrc, cd back to the directory you created earlier. 

From here, download the itsdaq software from the cern gitlab. Note,this will require access permission as well as your cern login credentials.

```
git clone https://gitlab.cern.ch/cambridge-ITK/itsdaq-sw
```
Now it is best to create the necessary directories and set the environment variables. First:

```
cd itsdaq-sw; mkdir sctvar; cd sctvar; mkdir data etc ps results; cd ..; cp -r config/ sctvar/;
```
Now, go back to the .bashrc file and add the following lines:

```
export SCTDAQ_ROOT=~/<pathToWorkspace>/itsdaq-sw;
export SCTDAQ_VAR=~/<pathToWorkspace>/itsdaq-sw/sctvar;
```
Now, ITSDAQ can be installed. Return to the itsdaq-sw folder and run the following commands:

```
sudo apt-get install libboost-all-dev libusb-1.0-0-dev libpcap-dev 
python waf configure
python waf build
python waf install 
```
Note: At the time of writing cofigure cannot find the files lmcurve_tyd.h and nivxi.h. It is unknown if this will casue issues.

A bug has been notieced when tryig to load the LV and HV automatic control libraries. To fix this:

```
cd macros; nano Stavelet.cpp

```
Find the following lines of code and change the 1s to zeros. Should look like this when done.
```
  // Load power supplies in ROOT6
  // (may already be loaded in ROOT5 version of rootlogon.C)
  Int_t lvtype = 0; // 0 indicates manual control of LV,
                    // 4 indicates ITSDAQ GUI control of LV,
                    // 130 indicates StarChips ASIC test setup with QL355TP
                    // (according to https://twiki.cern.ch/twiki/bin/view/Atlas>
  Int_t hvtype = 0;
```

### Setup the HSIO
Enter the following commands to setup the HSIO FIFOs

```
mkfifo /tmp/hsioPipe.fromHsio
mkfifo /tmp/hsioPipe.toHsio 
```
### Download the Irrad. Scripts

In the <workspace> directory clone this repo:
  
```
git clone https://github.com/GrGreig/Irradiation-Test-Scripts
```
Now move the macro script to the itsdaq-sw directory:

```
cp Irradiation-Test-Scripts/RegisterReadBack_V0.C itsdaq-sw/
```

### Nexys Firmware Setup

To upload the firmware to the Nexys Video, upload the .bit file found in this repo onto an SD card. Set the JP3 and JP4 jumpers on the board both to SD and power cycle the board. 

### Running a Test

Now the software should be setup to run a test.

Start by setting up the HSIO pipe. Navigate to the itsdaq-sw directory and run the following command:

```
ifconfig
```

This will give the network parameters needed for raw connection to the HSIO. The inet parameter will give the MAC addess needed to connect with the device. The name of the device will be a headder to the packet of information. It is up to the user to determine which port the nexysis connected on. Using this information the hsio pipe can be setup with the following commands:
```
sudo su;

bin/hsioPipe --eth eth1,e0:dd:cc:bb:aa:00 --file /tmp/hsioPipe.toHsio, /tmp/hsioPipe.fromHsio &
```
Here, `eth1` and `e0:dd:cc:bb:aa:00` are taken as outputs from the `ifconfig` query.

Following this, open a new terminal and navigate to the itsdaq-sw folder:

```
cd $SCTDAQ_ROOT
```
Run the following commnad and enter your initials when prompted.

``` 
./RUNITSDAQ.sh
```



