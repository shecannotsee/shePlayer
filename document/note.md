#### 一些记录

- 使用的ffmpeg版本是 5.1.3
- third_party : 用于存放可以直接引用的库
- third_party_src : 用于存放引用库的源代码
- src : 项目源代码
- test : 对src进行的一些测试代码



#### 运行问题

安装sdl1.2

```bash
git clone https://github.com/libsdl-org/SDL-1.2.git
cd SDL-1.2
./configure
make -j8
sudo make install
```

安装sdl2

```bash
git clone https://github.com/libsdl-org/SDL.git -b SDL2
cd SDL
./configure
make
sudo make install
```

ps:编译安装sdl2的时候请注意版本问题,最好使用已经发布的release,否则可能出现未知bug(例如在ubuntu22.04无法显示窗口的问题),并注意版本的兼容问题

在新的机器上还需要

```bash
sudo apt install nasm
```

使用以下命令编译后

```bash
# --enable-shared # 编译动态库
# --enable-libzvbi # 将libz编译需要使用 sudo apt-get install libzvbi-dev
# --enable-sdl
# --enable-sdl2
./configure --prefix=/home/shecannotsee/desktop/all_code/libraries/ffmpeg-5.1.3 --enable-sdl --enable-sdl2 --enable-libzvbi
make 
make install 
```

还需要安装依赖

```bash
sudo apt-get install zlib1g-dev
```

并且需要在链接时加上libz库