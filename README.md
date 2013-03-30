#xmradio
=======

Xia Mi Radio (虾米电台)

Linux client of http://www.xiami.com/radio


## How It Works

See src/lib/xmrservice.c for more details


## Build From Source

Dependencies see: debian/control (DEB) and opensuse/xmradio.spec (RPM)

cd PROJECT_DIR

mkdir build

cd build

cmake .. -DCMKAE_INSTALL_PREFIX=YOUR_PREFIX OTHER_OPTIONS

make

make install

## Packages

* openSUSE 12.2 and Tumbleweed

[Install from marguerite's repo](http://software.opensuse.org/package/xmradio) (You have to click "unsupported distribution" link due to a s.o.o repo recognition error for 12.2)

* Ubuntu PPA available(11.10 ~ 13.04)

sudo apt-add-repository ppa:timxx/xmradio

sudo apt-get update

sudo apt-get install xmradio

* Arch Linux

[Install from cuihao's AUR](https://aur.archlinux.org/packages/xmradio-git/)

yaourt -S xmradio-git
