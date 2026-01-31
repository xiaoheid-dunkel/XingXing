# 🌟 XingXing (行星引擎)

XingXing 是一个专为独立游戏开发的 **独立引擎**。本项目的初衷是为特定的独立游戏提供底层动力，并为未来的游戏模组 (Mod) 开发者提供一个透明、易用的参考框架。

---

## 🛠 引擎溯源
本引擎的最初底层代码源自于 [Hazel2D](https://github.com/TheCherno/Hazel)（由 TheCherno 开发）。在 Hazel2D 优秀的架构基础上，我们针对特定的游戏需求进行了深度的定制和功能扩展。

## 📜 开源目的与许可协议

### 1. 为什么开源？
我们选择开放源码，主要是为了**未来的模组 (Mod) 开发者**。通过阅读源码，开发者可以更好地理解游戏的底层运行逻辑，从而创作出更丰富、更具创意的游戏模组。

### 2. 使用限制 (重要)
虽然源码是开放的，但为了保护原创内容，本引擎遵循以下限制：
* **禁止商用**：严禁将本引擎的代码或其派生版本用于任何形式的商业盈利行为。
* **严禁严重复制**：本项目仅供学习、研究和模组开发使用。严禁直接“换壳”包装成自己的引擎或产品发布。
* **尊重原创**：如果你在非商用的个人学习项目中引用了部分代码，请务必保留对原始项目 (Hazel2D) 和本项目的致谢。

## 🚀 核心功能
* **高性能 2D 批处理渲染器** (基于 Renderer2D)
* **3D 方块渲染系统** (基于 Renderer3D) - 🆕
* **透视相机与第一人称控制** - 🆕
* **MSDF 字体渲染系统**
* **纹理与精灵图管理**
* **正交摄像机系统**

### 🎮 3D 功能预览
XingXing 现已支持 3D 渲染！特别针对类似《Minecraft》的方块沙盒游戏进行了优化。

**新增特性：**
- 透视相机（PerspectiveCamera）
- 3D 立方体批量渲染（Renderer3D）
- 简单的方向光照系统
- 可运行的 Voxel 世界示例

**快速开始 3D 开发：**
1. 查看 [3D 升级计划](docs/3D_UPGRADE_PLAN.md)
2. 阅读 [3D 集成指南](docs/3D_INTEGRATION_GUIDE.md)
3. 运行 Sandbox 项目查看 VoxelWorldLayer 示例

---

## 🏗 如何构建

1.  克隆本仓库：
    ```bash
    git clone [https://github.com/xiaoheid-dunkel/XingXing.git](https://github.com/xiaoheid-dunkel/XingXing.git)
    ```
2.  运行根目录下的 `GenerateProjectFiles.bat` 生成 Visual Studio 解决方案。
3.  使用 Visual Studio 2022 或更高版本打开 `XingXing.sln` 并进行编译。

---
