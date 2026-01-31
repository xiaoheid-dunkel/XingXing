# Scene 模块详解 - XingXing 引擎场景管理系统

## 📚 目录

1. [模块概述](#模块概述)
2. [核心架构](#核心架构)
3. [核心类详解](#核心类详解)
4. [使用指南](#使用指南)
5. [最佳实践](#最佳实践)

---

## 模块概述

Scene 模块是 XingXing 引擎的核心组成部分，负责管理游戏世界中的所有场景、实体和组件。它基于 **ECS (Entity-Component-System)** 架构设计，提供了高性能、灵活的游戏对象管理方案。

### 什么是 ECS？

**ECS** 是一种现代游戏引擎广泛采用的架构模式：

- **Entity（实体）**: 游戏世界中的基本对象，本质上是一个唯一标识符
- **Component（组件）**: 存储数据的容器，如位置、渲染、物理属性等
- **System（系统）**: 处理拥有特定组件集合的实体的逻辑

### 核心优势

1. **高性能**: 数据密集型存储，缓存友好
2. **灵活性**: 通过组合组件创建不同类型的游戏对象
3. **可维护性**: 数据与逻辑分离，代码结构清晰
4. **可扩展性**: 易于添加新的组件和系统

---

## 核心架构

### 模块文件结构

```
Scene/
├── Scene.h / Scene.cpp              # 场景管理核心类
├── Entity.h / Entity.cpp            # 实体包装类
├── Components.h                      # 所有组件定义
├── SceneCamera.h / SceneCamera.cpp  # 场景相机系统
├── SceneSerializer.h / .cpp         # 场景序列化
└── ScriptableEntity.h               # C++ 脚本基类
```

### 类关系图

```
Scene (场景容器)
  │
  ├── entt::registry (ECS 注册表，底层使用 EnTT 库)
  │     └── 管理所有实体和组件的存储
  │
  ├── Entity (实体包装器)
  │     ├── 提供友好的 API 接口
  │     └── 操作 registry 中的数据
  │
  ├── Components (各种组件)
  │     ├── IDComponent (唯一标识)
  │     ├── TagComponent (名称标签)
  │     ├── TransformComponent (变换)
  │     ├── SpriteRendererComponent (精灵渲染)
  │     ├── CameraComponent (相机)
  │     ├── ScriptComponent (C# 脚本)
  │     ├── NativeScriptComponent (C++ 脚本)
  │     ├── Rigidbody2DComponent (2D 刚体)
  │     ├── BoxCollider2DComponent (盒碰撞器)
  │     ├── CircleCollider2DComponent (圆碰撞器)
  │     └── TextComponent (文本渲染)
  │
  └── SceneSerializer (场景序列化器)
        └── 负责场景的保存和加载
```

---

## 核心类详解

### 1. Scene 类 - 场景管理核心

#### 类的职责

`Scene` 类是场景管理的中枢，负责：
- 实体的创建、销毁和查询
- 场景的生命周期管理（运行时、模拟、编辑器模式）
- 物理系统集成
- 渲染管道协调
- 脚本系统管理

#### 关键成员变量

```cpp
// 核心数据结构
entt::registry m_Registry;  // EnTT ECS 注册表，存储所有实体和组件

// 场景状态
uint32_t m_ViewportWidth, m_ViewportHeight;  // 视口尺寸
bool m_IsRunning;   // 是否处于运行时状态
bool m_IsPaused;    // 是否暂停
int m_StepFrames;   // 单步执行的帧数

// 物理世界
b2World* m_PhysicsWorld;  // Box2D 物理世界实例

// UUID 到实体的映射
std::unordered_map<UUID, entt::entity> m_EntityMap;
```

#### 核心方法详解

##### 1.1 实体管理

```cpp
// 创建实体
Entity CreateEntity(const std::string& name = std::string());
Entity CreateEntityWithUUID(UUID uuid, const std::string& name);
```

**功能说明**:
- 在 ECS 注册表中创建新实体
- 自动添加必需组件：IDComponent、TagComponent、TransformComponent
- 返回实体的包装对象供外部使用

**实现细节**:
```cpp
Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
{
    // 1. 在注册表中创建原始实体句柄
    Entity entity = { m_Registry.create(), this };
    
    // 2. 添加必需的基础组件
    entity.AddComponent<IDComponent>(uuid);           // 唯一标识
    entity.AddComponent<TransformComponent>();        // 变换信息
    auto& tag = entity.AddComponent<TagComponent>(); // 名称标签
    tag.Tag = name.empty() ? "Entity" : name;
    
    // 3. 维护 UUID 映射表，用于快速查找
    m_EntityMap[uuid] = entity;
    
    return entity;
}
```

```cpp
// 销毁实体
void DestroyEntity(Entity entity);
```

**功能说明**:
- 从场景中移除实体及其所有组件
- 自动清理 UUID 映射

```cpp
// 查找实体
Entity FindEntityByName(std::string_view name);  // 按名称查找
Entity GetEntityByUUID(UUID uuid);               // 按 UUID 查找
```

##### 1.2 生命周期管理

```cpp
// 运行时生命周期
void OnRuntimeStart();   // 开始运行时（游戏开始）
void OnRuntimeStop();    // 停止运行时（游戏结束）

// 模拟生命周期
void OnSimulationStart(); // 开始物理模拟（编辑器中测试物理）
void OnSimulationStop();  // 停止物理模拟

// 更新循环
void OnUpdateRuntime(Timestep ts);                      // 运行时更新
void OnUpdateSimulation(Timestep ts, EditorCamera& camera); // 模拟更新
void OnUpdateEditor(Timestep ts, EditorCamera& camera);     // 编辑器更新
```

**生命周期状态图**:

```
编辑器模式 (OnUpdateEditor)
    ↓ [Play 按钮]
运行时模式 (OnRuntimeStart)
    ↓
运行中 (OnUpdateRuntime) ← → 暂停状态
    ↓ [Stop 按钮]
停止 (OnRuntimeStop)
    ↓
返回编辑器模式

模拟模式 (用于物理测试，不执行脚本)
    OnSimulationStart → OnUpdateSimulation → OnSimulationStop
```

##### 1.3 运行时更新详解

`OnUpdateRuntime` 是游戏运行时的核心更新函数，执行顺序如下：

```cpp
void Scene::OnUpdateRuntime(Timestep ts)
{
    // ========== 第一阶段：脚本更新 ==========
    if (!m_IsPaused || m_StepFrames-- > 0)
    {
        // 1. 更新 C# 脚本实体
        auto view = m_Registry.view<ScriptComponent>();
        for (auto e : view)
        {
            Entity entity = { e, this };
            ScriptEngine::OnUpdateEntity(entity, ts);  // 调用 C# 的 OnUpdate
        }
        
        // 2. 更新 C++ 原生脚本
        m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
        {
            // 懒加载：首次使用时实例化脚本
            if (!nsc.Instance)
            {
                nsc.Instance = nsc.InstantiateScript();
                nsc.Instance->m_Entity = Entity{ entity, this };
                nsc.Instance->OnCreate();  // 调用创建回调
            }
            nsc.Instance->OnUpdate(ts);  // 调用更新回调
        });
        
        // ========== 第二阶段：物理模拟 ==========
        // Box2D 物理步进
        const int32_t velocityIterations = 6;
        const int32_t positionIterations = 2;
        m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);
        
        // 同步物理结果到 Transform 组件
        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = { e, this };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            
            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            const auto& position = body->GetPosition();
            transform.Translation.x = position.x;  // 更新位置
            transform.Translation.y = position.y;
            transform.Rotation.z = body->GetAngle(); // 更新旋转
        }
    }
    
    // ========== 第三阶段：渲染 ==========
    // 1. 查找主相机
    Camera* mainCamera = nullptr;
    glm::mat4 cameraTransform;
    {
        auto view = m_Registry.view<TransformComponent, CameraComponent>();
        for (auto entity : view)
        {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
            if (camera.Primary)  // 找到标记为主相机的实体
            {
                mainCamera = &camera.Camera;
                cameraTransform = transform.GetTransform();
                break;
            }
        }
    }
    
    // 2. 如果存在主相机，执行渲染
    if (mainCamera)
    {
        Renderer2D::BeginScene(*mainCamera, cameraTransform);
        
        // 渲染精灵
        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
        }
        
        // 渲染圆形
        auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();
        for (auto entity : circleView)
        {
            auto [transform, circle] = circleView.get<TransformComponent, CircleRendererComponent>(entity);
            Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, 
                                   circle.Thickness, circle.Fade, (int)entity);
        }
        
        // 渲染文本
        auto textView = m_Registry.view<TransformComponent, TextComponent>();
        for (auto entity : textView)
        {
            auto [transform, text] = textView.get<TransformComponent, TextComponent>(entity);
            Renderer2D::DrawString(text.TextString, transform.GetTransform(), text, (int)entity);
        }
        
        Renderer2D::EndScene();
    }
}
```

**更新流程图**:

```
OnUpdateRuntime(Timestep ts)
    │
    ├─→ 检查暂停状态 (m_IsPaused)
    │
    ├─→ [未暂停] 执行脚本更新
    │   ├─→ C# 脚本: ScriptEngine::OnUpdateEntity()
    │   └─→ C++ 脚本: NativeScriptComponent::OnUpdate()
    │
    ├─→ [未暂停] 执行物理模拟
    │   ├─→ Box2D 步进: m_PhysicsWorld->Step()
    │   └─→ 同步变换: 从 b2Body 更新 TransformComponent
    │
    └─→ 执行渲染
        ├─→ 查找主相机
        ├─→ BeginScene()
        ├─→ 绘制精灵、圆形、文本
        └─→ EndScene()
```

##### 1.4 物理系统管理

```cpp
void Scene::OnPhysics2DStart()
{
    // 1. 创建 Box2D 物理世界，设置重力
    m_PhysicsWorld = new b2World({ 0.0f, -9.8f });  // 重力加速度 -9.8 m/s²
    
    // 2. 遍历所有具有刚体组件的实体
    auto view = m_Registry.view<Rigidbody2DComponent>();
    for (auto e : view)
    {
        Entity entity = { e, this };
        auto& transform = entity.GetComponent<TransformComponent>();
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        
        // 3. 创建 Box2D 刚体
        b2BodyDef bodyDef;
        bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
        bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
        bodyDef.angle = transform.Rotation.z;
        
        b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);
        rb2d.RuntimeBody = body;  // 存储运行时指针
        
        // 4. 添加碰撞器（如果存在）
        // 盒碰撞器
        if (entity.HasComponent<BoxCollider2DComponent>())
        {
            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
            
            b2PolygonShape boxShape;
            boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, 
                             bc2d.Size.y * transform.Scale.y,
                             b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
            
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &boxShape;
            fixtureDef.density = bc2d.Density;
            fixtureDef.friction = bc2d.Friction;
            fixtureDef.restitution = bc2d.Restitution;
            body->CreateFixture(&fixtureDef);
        }
        
        // 圆形碰撞器
        if (entity.HasComponent<CircleCollider2DComponent>())
        {
            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
            
            b2CircleShape circleShape;
            circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
            circleShape.m_radius = transform.Scale.x * cc2d.Radius;
            
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circleShape;
            fixtureDef.density = cc2d.Density;
            fixtureDef.friction = cc2d.Friction;
            fixtureDef.restitution = cc2d.Restitution;
            body->CreateFixture(&fixtureDef);
        }
    }
}

void Scene::OnPhysics2DStop()
{
    delete m_PhysicsWorld;  // 销毁物理世界，自动清理所有物理对象
    m_PhysicsWorld = nullptr;
}
```

##### 1.5 场景复制

```cpp
Ref<Scene> Scene::Copy(Ref<Scene> other)
```

**功能**: 深度复制整个场景，包括所有实体和组件

**应用场景**:
- 场景备份
- 运行时场景切换
- 编辑器的撤销/重做功能

---

### 2. Entity 类 - 实体包装器

#### 设计理念

`Entity` 类是对 `entt::entity` (一个整数句柄) 的面向对象封装，提供：
- 类型安全的组件操作
- 便捷的 API 接口
- 自动的断言检查

#### 核心成员

```cpp
class Entity
{
private:
    entt::entity m_EntityHandle;  // EnTT 原始句柄
    Scene* m_Scene;               // 所属场景的指针
};
```

**为什么需要场景指针？**
- 所有组件操作都需要通过场景的注册表 (`m_Scene->m_Registry`)
- Entity 本身不存储数据，只是一个访问接口

#### 组件操作 API

##### 2.1 添加组件

```cpp
template<typename T, typename... Args>
T& AddComponent(Args&&... args)
{
    // 1. 断言检查：不能添加已存在的组件
    HZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
    
    // 2. 在注册表中添加组件（完美转发构造参数）
    T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
    
    // 3. 触发组件添加回调（用于初始化工作，如相机设置视口）
    m_Scene->OnComponentAdded<T>(*this, component);
    
    return component;
}
```

**使用示例**:
```cpp
Entity entity = scene->CreateEntity("Player");

// 添加精灵渲染组件
auto& sprite = entity.AddComponent<SpriteRendererComponent>();
sprite.Color = { 1.0f, 0.0f, 0.0f, 1.0f };  // 红色

// 添加刚体组件
auto& rb2d = entity.AddComponent<Rigidbody2DComponent>();
rb2d.Type = Rigidbody2DComponent::BodyType::Dynamic;
```

##### 2.2 获取组件

```cpp
template<typename T>
T& GetComponent()
{
    // 断言检查：组件必须存在
    HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
    return m_Scene->m_Registry.get<T>(m_EntityHandle);
}
```

##### 2.3 检查组件

```cpp
template<typename T>
bool HasComponent()
{
    return m_Scene->m_Registry.has<T>(m_EntityHandle);
}
```

##### 2.4 移除组件

```cpp
template<typename T>
void RemoveComponent()
{
    HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
    m_Scene->m_Registry.remove<T>(m_EntityHandle);
}
```

#### 便捷方法

```cpp
// 获取实体 UUID
UUID GetUUID() { return GetComponent<IDComponent>().ID; }

// 获取实体名称
const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

// 布尔转换：检查实体是否有效
operator bool() const { return m_EntityHandle != entt::null; }
```

**使用示例**:
```cpp
Entity entity = scene->FindEntityByName("Player");
if (entity)  // 检查实体是否有效
{
    std::cout << "Found entity: " << entity.GetName() << std::endl;
    UUID id = entity.GetUUID();
}
```

---

### 3. Components - 组件系统

组件是 ECS 架构中的数据容器，只存储数据，不包含逻辑。

#### 3.1 基础组件

##### IDComponent - 唯一标识组件

```cpp
struct IDComponent
{
    UUID ID;  // 128位唯一标识符
};
```

**用途**:
- 跨场景的实体引用
- 场景序列化
- 网络同步

##### TagComponent - 名称标签组件

```cpp
struct TagComponent
{
    std::string Tag;  // 实体的可读名称
};
```

**用途**:
- 编辑器显示
- 按名称查找实体
- 调试信息输出

##### TransformComponent - 变换组件

```cpp
struct TransformComponent
{
    glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };  // 位置
    glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };     // 旋转（欧拉角）
    glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };        // 缩放
    
    // 计算变换矩阵
    glm::mat4 GetTransform() const
    {
        glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));  // 转为四元数避免万向锁
        
        return glm::translate(glm::mat4(1.0f), Translation)
             * rotation
             * glm::scale(glm::mat4(1.0f), Scale);
    }
};
```

**关键点**:
- 旋转使用欧拉角存储，但渲染时转换为四元数
- `GetTransform()` 返回最终的 4x4 变换矩阵用于渲染

#### 3.2 渲染组件

##### SpriteRendererComponent - 精灵渲染组件

```cpp
struct SpriteRendererComponent
{
    glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };  // 颜色/着色
    Ref<Texture2D> Texture;                      // 纹理（可选）
    float TilingFactor = 1.0f;                   // 纹理平铺因子
};
```

**用途**:
- 2D 精灵渲染
- 纯色矩形
- 纹理贴图

**示例**:
```cpp
auto& sprite = entity.AddComponent<SpriteRendererComponent>();
sprite.Color = { 1.0f, 0.5f, 0.2f, 1.0f };  // 橙色
sprite.Texture = Texture2D::Create("assets/player.png");
sprite.TilingFactor = 2.0f;  // 纹理重复2次
```

##### CircleRendererComponent - 圆形渲染组件

```cpp
struct CircleRendererComponent
{
    glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };  // 颜色
    float Thickness = 1.0f;                      // 边框厚度 (0-1)
    float Fade = 0.005f;                         // 边缘渐变程度
};
```

**用途**:
- 圆形/环形渲染
- UI 元素
- 调试可视化

##### TextComponent - 文本渲染组件

```cpp
struct TextComponent
{
    std::string TextString;            // 显示的文本
    Ref<Font> FontAsset;              // 字体资源
    glm::vec4 Color{ 1.0f };          // 文本颜色
    float Kerning = 0.0f;             // 字距调整
    float LineSpacing = 0.0f;         // 行距
};
```

#### 3.3 相机组件

##### CameraComponent

```cpp
struct CameraComponent
{
    SceneCamera Camera;                // 相机对象
    bool Primary = true;               // 是否为主相机
    bool FixedAspectRatio = false;     // 是否固定纵横比
};
```

**关键点**:
- 一个场景可以有多个相机，但只有一个主相机用于渲染
- `FixedAspectRatio` 控制窗口大小改变时是否自动调整视口

#### 3.4 脚本组件

##### ScriptComponent - C# 脚本组件

```cpp
struct ScriptComponent
{
    std::string ClassName;  // C# 脚本类名
};
```

##### NativeScriptComponent - C++ 脚本组件

```cpp
struct NativeScriptComponent
{
    ScriptableEntity* Instance = nullptr;  // 脚本实例
    
    ScriptableEntity*(*InstantiateScript)();       // 实例化函数指针
    void (*DestroyScript)(NativeScriptComponent*); // 销毁函数指针
    
    template<typename T>
    void Bind()
    {
        InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
        DestroyScript = [](NativeScriptComponent* nsc) { 
            delete nsc->Instance; 
            nsc->Instance = nullptr; 
        };
    }
};
```

**使用示例**:
```cpp
// 定义自定义脚本类
class PlayerController : public ScriptableEntity
{
protected:
    void OnCreate() override
    {
        // 初始化逻辑
    }
    
    void OnUpdate(Timestep ts) override
    {
        auto& transform = GetComponent<TransformComponent>();
        float speed = 5.0f;
        
        if (Input::IsKeyPressed(Key::W))
            transform.Translation.y += speed * ts;
        if (Input::IsKeyPressed(Key::S))
            transform.Translation.y -= speed * ts;
    }
};

// 绑定到实体
auto& nsc = entity.AddComponent<NativeScriptComponent>();
nsc.Bind<PlayerController>();
```

#### 3.5 物理组件

##### Rigidbody2DComponent - 2D 刚体组件

```cpp
struct Rigidbody2DComponent
{
    enum class BodyType { Static = 0, Dynamic, Kinematic };
    
    BodyType Type = BodyType::Static;  // 刚体类型
    bool FixedRotation = false;        // 是否固定旋转
    void* RuntimeBody = nullptr;       // Box2D 运行时指针
};
```

**刚体类型说明**:
- **Static**: 静态，不受力影响，如地面、墙壁
- **Dynamic**: 动态，受重力和碰撞影响，如玩家、敌人
- **Kinematic**: 运动学，可以移动但不受力影响，如移动平台

##### BoxCollider2DComponent - 盒碰撞器

```cpp
struct BoxCollider2DComponent
{
    glm::vec2 Offset = { 0.0f, 0.0f };  // 相对于实体的偏移
    glm::vec2 Size = { 0.5f, 0.5f };    // 碰撞盒大小
    
    // 物理材质属性
    float Density = 1.0f;                // 密度（影响质量）
    float Friction = 0.5f;               // 摩擦力 (0-1)
    float Restitution = 0.0f;            // 弹性系数 (0-1)
    float RestitutionThreshold = 0.5f;   // 弹性阈值
    
    void* RuntimeFixture = nullptr;      // Box2D 运行时指针
};
```

##### CircleCollider2DComponent - 圆形碰撞器

```cpp
struct CircleCollider2DComponent
{
    glm::vec2 Offset = { 0.0f, 0.0f };  // 相对于实体的偏移
    float Radius = 0.5f;                // 半径
    
    // 物理材质属性（同盒碰撞器）
    float Density = 1.0f;
    float Friction = 0.5f;
    float Restitution = 0.0f;
    float RestitutionThreshold = 0.5f;
    
    void* RuntimeFixture = nullptr;
};
```

---

### 4. SceneCamera 类 - 场景相机

#### 相机类型

`SceneCamera` 支持两种投影类型：

##### 正交投影 (Orthographic)

```cpp
void SetOrthographic(float size, float nearClip, float farClip);
```

**特点**:
- 无透视效果，远近物体大小相同
- 常用于 2D 游戏、UI、CAD 软件
- `size` 参数控制相机的视野高度

##### 透视投影 (Perspective)

```cpp
void SetPerspective(float verticalFOV, float nearClip, float farClip);
```

**特点**:
- 有透视效果，远小近大
- 常用于 3D 游戏
- `verticalFOV` 是垂直视场角（弧度）

#### 相机参数

```cpp
// 透视相机参数
float m_PerspectiveFOV = glm::radians(45.0f);  // 视场角 45°
float m_PerspectiveNear = 0.01f;               // 近裁剪面
float m_PerspectiveFar = 1000.0f;              // 远裁剪面

// 正交相机参数
float m_OrthographicSize = 10.0f;              // 视野高度
float m_OrthographicNear = -1.0f;              // 近裁剪面
float m_OrthographicFar = 1.0f;                // 远裁剪面
```

#### 视口调整

```cpp
void SetViewportSize(uint32_t width, uint32_t height);
```

**功能**:
- 当窗口大小改变时更新相机的纵横比
- 自动重新计算投影矩阵

---

### 5. SceneSerializer 类 - 场景序列化

#### 职责

负责场景的保存和加载，支持：
- 编辑器场景文件 (.hazel)
- 运行时场景数据

#### 核心方法

```cpp
class SceneSerializer
{
public:
    SceneSerializer(const Ref<Scene>& scene);
    
    // 序列化场景到文件
    void Serialize(const std::string& filepath);
    
    // 从文件反序列化场景
    bool Deserialize(const std::string& filepath);
    
    // 运行时序列化（二进制格式，更快）
    void SerializeRuntime(const std::string& filepath);
    bool DeserializeRuntime(const std::string& filepath);
};
```

#### 文件格式

场景文件使用 YAML 格式，可读性强，便于调试和手动编辑。

**示例场景文件结构**:
```yaml
Scene: Untitled
Entities:
  - Entity: 12345678901234567890  # UUID
    TagComponent:
      Tag: Camera
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    CameraComponent:
      Camera:
        ProjectionType: 1  # Orthographic
        OrthographicSize: 10
        OrthographicNear: -1
        OrthographicFar: 1
      Primary: true
      FixedAspectRatio: false
      
  - Entity: 98765432109876543210
    TagComponent:
      Tag: Player
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    SpriteRendererComponent:
      Color: [1, 1, 1, 1]
    Rigidbody2DComponent:
      BodyType: 1  # Dynamic
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      Size: [0.5, 0.5]
      Density: 1
      Friction: 0.5
```

---

### 6. ScriptableEntity 类 - C++ 脚本基类

#### 设计用途

`ScriptableEntity` 是所有 C++ 游戏逻辑脚本的基类，提供：
- 访问实体组件的便捷接口
- 生命周期回调钩子

#### 类定义

```cpp
class ScriptableEntity
{
public:
    virtual ~ScriptableEntity() {}
    
    // 便捷方法：获取组件
    template<typename T>
    T& GetComponent()
    {
        return m_Entity.GetComponent<T>();
    }
    
protected:
    // 生命周期钩子（由子类重写）
    virtual void OnCreate() {}     // 脚本创建时调用
    virtual void OnDestroy() {}    // 脚本销毁时调用
    virtual void OnUpdate(Timestep ts) {}  // 每帧调用
    
private:
    Entity m_Entity;  // 脚本所属的实体
    friend class Scene;
};
```

#### 使用模式

```cpp
// 1. 继承 ScriptableEntity
class EnemyAI : public ScriptableEntity
{
private:
    float m_Speed = 2.0f;
    float m_DetectionRange = 5.0f;
    
protected:
    void OnCreate() override
    {
        // 初始化 AI 状态
        HZ_CORE_INFO("Enemy AI created!");
    }
    
    void OnUpdate(Timestep ts) override
    {
        // 实现 AI 逻辑
        auto& transform = GetComponent<TransformComponent>();
        
        // 简单的巡逻逻辑
        transform.Translation.x += m_Speed * ts;
        
        if (abs(transform.Translation.x) > 10.0f)
            m_Speed = -m_Speed;  // 到达边界反向
    }
    
    void OnDestroy() override
    {
        // 清理资源
        HZ_CORE_INFO("Enemy AI destroyed!");
    }
};

// 2. 绑定到实体
Entity enemy = scene->CreateEntity("Enemy");
enemy.AddComponent<SpriteRendererComponent>();
auto& nsc = enemy.AddComponent<NativeScriptComponent>();
nsc.Bind<EnemyAI>();
```

---

## 使用指南

### 1. 创建和管理场景

#### 1.1 创建场景

```cpp
// 创建新场景
Ref<Scene> scene = CreateRef<Scene>();
```

#### 1.2 场景生命周期

```cpp
// 编辑器模式
void OnEditorUpdate(Timestep ts)
{
    scene->OnUpdateEditor(ts, editorCamera);
}

// 运行游戏
void OnPlay()
{
    scene->OnRuntimeStart();
}

void OnGameUpdate(Timestep ts)
{
    scene->OnUpdateRuntime(ts);
}

void OnStop()
{
    scene->OnRuntimeStop();
}
```

### 2. 实体操作

#### 2.1 创建实体

```cpp
// 创建空实体
Entity entity = scene->CreateEntity();

// 创建带名称的实体
Entity player = scene->CreateEntity("Player");
```

#### 2.2 添加组件

```cpp
// 添加变换组件（自动添加，无需手动）
auto& transform = player.GetComponent<TransformComponent>();
transform.Translation = { 0.0f, 0.0f, 0.0f };

// 添加精灵渲染组件
auto& sprite = player.AddComponent<SpriteRendererComponent>();
sprite.Color = { 1.0f, 0.0f, 0.0f, 1.0f };

// 添加物理组件
auto& rb2d = player.AddComponent<Rigidbody2DComponent>();
rb2d.Type = Rigidbody2DComponent::BodyType::Dynamic;

auto& collider = player.AddComponent<BoxCollider2DComponent>();
collider.Size = { 0.5f, 0.5f };
```

#### 2.3 查找实体

```cpp
// 按名称查找
Entity player = scene->FindEntityByName("Player");

// 按 UUID 查找
UUID playerUUID = ...; // 从某处获取
Entity player = scene->GetEntityByUUID(playerUUID);
```

#### 2.4 复制实体

```cpp
Entity original = scene->FindEntityByName("Enemy");
Entity clone = scene->DuplicateEntity(original);
```

#### 2.5 销毁实体

```cpp
scene->DestroyEntity(player);
```

### 3. 组件操作示例

#### 3.1 创建一个完整的玩家实体

```cpp
Entity CreatePlayer(Ref<Scene> scene)
{
    Entity player = scene->CreateEntity("Player");
    
    // 设置变换
    auto& transform = player.GetComponent<TransformComponent>();
    transform.Translation = { 0.0f, 0.0f, 0.0f };
    transform.Scale = { 1.0f, 1.0f, 1.0f };
    
    // 添加精灵渲染
    auto& sprite = player.AddComponent<SpriteRendererComponent>();
    sprite.Texture = Texture2D::Create("assets/player.png");
    
    // 添加物理
    auto& rb2d = player.AddComponent<Rigidbody2DComponent>();
    rb2d.Type = Rigidbody2DComponent::BodyType::Dynamic;
    rb2d.FixedRotation = true;
    
    auto& collider = player.AddComponent<BoxCollider2DComponent>();
    collider.Size = { 0.5f, 0.8f };
    collider.Density = 1.0f;
    collider.Friction = 0.3f;
    
    // 添加脚本
    auto& nsc = player.AddComponent<NativeScriptComponent>();
    nsc.Bind<PlayerController>();
    
    return player;
}
```

#### 3.2 创建相机实体

```cpp
Entity CreateCamera(Ref<Scene> scene)
{
    Entity camera = scene->CreateEntity("MainCamera");
    
    auto& cameraComp = camera.AddComponent<CameraComponent>();
    cameraComp.Primary = true;
    cameraComp.FixedAspectRatio = false;
    cameraComp.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
    cameraComp.Camera.SetOrthographicSize(10.0f);
    
    return camera;
}
```

### 4. 场景序列化

#### 4.1 保存场景

```cpp
SceneSerializer serializer(scene);
serializer.Serialize("assets/scenes/Level1.hazel");
```

#### 4.2 加载场景

```cpp
Ref<Scene> scene = CreateRef<Scene>();
SceneSerializer serializer(scene);

if (serializer.Deserialize("assets/scenes/Level1.hazel"))
{
    HZ_CORE_INFO("Scene loaded successfully!");
}
else
{
    HZ_CORE_ERROR("Failed to load scene!");
}
```

---

## 最佳实践

### 1. 实体组件设计原则

#### 1.1 组件应该只包含数据

❌ **错误示例**:
```cpp
struct BadComponent
{
    float speed;
    
    void Update(float deltaTime)  // 不要在组件中添加逻辑！
    {
        // ...
    }
};
```

✅ **正确示例**:
```cpp
struct GoodComponent
{
    float Speed;  // 只包含数据
    float Acceleration;
};

// 逻辑应该在 System 或 Script 中
class MovementSystem
{
    void Update(Scene* scene, Timestep ts)
    {
        auto view = scene->GetAllEntitiesWith<TransformComponent, GoodComponent>();
        for (auto entity : view)
        {
            // 在这里处理逻辑
        }
    }
};
```

#### 1.2 合理拆分组件

将功能正交的数据分离到不同组件中，而不是创建大型的"万能组件"。

❌ **错误示例**:
```cpp
struct GodComponent  // 过于庞大的组件
{
    // 渲染相关
    glm::vec4 Color;
    Ref<Texture2D> Texture;
    
    // 物理相关
    float Mass;
    float Friction;
    
    // AI 相关
    float AggroRange;
    float AttackDamage;
    
    // 音频相关
    Ref<AudioClip> WalkSound;
};
```

✅ **正确示例**:
```cpp
// 分离为多个专注的组件
struct SpriteRendererComponent { /* 渲染数据 */ };
struct Rigidbody2DComponent { /* 物理数据 */ };
struct AIComponent { /* AI 数据 */ };
struct AudioSourceComponent { /* 音频数据 */ };
```

### 2. 性能优化技巧

#### 2.1 使用组视图进行批量操作

```cpp
// 高效的批量更新
auto view = scene->GetAllEntitiesWith<TransformComponent, Rigidbody2DComponent>();
for (auto entity : view)
{
    auto [transform, rb2d] = view.get<TransformComponent, Rigidbody2DComponent>(entity);
    // 处理逻辑...
}
```

#### 2.2 避免频繁的 HasComponent 检查

❌ **低效**:
```cpp
for (auto entity : entities)
{
    if (entity.HasComponent<TransformComponent>())  // 每次迭代都检查
    {
        auto& transform = entity.GetComponent<TransformComponent>();
        // ...
    }
}
```

✅ **高效**:
```cpp
// 直接获取具有特定组件的实体视图
auto view = scene->GetAllEntitiesWith<TransformComponent>();
for (auto entity : view)
{
    auto& transform = view.get<TransformComponent>(entity);
    // ...
}
```

#### 2.3 缓存常用的实体引用

```cpp
class GameLayer
{
private:
    Entity m_Player;  // 缓存玩家实体引用
    
public:
    void OnAttach()
    {
        m_Player = m_Scene->FindEntityByName("Player");  // 只查找一次
    }
    
    void OnUpdate(Timestep ts)
    {
        // 直接使用缓存的引用
        if (m_Player)
        {
            auto& transform = m_Player.GetComponent<TransformComponent>();
            // ...
        }
    }
};
```

### 3. 脚本开发建议

#### 3.1 使用 NativeScriptComponent 进行原型开发

```cpp
class PrototypeScript : public ScriptableEntity
{
protected:
    void OnUpdate(Timestep ts) override
    {
        // 快速测试游戏逻辑
        auto& transform = GetComponent<TransformComponent>();
        
        if (Input::IsKeyPressed(Key::Space))
        {
            // 跳跃逻辑
        }
    }
};
```

#### 3.2 生产环境使用 C# 脚本

- C++ 脚本：引擎核心功能、性能关键路径
- C# 脚本：游戏逻辑、AI、UI 交互

### 4. 场景管理策略

#### 4.1 多场景管理

```cpp
class SceneManager
{
private:
    std::unordered_map<std::string, Ref<Scene>> m_Scenes;
    Ref<Scene> m_ActiveScene;
    
public:
    void LoadScene(const std::string& name, const std::string& filepath)
    {
        Ref<Scene> scene = CreateRef<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(filepath);
        m_Scenes[name] = scene;
    }
    
    void SetActiveScene(const std::string& name)
    {
        if (m_ActiveScene)
            m_ActiveScene->OnRuntimeStop();
        
        m_ActiveScene = m_Scenes[name];
        m_ActiveScene->OnRuntimeStart();
    }
};
```

#### 4.2 场景切换时的清理

```cpp
void TransitionToScene(Ref<Scene> newScene)
{
    // 1. 停止当前场景
    if (m_CurrentScene)
    {
        m_CurrentScene->OnRuntimeStop();
    }
    
    // 2. 切换场景
    m_CurrentScene = newScene;
    
    // 3. 启动新场景
    m_CurrentScene->OnRuntimeStart();
    m_CurrentScene->OnViewportResize(m_ViewportWidth, m_ViewportHeight);
}
```

### 5. 调试技巧

#### 5.1 实体信息输出

```cpp
void PrintEntityInfo(Entity entity)
{
    if (!entity)
    {
        HZ_CORE_WARN("Invalid entity!");
        return;
    }
    
    HZ_CORE_INFO("Entity: {0}", entity.GetName());
    HZ_CORE_INFO("  UUID: {0}", entity.GetUUID());
    
    if (entity.HasComponent<TransformComponent>())
    {
        auto& transform = entity.GetComponent<TransformComponent>();
        HZ_CORE_INFO("  Position: ({0}, {1}, {2})", 
            transform.Translation.x, 
            transform.Translation.y, 
            transform.Translation.z);
    }
}
```

#### 5.2 场景统计信息

```cpp
void PrintSceneStats(Ref<Scene> scene)
{
    int entityCount = 0;
    auto view = scene->GetAllEntitiesWith<IDComponent>();
    for (auto e : view)
        entityCount++;
    
    HZ_CORE_INFO("Scene Statistics:");
    HZ_CORE_INFO("  Total Entities: {0}", entityCount);
}
```

---

## 总结

XingXing 引擎的 Scene 模块采用现代化的 ECS 架构，提供了：

1. **高性能**: 数据导向设计，缓存友好
2. **灵活性**: 组件化设计，易于扩展
3. **易用性**: 友好的 API 接口
4. **完整性**: 物理、渲染、脚本系统集成

通过本文档，你应该能够：
- 理解 Scene 模块的整体架构
- 掌握实体和组件的使用方法
- 编写自定义的游戏逻辑脚本
- 进行场景的序列化和反序列化

如果你是 Mod 开发者，建议重点关注：
- `ScriptableEntity` - 用于编写自定义 C++ 游戏逻辑
- `Components.h` - 了解可用的组件类型
- `SceneSerializer` - 理解场景文件格式

祝你在 XingXing 引擎上的开发之旅愉快！🚀
