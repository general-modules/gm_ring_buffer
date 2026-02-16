# gm_ring_buffer

`gm_ring_buffer` 模块基于 POSIX 接口实现，提供通用的环形缓冲区（Ring Buffer）功能。

## 目录结构

```bash
gm_ring_buffer/
├── build/                    # 编译输出目录
├── CMakeLists.txt
├── examples/                 # 示例代码
│   ├── CMakeLists.txt
│   └── example_ring_buffer.c
├── gm_ring_buffer/           # 模块核心源码
│   ├── CMakeLists.txt
│   ├── gm_ring_buffer.c
│   └── gm_ring_buffer.h
├── LICENSE
└── README.md
```

## 编译与运行

### 编译

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

编译完成后，`build` 目录结构如下（仅说明关键文件）：

``` bash
build/
├── examples/
│   └── example_ring_buffer # 可执行文件
└── gm_ring_buffer/
    └── libgm_ring_buffer.a # 静态库
```

### 运行示例

```bash
$ cd build/examples
$ sudo ./example_ring_buffer
```

## 移植

### 方式一：使用源码

将 `gm_ring_buffer` 目录下的源码文件复制到你的项目目录中，并在代码中包含 `gm_ring_buffer.h` 头文件。
可参考 `gm_ring_buffer/CMakeLists.txt` 中的写法，将其作为一个独立模块进行编译。

### 方式二：使用静态库

将生成的 `libgm_ring_buffer.a` 和 `gm_ring_buffer.h` 拷贝到你的项目中，包含 `gm_ring_buffer.h` 头文件并链接 `libgm_ring_buffer.a` 库即可。

## 注意事项

- 接口行为及返回值请以头文件注释为准

## 问题与建议

有任何问题或建议欢迎提交 [issue](https://github.com/general-modules/gm_ring_buffer/issues) 进行讨论。
