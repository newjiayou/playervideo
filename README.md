# QtVideoPlayer - 基于 FFmpeg 的高性能多线程播放器

本项目是一款基于 **Qt 6** 与 **FFmpeg** 开发的视频播放引擎。采用多线程异步解码架构，深度优化了音视频同步算法，支持 4K 视频稳定播放。

## 🛠 开发环境与依赖 (重要)

由于 FFmpeg 开发库文件较大，本仓库未包含其二进制文件。在尝试编译运行前，**请务必按照以下步骤配置环境**：

### 1. 环境要求
- **操作系统**：Windows (10/11)
- **编译器**：MinGW 64-bit (建议与 Qt 版本配套)
- **Qt 版本**：Qt 6.2 或更高版本
- **FFmpeg 版本**：FFmpeg 6.x (Shared 或 Dev 版本)

### 2. FFmpeg 目录配置
请前往 [FFmpeg 官网](https://ffmpeg.org/download.html) 或 [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) 下载对应的 **Shared** 开发包，并解压至项目根目录下的 `3rdparty/ffmpeg` 文件夹中。

**必须确保目录结构如下所示**：
```text
videoplayer/
├── 3rdparty/
│   └── ffmpeg/
│       ├── bin/      # 存放 .dll 文件
│       ├── include/  # 存放头文件 (.h)
│       └── lib/      # 存放库文件 (.lib 或 .a)
├── src/              # 源代码
└── CMakeLists.txt