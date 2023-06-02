#### 一些记录

- 使用的ffmpeg版本是 5.1.3
- third_party : 用于存放可以直接引用的库
- third_party_src : 用于存放引用库的源代码
- src : 项目源代码
- test : 对src进行的一些测试代码



#### 运行问题

使用以下命令编译后

```bash
./configure --prefix=/home/shecannotsee/desktop/all_code/libraries/ffmpeg-5.1.3 --enable-shared
make 
make install 
```

还需要安装依赖

```bash
sudo apt-get install manpages-posix manpages-posix-dev
```

