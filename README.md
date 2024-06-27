# eProsima DDS Record & Replay: Server Fork

## Introduction

This repo works similar to eProsima's DDS Record and Replay, the difference being that this fork is tailored to work with servers instead of simple discovery. 

## How It Works

With DDS Record and Replay, you can record any topic on a given domain.


## How to Install

To install this repo, simply run the following after cloning this repo:

```
git clone https://github.com/Ryan-Red/DDS-Record-Replay.git
git submodule init
git submodule update --recursive
sudo ./install_mcap_amd64.sh
sudo ./install_dependencies.sh
```
All dependencies are installed in the install scripts. 


**NOTE:** This MCAP install script uses the linux amd64 release, if using another architecture make sure to follow the same steps and find the right binary [here](https://github.com/foxglove/mcap/releases/)

## Usage guide:

### DDSRecorder

To record all topics on domain, simply run

```
ddsrecord -d DOMAIN
```
Where DOMAIN is the domain we are recording over. It should just be a number. The recording will be saved as an MCAP file. 

The program should print out the topics being recorded and the domain currently being used as a sanity check.

There is a maximum number of samples to record per topic of 50 000 000 samples. This means that when recording a topic publishing at 1000 Hz, you can record up to 50 000 seconds or 13 hours and 58 minutes. 

#### Optional Configuration
If for some reason you need to ignore certain topics, follow [this guide](https://dds-recorder.readthedocs.io/en/latest/rst/recording/usage/configuration.html) to create your configuration file.

To run this configuration file while recording, run:

```
ddsrecord -d DOMAIN -c CONFIGURATION.yaml
```
Where DOMAIN is the domain we are recording on and CONFIGURATION.yaml is the configuration file.


### DDSReplayer

To replay a recording, **first make sure that the discovery servers are running!**

Next, to replay a file simply run:

```
ddsreplayer -i FILE.mcap -d DOMAIN

```
Where FILE.mcap is the MCAP file we want to replay (that we previously recorded) and DOMAIN is the domain we are replaying on.

**NOTE:** We can replay a topic on any domain, not just the one we recorded it on.

#### Optional Configuration

Just like the recorder, there are some extra configuration parameters. Follow [this guide](https://dds-recorder.readthedocs.io/en/latest/rst/replaying/usage/configuration.html) to learn how to create the configuration file.

To run this configuration:

```
ddsreplayer -i FILE.mcap -d DOMAIN -c CONFIGURATION.yaml
```

Where FILE.mcap is the MCAP file we want to replay, DOMAIN is the domain we are recording on and CONFIGURATION.yaml is the configuration file.
