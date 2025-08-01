ifconfig eth1 2.3.4.6
ifconfig eth1 netmask 255.255.255.0
rm -rf mp3
wget http://2.3.4.5:5600/mp3
chmod +x mp3
gdbserver :1234 mp3