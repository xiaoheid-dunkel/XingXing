# XingXing-ScriptCore

**XingXing 引擎的 C# 脚本核心库**

## 📚 文档

本目录包含 XingXing 引擎的 C# 脚本 API，用于编写游戏逻辑和实体行为。

### Hazel C# API 文档

XingXing 引擎使用 Hazel 作为 C# 脚本 API 的命名空间。完整的教程和参考文档位于：

- **📖 [完整教程](Source/Hazel/README.md)** - 教科书风格的详细讲解
  - API 设计理念与架构
  - 实体组件系统 (ECS)
  - 向量数学库
  - 输入系统
  - 渲染组件
  - 物理系统
  - 内部调用机制
  - 完整代码示例

- **⚡ [快速参考](Source/Hazel/API-QuickReference.md)** - 快速查询手册
  - API 速查表
  - 代码模板
  - 性能优化建议
  - 常见错误避免

## 🎮 快速开始

### 创建你的第一个实体脚本

```csharp
using Hazel;

namespace Game
{
    public class Player : Entity
    {
        private SpriteRendererComponent sprite;
        private float speed = 10.0f;
        
        // 初始化
        public void OnCreate()
        {
            sprite = GetComponent<SpriteRendererComponent>();
            sprite.Color = new Vector4(0.0f, 1.0f, 0.0f, 1.0f);  // 绿色
        }
        
        // 每帧更新
        public void OnUpdate(float deltaTime)
        {
            // 移动控制
            Vector3 pos = Translation;
            
            if (Input.IsKeyDown(KeyCode.W))
                pos.Y += speed * deltaTime;
            if (Input.IsKeyDown(KeyCode.S))
                pos.Y -= speed * deltaTime;
            if (Input.IsKeyDown(KeyCode.A))
                pos.X -= speed * deltaTime;
            if (Input.IsKeyDown(KeyCode.D))
                pos.X += speed * deltaTime;
            
            Translation = pos;
        }
    }
}
```

## 📁 目录结构

```
XingXing-ScriptCore/
├── README.md                    # 本文件
├── premake5.lua                 # 构建配置
└── Source/
    └── Hazel/                   # Hazel C# API
        ├── README.md            # 完整教程文档
        ├── API-QuickReference.md # 快速参考手册
        ├── Entity.cs            # 实体类
        ├── Components.cs        # 组件类
        ├── Input.cs             # 输入系统
        ├── KeyCode.cs           # 键码枚举
        ├── Vector2.cs           # 2D 向量
        ├── Vector3.cs           # 3D 向量
        ├── Vector4.cs           # 4D 向量
        ├── InternalCalls.cs     # 内部调用桥接
        └── Scene/               # 场景相关
            ├── Entity.cs
            └── Components.cs
```

## 🔗 相关链接

- [XingXing 引擎主仓库](../../)
- [引擎源码](../XingXing/)
- [编辑器源码](../XingXingEditor/)

## 📝 许可协议

本项目遵循主仓库的许可协议。详见根目录的 [LICENSE](../LICENSE) 文件。

**重要提示**：
- ❌ 禁止商用
- ❌ 严禁严重复制
- ✅ 仅供学习、研究和模组开发使用
