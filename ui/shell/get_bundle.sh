#!/bin/bash
rm -rf bundle.tar.gz

ifconfig eth1 up
ifconfig eth1 2.3.4.6
ifconfig eth1 netmask 255.255.255.0

wget http://2.3.4.5:5600/bundle.tar.gz
tar -xvf bundle.tar.gz
rm -rf bundle.tar.gz