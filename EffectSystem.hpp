// EffectSystem.hpp - 效果系统统一接口
// 使用方式：在项目中直接 #include "EffectSystem/EffectSystem.hpp" 即可引入全部核心功能。
#pragma once

#ifndef _VCRUNTIME_H
#include <vcruntime.h>
#endif

#if _HAS_CXX20

#include "src/Effect.hpp"
#include "src/EffectTarget.hpp"
#include "src/Trigger.hpp"

#else
#error "EffectSystem requires C++20 or higher!"
#endif

// 此处可以继续添加全局配置或辅助函数，例如版本信息等。

//这里是 EffectSystem 使用指南

/*
1. 系统概述


EffectSystem 是一个基于 C++20 的轻量级效果框架，为游戏或模拟程序中的角色提供统一的效果（Buff/Debuff）管理。它支持：

多种效果类型：可叠加、限层、每层独立计时、永久效果等
高效的事件驱动与按需更新（避免全量遍历）
效果数据的动态存储与交互（通过 EffectData）
灵活的扩展：通过继承基类并覆写关键虚函数即可定制新效果

所有功能通过以下几个头文件提供：

文件	                作用
EffectSystem.hpp	统一包含入口（自动包含以下所有文件）
src/Effect.hpp	        效果基类及常用派生类（StackEffect、ForeverEffect 等）
src/EffectTarget.hpp	效果拥有者基类（角色、物品等继承它）
src/Trigger.hpp	        事件触发器系统（可选，内部使用）


2. 核心概念


2.1 效果 (Effect)
所有效果均派生自 Effect 基类。一个效果具有：

名称 (effect_name)：唯一标识符，同一角色上不允许同名效果共存（用于叠层）。
拥有者 (owner)：挂载效果的对象（EffectTarget 的子类）。
持续时长 (last_duration)：剩余时间（毫秒），≤0 表示无限时或已过期。
效果数据 (effect_data)：一个 std::map<std::string, std::any> 字典，用于传递/存储自定义参数。

2.2 效果目标 (EffectTarget)
任何需要挂载效果的对象都应继承 EffectTarget（别名为 EffectOwner）。该类负责：

存储所有效果
按需更新效果（updateAllEffects）
响应游戏事件并触发效果（triggerEvent）

2.3 事件触发 (Trigger)
系统内部使用位掩码标识事件时机（Timing::ON_DAMAGED 等）。效果可以通过覆写 getTriggerTimings() 声明自己关心哪些事件，EffectTarget 在添加效果时自动注册，在事件发生时自动调用效果的 trigger()。

2.4 更新策略
为避免每帧遍历所有效果，系统将效果分为两类：

需要更新：覆写 needsUpdate() 返回 true，放入 updateList，由 updateAllEffects 调用。
事件驱动：覆写 getTriggerTimings() 返回非零，注册到事件系统，仅在对应事件发生时触发。

一个效果可以同时属于两类。


3. 快速开始


3.1 让角色支持效果
cpp
#include "EffectSystem/EffectSystem.hpp"

class Player : public EffectOwner {
public:
    std::string name;
    int hp = 100;
    // ... 其他成员
};
3.2 创建自定义效果
以“中毒”为例：

cpp
#include "EffectSystem/ExampleEffects.hpp"   // 或自己写

class PoisonEffect : public SeparateStackEffect {
public:
    PoisonEffect(EffectOwner* owner, const std::string& name)
        : Effect(owner, name), SeparateStackEffect(owner, name) {
        setLastDuration(3000);   // 每层持续 3 秒
    }

    // 需要每帧更新
    bool needsUpdate() const override { return true; }

    void update(int64_t delta) override {
        SeparateStackEffect::update(delta);
        if (getStackCount() > 0) {
            // 造成伤害等逻辑
        }
    }

    void trigger() override {}
    std::string description() override {
        return "Poison: " + std::to_string(getStackCount()) + " stacks";
    }
};
要点：

构造函数必须直接调用虚拟基类 Effect 的构造函数（由于菱形继承），然后调用直接基类（如 SeparateStackEffect）的构造函数。
覆写 needsUpdate() / getTriggerTimings() 以声明行为特性。
update 内处理每帧逻辑，并记得调用基类 update（如果基类有自己的倒计时逻辑）。
覆写 description() 返回可读的效果描述。

3.3 添加效果到角色
cpp
Player player;
player.addEffect<PoisonEffect>(nullptr, "Venom");   // 参数：数据指针, 效果名称
如果同名效果已存在，会调用 start(data, false) 进行叠层（具体行为由效果类决定）。

可以传入 EffectData 指针来初始化自定义数据。

3.4 驱动效果更新
每帧调用 updateAllEffects，传入本帧的时间增量（毫秒）：

cpp
int64_t deltaTime = 16;   // 约 60FPS
player.updateAllEffects(deltaTime);
系统会自动更新所有需要更新的效果，并移除已过期的效果。

3.5 触发游戏事件
cpp
player.takeDamage(25);   // 例如：在受伤逻辑中触发事件
// 在 takeDamage 内部调用 triggerEvent(Timing::ON_DAMAGED);
护盾等效果会通过 trigger() 响应并执行吸收逻辑。

3.6 查询与移除
cpp
bool has = player.hasEffect("Venom");
Effect* eff = player.getEffect("Venom");
PoisonEffect* poison = dynamic_cast<PoisonEffect*>(eff);   // 类型安全转换

player.removeEffect("Venom");   // 如果效果的 end() 允许移除


4. 内置效果类型


类名	                说明
Effect	                抽象基类，所有效果的直接或间接父类
ForeverEffect	            永久效果，end() 返回 false（不可移除）
StackEffect	            可叠加效果，默认无上限
LimitedStackEffect	        有最大层数限制的叠加效果
SeparateStackEffect	    每层独立计时的叠加效果（自动过期）
您可以多重继承它们，例如：

cpp
class MyEffect : public ForeverEffect, public StackEffect {
    // 必须显式构造 Effect 和两个基类
};


5. 效果间数据交互


通过 EffectData 可在添加效果时传递参数：

cpp
EffectData data;
data["value"] = 100;
player.addEffect<SomeEffect>(&data, "Buff");
在效果的 start 中读取：

cpp
void start(EffectData* data, bool is_new) override {
    if (data && data->contains("value")) {
        if (auto* p = std::any_cast<int>(&(*data)["value"]))
            myValue = *p;
    }
}
也可以在 update 期间修改 effect_data，供其他效果读取（通过裸指针传递效果引用）。


6. 自定义效果基类规范


当您创建新的效果中间类（如“周期性效果”）时，请遵循以下规则：

构造函数必须调用虚拟基类 Effect 的构造函数，并调用直接基类（如果有）。
覆写 needsUpdate() 和 getTriggerTimings() 以让系统自动管理。
如果效果需要每帧更新，请在 update 中实现，并确保在不需要时（如层数归零）将 last_duration 设为 0 或让 end() 返回 true，以便系统自动移除。
trigger() 是在事件触发时调用的入口，通常用于一次性反应（如护盾吸收伤害）。
description() 用于调试/UI显示，应返回简洁可读文本。


7. 性能注意事项


只有覆写 needsUpdate() == true 的效果才会出现在每帧更新列表中。

只有覆写 getTriggerTimings() != 0 的效果才会在指定事件时被调用。

永久效果（ForeverEffect）通常两种都不需要，无运行时开销。

避免在 update 中直接移除效果，系统会在 updateAllEffects 循环结束后统一移除。


8. 常见问题


Q：为什么我继承 StackEffect 编译失败？
A：所有最终效果类必须在其构造函数初始化列表中直接调用 Effect 的构造函数，例如：

cpp
MyEffect(EffectOwner* owner, const std::string& name)
    : Effect(owner, name), StackEffect(owner, name) { ... }
这是 C++ 虚拟继承的要求。

Q：如何让一个效果既有持续时间又能响应事件？
A：同时覆写 needsUpdate() = true 和 getTriggerTimings() 返回关心的事件即可。

Q：EffectData 支持哪些类型？
A：由于使用 std::any，可以存放任意可拷贝类型。但系统辅助序列化时只处理 int、int64_t、float、double、std::string。如果需要复杂对象，可以存储裸指针或智能指针，但需自行管理生命周期。

Q：如何在不使用 ExampleEffects 的情况下快速开始？
A：只需包含 EffectSystem.hpp，然后按第3节的例子创建自己的效果类，无需依赖 ExampleEffects.hpp。


9. 项目结构建议


text
YourProject/
├── EffectSystem/           (框架文件)
│   ├── EffectSystem.hpp
│   ├── ExampleEffects.hpp  (可选，作为示例)
│   └── src/
│       ├── Effect.hpp
│       ├── EffectTarget.hpp
│       └── Trigger.hpp
├── GameEffects.hpp         (你的游戏专属效果)
└── main.cpp
在游戏效果文件中包含 EffectSystem.hpp 并定义您的效果类。
*/
