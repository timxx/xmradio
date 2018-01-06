# xmradio

## xmradio is no longer maintained
## 已弃疗，喜欢用客户端听歌的朋友，推荐使用网易云音乐客户端（支持Linux）。可使用工具（[点我去看看](https://github.com/timxx/musicexporter)）导出虾米音乐收藏的歌曲列表，然后导入到网易的（貌似只有低版本的才有导入功能，比如2.0）


Xia Mi Radio (虾米电台)

Linux client of http://www.xiami.com/radio


## How It Works

See src/lib/xmrservice.c for more details


## Build From Source

Dependencies see: debian/control (DEB) and opensuse/xmradio.spec (RPM)

mkdir build

cd build

cmake ../SOURCE_CODE_DIR -DCMAKE_INSTALL_PREFIX=YOUR_PREFIX OTHER_OPTIONS

make

make install

## Packages

* openSUSE 12.2 and Tumbleweed

[Install from marguerite's repo](http://software.opensuse.org/package/xmradio) (You have to click "unsupported distribution" link due to a s.o.o repo recognition error for 12.2)

* Ubuntu PPA available(11.10 ~ 14.04)

sudo apt-add-repository ppa:timxx/xmradio

sudo apt-get update

sudo apt-get install xmradio

* Arch Linux

[Install from cuihao's AUR](https://aur.archlinux.org/packages/xmradio-git/)

yaourt -S xmradio-git
