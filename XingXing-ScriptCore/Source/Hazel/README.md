# 📚 Hazel C# API 封装教程

> 本文档以教科书的方式，详细讲解 XingXing 引擎中 Hazel C# API 的设计原理、使用方法和最佳实践。

## 目录

1. [概述](#概述)
2. [架构设计](#架构设计)
3. [实体组件系统 (ECS)](#实体组件系统-ecs)
4. [向量数学库](#向量数学库)
5. [输入系统](#输入系统)
6. [渲染组件](#渲染组件)
7. [物理系统](#物理系统)
8. [内部调用机制](#内部调用机制)
9. [完整示例](#完整示例)

---

## 概述

### 什么是 Hazel C# API？

Hazel C# API 是 XingXing 引擎为脚本开发者提供的高级编程接口。它封装了底层 C++ 引擎的功能，使得开发者可以使用 C# 语言编写游戏逻辑，而无需直接处理复杂的底层细节。

### 设计理念

本 API 遵循以下核心设计理念：

1. **简洁性**：提供简单易用的接口，隐藏底层复杂性
2. **类型安全**：利用 C# 的强类型系统，在编译时发现错误
3. **性能优化**：通过内部调用机制实现高效的 C#/C++ 互操作
4. **模块化**：清晰的模块划分，便于理解和维护

---

## 架构设计

### 整体架构

```
┌─────────────────────────────────────┐
│        C# 脚本层 (游戏逻辑)          │
│  ├── Entity.cs                      │
│  ├── Components.cs                  │
│  ├── Input.cs                       │
│  └── Vector*.cs                     │
├─────────────────────────────────────┤
│      InternalCalls.cs (桥接层)      │  ← P/Invoke 调用
├─────────────────────────────────────┤
│     C++ 引擎层 (底层实现)            │
│  ├── ScriptEngine                   │
│  ├── Scene System                   │
│  └── Renderer                       │
└─────────────────────────────────────┘
```

### 关键技术点

1. **P/Invoke (Platform Invoke)**：C# 调用 C++ 的核心技术
2. **MethodImplAttribute**：标记内部调用方法
3. **泛型系统**：实现类型安全的组件获取
4. **值类型 vs 引用类型**：向量使用 struct，实体使用 class

---

## 实体组件系统 (ECS)

### 理论基础

**实体组件系统 (Entity-Component-System)** 是现代游戏引擎广泛采用的架构模式：

- **Entity（实体）**：游戏世界中的一个对象，仅持有唯一标识符
- **Component（组件）**：实体的数据和属性，如位置、颜色、物理属性
- **System（系统）**：处理具有特定组件的实体的逻辑

### Entity 类详解

#### 核心设计

```csharp
public class Entity
{
    public readonly ulong ID;  // 唯一标识符，64位无符号整数
    
    internal Entity(ulong id)  // 内部构造函数，防止外部随意创建
    {
        ID = id;
    }
}
```

**设计要点**：
1. `ID` 是只读的，确保实体标识符不可变
2. 构造函数是 `internal` 的，只能由引擎内部创建实体
3. 使用 `ulong` (64位) 可以支持海量实体而不重复

#### 主要功能

##### 1. 位置属性访问

```csharp
public Vector3 Translation
{
    get
    {
        InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 result);
        return result;
    }
    set
    {
        InternalCalls.TransformComponent_SetTranslation(ID, ref value);
    }
}
```

**知识点**：
- 使用属性 (Property) 而非字段，提供封装
- `out` 参数：C++ 将结果写入该参数
- `ref` 参数：传递引用，避免复制大型结构

##### 2. 组件查询

```csharp
public bool HasComponent<T>() where T : Component, new()
{
    Type componentType = typeof(T);
    return InternalCalls.Entity_HasComponent(ID, componentType);
}
```

**知识点**：
- **泛型约束**：`where T : Component, new()`
  - `T : Component`：T 必须继承自 Component
  - `new()`：T 必须有无参构造函数
- **反射**：使用 `typeof(T)` 获取类型信息

##### 3. 组件获取

```csharp
public T GetComponent<T>() where T : Component, new()
{
    if (!HasComponent<T>())
        return null;
    
    T component = new T() { Entity = this };
    return component;
}
```

**设计模式分析**：
- **防御性编程**：先检查组件是否存在
- **对象初始化器**：`new T() { Entity = this }`
- **延迟创建**：组件对象在需要时才创建

##### 4. 实体查找

```csharp
public Entity FindEntityByName(string name)
{
    ulong entityID = InternalCalls.Entity_FindEntityByName(name);
    if (entityID == 0)
        return null;
    
    return new Entity(entityID);
}
```

**约定**：
- `entityID == 0` 表示未找到实体
- 返回 `null` 而非抛出异常，遵循查询方法的常见约定

##### 5. 类型转换

```csharp
public T As<T>() where T : Entity, new()
{
    object instance = InternalCalls.GetScriptInstance(ID);
    return instance as T;
}
```

**用途**：
- 将通用 Entity 转换为用户自定义的实体类型
- 例如：`player.As<Player>()`

---

### Component 基类

```csharp
public abstract class Component
{
    public Entity Entity { get; internal set; }
}
```

**设计要点**：
1. **抽象类**：不能直接实例化，必须继承
2. **internal set**：只允许 Hazel 内部设置 Entity 引用
3. **双向关联**：Component 知道自己属于哪个 Entity

---

### 组件类型详解

#### 1. TransformComponent（变换组件）

**用途**：控制实体的位置、旋转和缩放

```csharp
public class TransformComponent : Component
{
    public Vector3 Translation
    {
        get
        {
            InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
            return translation;
        }
        set
        {
            InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
        }
    }
}
```

**使用示例**：
```csharp
var transform = entity.GetComponent<TransformComponent>();
transform.Translation = new Vector3(10.0f, 5.0f, 0.0f);
```

#### 2. SpriteRendererComponent（精灵渲染组件）

**用途**：渲染 2D 精灵图像

```csharp
public class SpriteRendererComponent : Component
{
    public Vector4 Color
    {
        get
        {
            InternalCalls.SpriteRendererComponent_GetColor(Entity.ID, out Vector4 color);
            return color;
        }
        set
        {
            InternalCalls.SpriteRendererComponent_SetColor(Entity.ID, ref value);
        }
    }
}
```

**颜色表示**：
- Vector4(R, G, B, A)
- 每个分量范围 [0.0, 1.0]
- 例如：红色 = `new Vector4(1.0f, 0.0f, 0.0f, 1.0f)`

#### 3. CircleRendererComponent（圆形渲染组件）

**用途**：渲染圆形或环形

```csharp
public class CircleRendererComponent : Component
{
    public Vector4 Color { get; set; }
    public float Thickness { get; set; }  // 厚度：0.0 = 实心，1.0 = 细环
    public float Fade { get; set; }       // 边缘渐变
}
```

**特性分析**：
- **表达式体属性**：使用 `=>` 简化 getter
- **参数用途**：
  - `Thickness = 0.0f`：实心圆
  - `Thickness = 1.0f`：细环
  - `Fade`：控制边缘平滑度

#### 4. Rigidbody2DComponent（2D 刚体组件）

**用途**：为实体添加物理行为

```csharp
public class Rigidbody2DComponent : Component
{
    public enum BodyType { Static = 0, Dynamic, Kinematic }
    
    public Vector2 LinearVelocity { get; }  // 只读：当前速度
    public BodyType Type { get; set; }      // 刚体类型
    
    public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
    {
        InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(
            Entity.ID, ref impulse, ref worldPosition, wake);
    }
    
    public void ApplyLinearImpulse(Vector2 impulse, bool wake)
    {
        InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(
            Entity.ID, ref impulse, wake);
    }
}
```

**刚体类型说明**：
- **Static（静态）**：不移动，如地面、墙壁
- **Dynamic（动态）**：受力和重力影响，如玩家、敌人
- **Kinematic（运动学）**：可移动但不受力影响，如移动平台

**方法重载**：
- 第一个重载：在指定世界坐标施加冲量
- 第二个重载：在刚体中心施加冲量

#### 5. TextComponent（文本组件）

**用途**：渲染文本

```csharp
public class TextComponent : Component
{
    public string Text { get; set; }      // 文本内容
    public Vector4 Color { get; set; }    // 文本颜色
    public float Kerning { get; set; }    // 字距（字符间距）
    public float LineSpacing { get; set; } // 行距
}
```

**排版参数**：
- **Kerning（字距）**：调整字符之间的水平间距
- **LineSpacing（行距）**：调整行与行之间的垂直间距

---

## 向量数学库

### 设计哲学

向量使用 **struct（值类型）** 而非 class（引用类型），原因：

1. **性能**：值类型在栈上分配，速度快
2. **语义**：向量是数值，应该像 int、float 一样使用
3. **避免 null**：值类型不能为 null，减少空引用错误

### Vector2 详解

#### 数据结构

```csharp
public struct Vector2
{
    public float X, Y;  // 公共字段，直接访问
}
```

#### 构造函数

```csharp
// 标量构造：两个分量相同
public Vector2(float scalar)
{
    X = scalar;
    Y = scalar;
}

// 分量构造：指定 X 和 Y
public Vector2(float x, float y)
{
    X = x;
    Y = y;
}
```

**使用场景**：
```csharp
var v1 = new Vector2(5.0f);       // (5, 5)
var v2 = new Vector2(3.0f, 4.0f); // (3, 4)
```

#### 静态属性

```csharp
public static Vector2 Zero => new Vector2(0.0f);  // 零向量 (0, 0)
```

**知识点**：
- **表达式体属性**：使用 `=>` 定义只读属性
- **静态属性**：无需实例即可访问：`Vector2.Zero`

#### 运算符重载

##### 向量加法

```csharp
public static Vector2 operator +(Vector2 a, Vector2 b)
{
    return new Vector2(a.X + b.X, a.Y + b.Y);
}
```

**使用**：
```csharp
var v1 = new Vector2(1.0f, 2.0f);
var v2 = new Vector2(3.0f, 4.0f);
var sum = v1 + v2;  // (4, 6)
```

##### 标量乘法

```csharp
public static Vector2 operator *(Vector2 vector, float scalar)
{
    return new Vector2(vector.X * scalar, vector.Y * scalar);
}
```

**数学意义**：缩放向量长度

#### 长度计算

```csharp
public float LengthSquared()
{
    return X * X + Y * Y;
}

public float Length()
{
    return (float)Math.Sqrt(LengthSquared());
}
```

**性能优化技巧**：
- 比较距离时，使用 `LengthSquared()` 避免开方运算
- 例如：`if (v.LengthSquared() < 100.0f)` 比 `if (v.Length() < 10.0f)` 快

---

### Vector3 详解

#### 扩展功能

```csharp
public struct Vector3
{
    public float X, Y, Z;
    
    // 从 Vector2 构造
    public Vector3(Vector2 xy, float z)
    {
        X = xy.X;
        Y = xy.Y;
        Z = z;
    }
    
    // XY 平面投影
    public Vector2 XY
    {
        get => new Vector2(X, Y);
        set
        {
            X = value.X;
            Y = value.Y;
        }
    }
}
```

**用途示例**：
```csharp
// 2D 到 3D 转换
Vector2 pos2D = new Vector2(10.0f, 20.0f);
Vector3 pos3D = new Vector3(pos2D, 0.0f);

// 3D 到 2D 投影
Vector3 worldPos = entity.Translation;
Vector2 screenPos = worldPos.XY;
```

---

### Vector4 详解

#### 用途

1. **RGBA 颜色**：(R, G, B, A)
2. **齐次坐标**：(X, Y, Z, W) 用于矩阵变换
3. **矩形**：(left, top, right, bottom)

#### XYZ 投影

```csharp
public Vector3 XYZ
{
    get => new Vector3(X, Y, Z);
    set
    {
        X = value.X;
        Y = value.Y;
        Z = value.Z;
    }
}
```

**颜色操作示例**：
```csharp
// 创建半透明红色
var color = new Vector4(1.0f, 0.0f, 0.0f, 0.5f);

// 提取 RGB 部分
Vector3 rgb = color.XYZ;
```

---

## 输入系统

### 设计简洁性

```csharp
public class Input
{
    public static bool IsKeyDown(KeyCode keycode)
    {
        return InternalCalls.Input_IsKeyDown(keycode);
    }
}
```

**设计特点**：
1. **静态类**：无需实例化，直接使用
2. **单一职责**：每个方法只做一件事
3. **枚举参数**：使用 KeyCode 枚举，避免魔法数字

### KeyCode 枚举

```csharp
public enum KeyCode
{
    Space = 32,
    A = 65,
    B = 66,
    // ... 更多按键
    Enter = 257,
    Escape = 256,
}
```

**来源**：基于 GLFW 键码标准

### 使用示例

```csharp
public class Player : Entity
{
    private Vector2 velocity;
    
    public void OnUpdate(float deltaTime)
    {
        // 移动控制
        if (Input.IsKeyDown(KeyCode.W))
            velocity.Y = 5.0f;
        if (Input.IsKeyDown(KeyCode.S))
            velocity.Y = -5.0f;
        if (Input.IsKeyDown(KeyCode.A))
            velocity.X = -5.0f;
        if (Input.IsKeyDown(KeyCode.D))
            velocity.X = 5.0f;
        
        // 更新位置
        Translation += new Vector3(velocity * deltaTime, 0.0f);
    }
}
```

---

## 渲染组件

### 渲染流程概述

```
Entity → 添加渲染组件 → 引擎自动渲染 → 显示在屏幕上
```

### 精灵渲染示例

```csharp
// 创建一个红色方块
var entity = new Entity();
var sprite = entity.GetComponent<SpriteRendererComponent>();
sprite.Color = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);  // 红色
```

### 圆形渲染进阶

```csharp
// 创建一个带渐变的环形
var circle = entity.GetComponent<CircleRendererComponent>();
circle.Color = new Vector4(0.2f, 0.6f, 1.0f, 1.0f);  // 蓝色
circle.Thickness = 0.8f;  // 较粗的环
circle.Fade = 0.1f;       // 柔和的边缘
```

**视觉效果**：
- `Thickness` 接近 0：实心圆
- `Thickness` 接近 1：细环
- `Fade` 越大：边缘越模糊

---

## 物理系统

### Box2D 集成

XingXing 引擎使用 Box2D 物理引擎，Hazel API 对其进行了封装。

### 施加冲量示例

```csharp
public class Ball : Entity
{
    public void Kick(Vector2 direction, float power)
    {
        var rb = GetComponent<Rigidbody2DComponent>();
        Vector2 impulse = direction * power;
        rb.ApplyLinearImpulse(impulse, wake: true);
    }
}
```

**参数说明**：
- `impulse`：冲量向量（方向 × 力度）
- `wake`：是否唤醒休眠的物体

### 速度查询

```csharp
public class SpeedMeter : Entity
{
    public void DisplaySpeed()
    {
        var rb = GetComponent<Rigidbody2DComponent>();
        Vector2 velocity = rb.LinearVelocity;
        float speed = velocity.Length();
        
        // 显示速度
        var text = GetComponent<TextComponent>();
        text.Text = $"速度: {speed:F2} m/s";
    }
}
```

---

## 内部调用机制

### P/Invoke 基础

**P/Invoke（Platform Invoke）** 是 .NET 调用非托管代码（如 C++）的机制。

### MethodImplAttribute 详解

```csharp
[MethodImplAttribute(MethodImplOptions.InternalCall)]
internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
```

**关键点**：
1. **MethodImplOptions.InternalCall**：标记为内部调用
2. **extern**：表示方法实现在外部（C++）
3. **static**：静态方法，无需实例
4. **internal**：仅 Hazel 内部可见

### 参数传递约定

#### out 参数

```csharp
internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);
```

**用途**：C++ 将结果写入该参数

#### ref 参数

```csharp
internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
```

**用途**：传递引用，避免大型结构复制

### 命名约定

内部调用遵循命名模式：`<ComponentType>_<Action>`

例如：
- `Entity_HasComponent`
- `TransformComponent_GetTranslation`
- `Rigidbody2DComponent_ApplyLinearImpulse`

**优点**：
1. 清晰表达功能
2. 便于在 C++ 端查找对应实现
3. 避免命名冲突

---

## 完整示例

### 示例 1：可控制的玩家角色

```csharp
using Hazel;

namespace Game
{
    public class Player : Entity
    {
        private float moveSpeed = 10.0f;
        private float jumpForce = 500.0f;
        
        private Rigidbody2DComponent rigidbody;
        private SpriteRendererComponent sprite;
        
        // 初始化
        public void OnCreate()
        {
            rigidbody = GetComponent<Rigidbody2DComponent>();
            sprite = GetComponent<SpriteRendererComponent>();
            
            // 设置为绿色
            sprite.Color = new Vector4(0.0f, 1.0f, 0.0f, 1.0f);
        }
        
        // 每帧更新
        public void OnUpdate(float deltaTime)
        {
            HandleMovement(deltaTime);
            HandleJump();
            UpdateColor();
        }
        
        private void HandleMovement(float deltaTime)
        {
            Vector2 velocity = rigidbody.LinearVelocity;
            
            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -moveSpeed;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = moveSpeed;
            else
                velocity.X = 0.0f;
            
            // 注意：这里简化了处理，实际应通过 rigidbody 设置速度
            Translation += new Vector3(velocity.X * deltaTime, 0.0f, 0.0f);
        }
        
        private void HandleJump()
        {
            if (Input.IsKeyDown(KeyCode.Space))
            {
                Vector2 impulse = new Vector2(0.0f, jumpForce);
                rigidbody.ApplyLinearImpulse(impulse, wake: true);
            }
        }
        
        private void UpdateColor()
        {
            // 根据速度改变颜色
            float speed = rigidbody.LinearVelocity.Length();
            float t = speed / 20.0f;  // 归一化
            
            // 速度越快越红
            sprite.Color = new Vector4(t, 1.0f - t, 0.0f, 1.0f);
        }
    }
}
```

### 示例 2：追踪敌人

```csharp
using Hazel;
using System;

namespace Game
{
    public class Enemy : Entity
    {
        private Entity target;
        private float chaseSpeed = 3.0f;
        
        public void OnCreate()
        {
            // 查找玩家
            target = FindEntityByName("Player");
        }
        
        public void OnUpdate(float deltaTime)
        {
            if (target == null)
                return;
            
            // 计算方向
            Vector3 targetPos = target.Translation;
            Vector3 currentPos = Translation;
            Vector3 direction = targetPos - currentPos;
            
            // 归一化（需要 Vector3 扩展方法）
            float distance = direction.Length();
            if (distance > 0.1f)
            {
                direction = direction * (1.0f / distance);
                Translation += direction * chaseSpeed * deltaTime;
            }
            
            // 改变颜色表示距离
            var sprite = GetComponent<SpriteRendererComponent>();
            float intensity = 1.0f - Math.Min(distance / 50.0f, 1.0f);
            sprite.Color = new Vector4(intensity, 0.0f, 0.0f, 1.0f);
        }
    }
}
```

**扩展方法补充**（可添加到 Vector3.cs）：
```csharp
public float Length()
{
    return (float)Math.Sqrt(X * X + Y * Y + Z * Z);
}

public static Vector3 operator -(Vector3 a, Vector3 b)
{
    return new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
}
```

### 示例 3：文本显示面板

```csharp
using Hazel;

namespace Game
{
    public class ScoreBoard : Entity
    {
        private int score = 0;
        private TextComponent textComponent;
        
        public void OnCreate()
        {
            textComponent = GetComponent<TextComponent>();
            UpdateDisplay();
        }
        
        public void AddScore(int points)
        {
            score += points;
            UpdateDisplay();
        }
        
        private void UpdateDisplay()
        {
            textComponent.Text = $"得分: {score}";
            textComponent.Color = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // 白色
            textComponent.Kerning = 0.0f;
            textComponent.LineSpacing = 1.0f;
        }
    }
}
```

---

## 最佳实践

### 1. 组件缓存

❌ **不好的做法**（每帧查询）：
```csharp
public void OnUpdate(float deltaTime)
{
    var sprite = GetComponent<SpriteRendererComponent>();
    sprite.Color = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
}
```

✅ **好的做法**（缓存组件）：
```csharp
private SpriteRendererComponent sprite;

public void OnCreate()
{
    sprite = GetComponent<SpriteRendererComponent>();
}

public void OnUpdate(float deltaTime)
{
    sprite.Color = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
}
```

### 2. 避免频繁的跨语言调用

❌ **不好的做法**：
```csharp
public void OnUpdate(float deltaTime)
{
    Translation = new Vector3(Translation.X + 1.0f, Translation.Y, Translation.Z);
}
```

✅ **好的做法**：
```csharp
public void OnUpdate(float deltaTime)
{
    Vector3 pos = Translation;  // 一次调用
    pos.X += 1.0f;
    Translation = pos;          // 一次调用
}
```

### 3. 使用值类型优化性能

对于小型数据（≤16字节），使用 struct：
```csharp
public struct GameState  // struct，不是 class
{
    public int Health;
    public int Mana;
    public float Speed;
}
```

### 4. 空检查

```csharp
var entity = FindEntityByName("Boss");
if (entity != null)  // 总是检查 null
{
    // 使用 entity
}
```

### 5. 枚举优于魔法数字

❌ **不好的做法**：
```csharp
if (Input.IsKeyDown(87))  // 87 是什么键？
```

✅ **好的做法**：
```csharp
if (Input.IsKeyDown(KeyCode.W))  // 清晰明了
```

---

## 常见问题 (FAQ)

### Q1: 为什么 Entity ID 是 ulong 而不是 int？

**答**：ulong (64位无符号整数) 的范围是 0 到 18,446,744,073,709,551,615，足以支持海量实体而不重复。即使每秒创建 1000 个实体，也需要 584 亿年才会用完 ID。

### Q2: Vector 为什么是 struct 而不是 class？

**答**：
1. **性能**：struct 在栈上分配，速度更快
2. **语义**：向量是值，应该像 int、float 一样传递
3. **内存**：避免堆分配和垃圾回收

### Q3: 为什么不能直接创建 Entity？

**答**：Entity 的构造函数是 `internal` 的，因为实体必须由引擎管理。直接创建的 Entity 没有对应的 C++ 底层对象，调用任何方法都会失败。

### Q4: Component.Entity 为什么是 internal set？

**答**：防止外部代码修改组件与实体的关联关系，这种关系应该由引擎内部维护。

### Q5: 如何调试 InternalCalls 失败？

**答**：
1. 检查 C++ 端是否正确注册了该方法
2. 确认方法签名（参数类型、顺序）完全匹配
3. 使用 Visual Studio 的调试器，同时调试 C# 和 C++ 代码

---

## 扩展阅读

1. **ECS 架构**：
   - [Game Programming Patterns - Component](https://gameprogrammingpatterns.com/component.html)
   
2. **P/Invoke**：
   - [Microsoft Docs - Platform Invoke](https://docs.microsoft.com/en-us/dotnet/standard/native-interop/pinvoke)

3. **Box2D 物理引擎**：
   - [Box2D Manual](https://box2d.org/documentation/)

4. **C# 性能优化**：
   - [C# Performance Tips](https://docs.microsoft.com/en-us/dotnet/csharp/write-safe-efficient-code)

---

## 总结

Hazel C# API 通过精心设计的封装，为游戏开发者提供了：

1. ✅ **简洁的接口**：隐藏 C++ 复杂性
2. ✅ **类型安全**：利用 C# 强类型系统
3. ✅ **高性能**：通过 P/Invoke 和值类型优化
4. ✅ **模块化设计**：易于理解和扩展

掌握这些概念后，你就可以开始使用 Hazel API 开发自己的游戏逻辑了！

---

**文档版本**：1.0  
**最后更新**：2026-01  
**维护者**：XingXing Engine Team
