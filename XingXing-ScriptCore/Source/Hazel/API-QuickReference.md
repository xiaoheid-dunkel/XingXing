# 🚀 Hazel C# API 快速参考手册

> 本文档提供 Hazel API 的快速查询表，适合已经熟悉基本概念的开发者。

---

## 📑 目录

- [Entity 类](#entity-类)
- [Component 组件](#component-组件)
- [Vector 向量](#vector-向量)
- [Input 输入](#input-输入)
- [KeyCode 键码](#keycode-键码)

---

## Entity 类

### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `ID` | `ulong` | 只读，实体唯一标识符 |
| `Translation` | `Vector3` | 读写，实体世界坐标 |

### 方法

```csharp
// 组件操作
bool HasComponent<T>()                // 检查是否有组件
T GetComponent<T>()                   // 获取组件（泛型）

// 实体查找
Entity FindEntityByName(string name) // 按名称查找实体

// 类型转换
T As<T>()                            // 转换为自定义实体类型
```

### 使用示例

```csharp
var entity = new Entity(id);
if (entity.HasComponent<SpriteRendererComponent>())
{
    var sprite = entity.GetComponent<SpriteRendererComponent>();
}
```

---

## Component 组件

### 基类

```csharp
public abstract class Component
{
    public Entity Entity { get; internal set; }
}
```

### TransformComponent

```csharp
var transform = entity.GetComponent<TransformComponent>();
transform.Translation = new Vector3(x, y, z);
Vector3 pos = transform.Translation;
```

### SpriteRendererComponent

```csharp
var sprite = entity.GetComponent<SpriteRendererComponent>();
sprite.Color = new Vector4(r, g, b, a);  // 范围 [0.0, 1.0]
```

**预设颜色**：
```csharp
// 红色
sprite.Color = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
// 绿色
sprite.Color = new Vector4(0.0f, 1.0f, 0.0f, 1.0f);
// 蓝色
sprite.Color = new Vector4(0.0f, 0.0f, 1.0f, 1.0f);
// 白色
sprite.Color = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
// 黑色
sprite.Color = new Vector4(0.0f, 0.0f, 0.0f, 1.0f);
// 半透明白色
sprite.Color = new Vector4(1.0f, 1.0f, 1.0f, 0.5f);
```

### CircleRendererComponent

```csharp
var circle = entity.GetComponent<CircleRendererComponent>();
circle.Color = new Vector4(r, g, b, a);
circle.Thickness = 0.8f;  // 0.0 = 实心，1.0 = 细环
circle.Fade = 0.05f;      // 边缘渐变（越大越模糊）
```

### Rigidbody2DComponent

```csharp
var rb = entity.GetComponent<Rigidbody2DComponent>();

// 刚体类型
rb.Type = Rigidbody2DComponent.BodyType.Dynamic;  // 动态
rb.Type = Rigidbody2DComponent.BodyType.Static;   // 静态
rb.Type = Rigidbody2DComponent.BodyType.Kinematic; // 运动学

// 速度查询（只读）
Vector2 velocity = rb.LinearVelocity;

// 施加冲量（在中心）
rb.ApplyLinearImpulse(new Vector2(x, y), wake: true);

// 施加冲量（指定位置）
rb.ApplyLinearImpulse(impulse, worldPosition, wake: true);
```

**BodyType 说明**：
- **Dynamic**：受力和重力影响（玩家、敌人、球）
- **Static**：静止不动（地面、墙壁）
- **Kinematic**：可移动但不受力（移动平台、电梯）

### BoxCollider2DComponent

```csharp
var collider = entity.GetComponent<BoxCollider2DComponent>();
// 当前为占位组件，暂无属性
```

### CircleCollider2DComponent

```csharp
var collider = entity.GetComponent<CircleCollider2DComponent>();
// 当前为占位组件，暂无属性
```

### TextComponent

```csharp
var text = entity.GetComponent<TextComponent>();
text.Text = "Hello, World!";
text.Color = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
text.Kerning = 0.0f;      // 字距（字符间距）
text.LineSpacing = 1.0f;  // 行距
```

### CameraComponent

```csharp
var camera = entity.GetComponent<CameraComponent>();
// 当前为占位组件，暂无属性
```

---

## Vector 向量

### Vector2

```csharp
// 构造
var v1 = new Vector2(x, y);
var v2 = new Vector2(scalar);  // (scalar, scalar)
var zero = Vector2.Zero;       // (0, 0)

// 字段
float x = v1.X;
float y = v1.Y;

// 运算
Vector2 sum = v1 + v2;           // 向量加法
Vector2 scaled = v1 * 2.0f;      // 标量乘法

// 方法
float lengthSq = v1.LengthSquared();  // 长度平方
float length = v1.Length();           // 长度
```

### Vector3

```csharp
// 构造
var v1 = new Vector3(x, y, z);
var v2 = new Vector3(scalar);
var v3 = new Vector3(vector2, z);  // 从 Vector2 扩展
var zero = Vector3.Zero;           // (0, 0, 0)

// 字段
float x = v1.X;
float y = v1.Y;
float z = v1.Z;

// XY 投影
Vector2 xy = v1.XY;
v1.XY = new Vector2(10, 20);  // 设置 X 和 Y

// 运算
Vector3 sum = v1 + v2;
Vector3 scaled = v1 * 2.0f;
```

### Vector4

```csharp
// 构造
var v1 = new Vector4(x, y, z, w);
var v2 = new Vector4(scalar);
var v3 = new Vector4(vector3, w);  // 从 Vector3 扩展
var zero = Vector4.Zero;           // (0, 0, 0, 0)

// 字段
float x = v1.X;
float y = v1.Y;
float z = v1.Z;
float w = v1.W;

// 投影
Vector2 xy = v1.XY;
v1.XY = new Vector2(10, 20);
Vector3 xyz = v1.XYZ;
v1.XYZ = new Vector3(1, 2, 3);

// 运算
Vector4 sum = v1 + v2;
Vector4 scaled = v1 * 2.0f;
```

---

## Input 输入

### 方法

```csharp
bool Input.IsKeyDown(KeyCode keycode)
```

### 使用示例

```csharp
if (Input.IsKeyDown(KeyCode.W))
{
    // W 键被按下
}

if (Input.IsKeyDown(KeyCode.Space))
{
    // 空格键被按下
}
```

---

## KeyCode 键码

### 常用按键

#### 字母键
```csharp
KeyCode.A ~ KeyCode.Z  // 65 ~ 90
```

#### 数字键
```csharp
KeyCode.D0 ~ KeyCode.D9  // 48 ~ 57 (D0 代表数字 0)
```

#### 方向键
```csharp
KeyCode.Up       // 上
KeyCode.Down     // 下
KeyCode.Left     // 左
KeyCode.Right    // 右
```

#### 功能键
```csharp
KeyCode.Space       // 空格
KeyCode.Enter       // 回车
KeyCode.Escape      // Esc
KeyCode.Tab         // Tab
KeyCode.Backspace   // 退格
KeyCode.Delete      // Delete
```

#### 修饰键
```csharp
KeyCode.LeftShift     // 左 Shift
KeyCode.RightShift    // 右 Shift
KeyCode.LeftControl   // 左 Ctrl
KeyCode.RightControl  // 右 Ctrl
KeyCode.LeftAlt       // 左 Alt
KeyCode.RightAlt      // 右 Alt
```

#### F 键
```csharp
KeyCode.F1 ~ KeyCode.F12  // F1 到 F12
```

#### 小键盘
```csharp
KeyCode.KP0 ~ KeyCode.KP9    // 数字 0-9
KeyCode.KPAdd                 // +
KeyCode.KPSubtract            // -
KeyCode.KPMultiply            // *
KeyCode.KPDivide              // /
KeyCode.KPEnter               // 回车
```

#### 符号键
```csharp
KeyCode.Comma        // ,
KeyCode.Period       // .
KeyCode.Slash        // /
KeyCode.Semicolon    // ;
KeyCode.Apostrophe   // '
KeyCode.LeftBracket  // [
KeyCode.RightBracket // ]
KeyCode.Minus        // -
KeyCode.Equal        // =
```

---

## 📘 代码模板

### 基础实体脚本

```csharp
using Hazel;

namespace Game
{
    public class MyEntity : Entity
    {
        // 组件缓存
        private TransformComponent transform;
        private SpriteRendererComponent sprite;
        
        // 初始化（游戏开始时调用一次）
        public void OnCreate()
        {
            transform = GetComponent<TransformComponent>();
            sprite = GetComponent<SpriteRendererComponent>();
        }
        
        // 更新（每帧调用）
        public void OnUpdate(float deltaTime)
        {
            // 游戏逻辑
        }
        
        // 销毁（实体被删除时调用）
        public void OnDestroy()
        {
            // 清理资源
        }
    }
}
```

### 移动控制模板

```csharp
public void OnUpdate(float deltaTime)
{
    float speed = 10.0f;
    Vector3 movement = Vector3.Zero;
    
    if (Input.IsKeyDown(KeyCode.W))
        movement.Y += 1.0f;
    if (Input.IsKeyDown(KeyCode.S))
        movement.Y -= 1.0f;
    if (Input.IsKeyDown(KeyCode.A))
        movement.X -= 1.0f;
    if (Input.IsKeyDown(KeyCode.D))
        movement.X += 1.0f;
    
    Translation += movement * speed * deltaTime;
}
```

### 物理控制模板

```csharp
public void OnUpdate(float deltaTime)
{
    var rb = GetComponent<Rigidbody2DComponent>();
    
    // 移动
    if (Input.IsKeyDown(KeyCode.A))
    {
        Vector2 impulse = new Vector2(-10.0f, 0.0f);
        rb.ApplyLinearImpulse(impulse, wake: true);
    }
    
    // 跳跃
    if (Input.IsKeyDown(KeyCode.Space))
    {
        Vector2 jumpImpulse = new Vector2(0.0f, 500.0f);
        rb.ApplyLinearImpulse(jumpImpulse, wake: true);
    }
}
```

### 颜色动画模板

```csharp
private float time = 0.0f;

public void OnUpdate(float deltaTime)
{
    time += deltaTime;
    
    var sprite = GetComponent<SpriteRendererComponent>();
    
    // 颜色循环变化
    float r = (float)Math.Sin(time) * 0.5f + 0.5f;
    float g = (float)Math.Sin(time + 2.0f) * 0.5f + 0.5f;
    float b = (float)Math.Sin(time + 4.0f) * 0.5f + 0.5f;
    
    sprite.Color = new Vector4(r, g, b, 1.0f);
}
```

---

## ⚡ 性能提示

### ✅ 最佳实践

```csharp
// 1. 缓存组件引用
private SpriteRendererComponent sprite;
public void OnCreate() 
{
    sprite = GetComponent<SpriteRendererComponent>();
}

// 2. 减少跨语言调用
Vector3 pos = Translation;  // 读取一次
pos.X += 1.0f;
pos.Y += 2.0f;
Translation = pos;          // 写入一次

// 3. 使用 LengthSquared 比较距离
if (v.LengthSquared() < 100.0f)  // 快
    // 等价于 v.Length() < 10.0f

// 4. 合理使用值类型
public struct GameData  // struct，不是 class
{
    public int Score;
    public float Time;
}
```

### ❌ 避免的写法

```csharp
// 1. 每帧查询组件
public void OnUpdate(float deltaTime)
{
    var sprite = GetComponent<SpriteRendererComponent>();  // ❌ 慢
}

// 2. 频繁的属性访问
Translation = new Vector3(Translation.X + 1, Translation.Y, Translation.Z);  // ❌ 两次调用

// 3. 不必要的开方
if (v.Length() < 10.0f)  // ❌ 有开方运算
```

---

## 🐛 常见错误

### 错误 1：空引用异常

```csharp
// ❌ 错误：没有检查 null
var entity = FindEntityByName("Player");
entity.Translation = Vector3.Zero;  // 可能抛出 NullReferenceException

// ✅ 正确：先检查
var entity = FindEntityByName("Player");
if (entity != null)
{
    entity.Translation = Vector3.Zero;
}
```

### 错误 2：组件不存在

```csharp
// ❌ 错误：假设组件存在
var sprite = entity.GetComponent<SpriteRendererComponent>();
sprite.Color = new Vector4(1, 0, 0, 1);  // 如果组件不存在，sprite 为 null

// ✅ 正确：先检查
if (entity.HasComponent<SpriteRendererComponent>())
{
    var sprite = entity.GetComponent<SpriteRendererComponent>();
    sprite.Color = new Vector4(1, 0, 0, 1);
}
```

### 错误 3：错误的刚体类型

```csharp
// ❌ 错误：对静态刚体施加力
var rb = entity.GetComponent<Rigidbody2DComponent>();
rb.Type = Rigidbody2DComponent.BodyType.Static;
rb.ApplyLinearImpulse(new Vector2(10, 0), true);  // 不会有效果

// ✅ 正确：使用动态刚体
var rb = entity.GetComponent<Rigidbody2DComponent>();
rb.Type = Rigidbody2DComponent.BodyType.Dynamic;
rb.ApplyLinearImpulse(new Vector2(10, 0), true);
```

---

## 📞 快速查询表

### 组件类型总览

| 组件类型 | 用途 | 主要属性/方法 |
|---------|------|--------------|
| `TransformComponent` | 位置变换 | `Translation` |
| `SpriteRendererComponent` | 精灵渲染 | `Color` |
| `CircleRendererComponent` | 圆形渲染 | `Color`, `Thickness`, `Fade` |
| `Rigidbody2DComponent` | 物理刚体 | `Type`, `LinearVelocity`, `ApplyLinearImpulse()` |
| `BoxCollider2DComponent` | 盒碰撞体 | （占位） |
| `CircleCollider2DComponent` | 圆碰撞体 | （占位） |
| `TextComponent` | 文本渲染 | `Text`, `Color`, `Kerning`, `LineSpacing` |
| `CameraComponent` | 摄像机 | （占位） |

### Vector 类型对比

| 类型 | 字段 | 常见用途 |
|------|------|---------|
| `Vector2` | `X, Y` | 2D 位置、速度、冲量 |
| `Vector3` | `X, Y, Z` | 3D 位置、方向 |
| `Vector4` | `X, Y, Z, W` | RGBA 颜色、齐次坐标 |

### 常用数学常量

```csharp
Vector2.Zero  // (0, 0)
Vector3.Zero  // (0, 0, 0)
Vector4.Zero  // (0, 0, 0, 0)
```

---

**快速参考版本**：1.0  
**配套完整教程**：[README.md](README.md)  
**最后更新**：2026-01
