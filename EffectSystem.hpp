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