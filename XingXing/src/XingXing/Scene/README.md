# Scene 模块 - 场景管理系统

## 📖 文档导航

欢迎来到 XingXing 引擎的 Scene 模块！本模块是引擎的核心组成部分，负责管理游戏世界中的场景、实体和组件。

### 主要文档

- **[Scene模块详解.md](./Scene模块详解.md)** - 📚 完整的教科书式详细文档
  - 包含模块概述、核心架构、详细API说明
  - 提供使用指南和最佳实践
  - 适合深入学习和参考

## 🗂️ 模块文件说明

### 核心文件

| 文件 | 说明 |
|-----|------|
| `Scene.h` / `Scene.cpp` | 场景管理核心类，负责实体生命周期、物理、渲染协调 |
| `Entity.h` / `Entity.cpp` | 实体包装类，提供面向对象的组件操作接口 |
| `Components.h` | 所有组件定义（变换、渲染、物理、脚本等） |
| `SceneCamera.h` / `SceneCamera.cpp` | 场景相机系统，支持正交和透视投影 |
| `SceneSerializer.h` / `SceneSerializer.cpp` | 场景序列化/反序列化，支持 YAML 格式 |
| `ScriptableEntity.h` | C++ 脚本基类，用于编写自定义游戏逻辑 |

## 🎯 快速开始

### 1. 创建场景

```cpp
// 创建新场景
Ref<Scene> scene = CreateRef<Scene>();

// 开始运行时
scene->OnRuntimeStart();
```

### 2. 创建实体

```cpp
// 创建玩家实体
Entity player = scene->CreateEntity("Player");

// 设置位置
auto& transform = player.GetComponent<TransformComponent>();
transform.Translation = { 0.0f, 0.0f, 0.0f };

// 添加精灵渲染
auto& sprite = player.AddComponent<SpriteRendererComponent>();
sprite.Color = { 1.0f, 0.0f, 0.0f, 1.0f };  // 红色
```

### 3. 添加物理

```cpp
// 添加刚体
auto& rb2d = player.AddComponent<Rigidbody2DComponent>();
rb2d.Type = Rigidbody2DComponent::BodyType::Dynamic;

// 添加碰撞器
auto& collider = player.AddComponent<BoxCollider2DComponent>();
collider.Size = { 0.5f, 0.5f };
```

### 4. 更新场景

```cpp
// 在游戏循环中
void OnUpdate(Timestep ts)
{
    scene->OnUpdateRuntime(ts);  // 更新脚本、物理、渲染
}
```

## 🏗️ 架构概览

Scene 模块采用 **ECS (Entity-Component-System)** 架构：

```
┌─────────────────────────────────────────────┐
│              Scene (场景容器)                │
├─────────────────────────────────────────────┤
│  • entt::registry (ECS 注册表)              │
│  • 物理世界 (Box2D)                          │
│  • 实体映射表 (UUID → Entity)                │
└────────────┬────────────────────────────────┘
             │
    ┌────────┴────────┐
    │                 │
┌───▼────┐      ┌────▼──────┐
│ Entity │      │ Component │
│ (实体) │      │  (组件)   │
└───┬────┘      └────┬──────┘
    │                │
    │  • IDComponent (唯一标识)
    │  • TagComponent (名称)
    │  • TransformComponent (变换)
    │  • SpriteRendererComponent (精灵渲染)
    │  • CameraComponent (相机)
    │  • Rigidbody2DComponent (刚体)
    │  • BoxCollider2DComponent (碰撞器)
    │  • ScriptComponent (脚本)
    │  • ...更多组件
    │
```

## 📚 核心概念

### Entity (实体)

实体是游戏世界中的基本对象，本质上是一个唯一标识符。实体本身不包含数据，而是作为组件的容器。

### Component (组件)

组件是存储数据的容器，定义了实体的属性：
- **TransformComponent**: 位置、旋转、缩放
- **SpriteRendererComponent**: 精灵纹理、颜色
- **Rigidbody2DComponent**: 物理属性
- **ScriptComponent**: 游戏逻辑

### System (系统)

系统处理拥有特定组件集合的实体的逻辑：
- **渲染系统**: 处理所有具有 Transform + Sprite 的实体
- **物理系统**: 处理所有具有 Transform + Rigidbody 的实体
- **脚本系统**: 处理所有具有 Script 的实体

## 🔧 常用操作

### 查找实体

```cpp
// 按名称查找
Entity player = scene->FindEntityByName("Player");

// 按 UUID 查找
Entity entity = scene->GetEntityByUUID(uuid);
```

### 组件操作

```cpp
// 检查组件
if (entity.HasComponent<SpriteRendererComponent>())
{
    // 获取组件
    auto& sprite = entity.GetComponent<SpriteRendererComponent>();
    sprite.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
}

// 移除组件
entity.RemoveComponent<Rigidbody2DComponent>();
```

### 遍历实体

```cpp
// 获取所有具有特定组件的实体
auto view = scene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
for (auto entity : view)
{
    auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);
    // 处理逻辑...
}
```

## 🎮 自定义脚本

### C++ 脚本示例

```cpp
class PlayerController : public ScriptableEntity
{
protected:
    void OnCreate() override
    {
        // 初始化
    }
    
    void OnUpdate(Timestep ts) override
    {
        auto& transform = GetComponent<TransformComponent>();
        float speed = 5.0f;
        
        if (Input::IsKeyPressed(Key::W))
            transform.Translation.y += speed * ts;
        if (Input::IsKeyPressed(Key::S))
            transform.Translation.y -= speed * ts;
        if (Input::IsKeyPressed(Key::A))
            transform.Translation.x -= speed * ts;
        if (Input::IsKeyPressed(Key::D))
            transform.Translation.x += speed * ts;
    }
};

// 绑定到实体
auto& nsc = player.AddComponent<NativeScriptComponent>();
nsc.Bind<PlayerController>();
```

## 💾 场景序列化

### 保存场景

```cpp
SceneSerializer serializer(scene);
serializer.Serialize("assets/scenes/MyScene.hazel");
```

### 加载场景

```cpp
Ref<Scene> scene = CreateRef<Scene>();
SceneSerializer serializer(scene);
serializer.Deserialize("assets/scenes/MyScene.hazel");
```

## 📖 学习资源

1. **[Scene模块详解.md](./Scene模块详解.md)** - 完整的教科书式文档
2. **源代码注释** - 查看头文件中的详细注释
3. **示例项目** - Sandbox 和 XingXingnut 项目中的使用示例

## 🤝 贡献指南

如果你发现文档有任何错误或需要改进的地方，欢迎：
1. 提交 Issue 报告问题
2. 提交 Pull Request 修复文档
3. 在社区讨论区分享你的使用经验

## 📄 许可证

本文档遵循与 XingXing 引擎相同的许可协议。请参阅根目录的 LICENSE 文件。

---

**注意**: 本模块是 XingXing 引擎的核心部分。如果你是 Mod 开发者，强烈建议先阅读完整的 [Scene模块详解.md](./Scene模块详解.md) 文档。

💡 **提示**: 按 `Ctrl+F` 在文档中搜索你需要的功能！
