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

### Nexys Firmware Setup

To upload the firmware to the Nexys Video, upload the .bit file found in this repo onto an SD card. Set the JP3 and JP4 jumpers on the board both to SD and power cycle the board. 
