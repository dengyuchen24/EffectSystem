// Effect.hpp - 效果基类及派生类型
#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <any>
#include <memory>
#include <concepts>
#include <vector>
#include <algorithm>

class EffectTarget;
using EffectData = std::map<std::string, std::any>;

class Effect {
protected:
	EffectTarget* owner;
	std::string   effect_name;
	EffectData    effect_data;
	int64_t       last_duration = 0;

public:
	Effect() = delete;
	Effect(EffectTarget* owner, const std::string& name)
		: owner(owner), effect_name(name) {
	}
	virtual ~Effect() = default;

	virtual void trigger() = 0;
	virtual bool end() { return true; }
	virtual void start(EffectData* data, bool is_new_effect = false) {}
	virtual std::string description() = 0;
	virtual void update(int64_t delta_time) = 0;

	virtual uint32_t getTriggerTimings() const { return 0; }
	virtual bool needsUpdate() const { return false; }

	void addEffectDataKey(const std::string& key, std::any val) { effect_data[key] = std::move(val); }
	void changeEffectDataKey(const std::string& key, std::any val) { effect_data[key] = std::move(val); }
	void removeEffectDataKey(const std::string& key) { effect_data.erase(key); }

	EffectTarget* getOwner() const { return owner; }
	const std::string& getName() const { return effect_name; }
	const EffectData& getData() const { return effect_data; }
	int64_t getLastDuration() const { return last_duration; }
	void setLastDuration(int64_t dur) { last_duration = dur; }
};

class ForeverEffect : public virtual Effect {
public:
	ForeverEffect(EffectTarget* owner, const std::string& name) : Effect(owner, name) {}
	void update(int64_t) override {}
	bool end() override { return false; }
	bool needsUpdate() const override { return false; }
};

class StackEffect : public virtual Effect {
protected:
	int stack_count = 0;
	int max_stacks = -1;

public:
	StackEffect(EffectTarget* owner, const std::string& name) : Effect(owner, name) {}
	void start(EffectData* data, bool is_new_effect) override {
		if (is_new_effect) stack_count = 1;
		else if (max_stacks < 0 || stack_count < max_stacks) ++stack_count;
		Effect::start(data, is_new_effect);
	}
	int getStackCount() const { return stack_count; }
};

class LimitedStackEffect : public StackEffect {
public:
	LimitedStackEffect(EffectTarget* owner, const std::string& name, int max)
		: Effect(owner, name), StackEffect(owner, name) {
		max_stacks = max;
	}
};

class SeparateStackEffect : public StackEffect {
protected:
	std::vector<int64_t> layer_durations;

public:
	SeparateStackEffect(EffectTarget* owner, const std::string& name)
		: Effect(owner, name), StackEffect(owner, name) {
	}

	void start(EffectData* data, bool is_new_effect) override {
		if (is_new_effect) {
			stack_count = 1;
			layer_durations.clear();
			layer_durations.push_back(last_duration);
		}
		else {
			if (max_stacks < 0 || stack_count < max_stacks) {
				++stack_count;
				layer_durations.push_back(last_duration);
			}
		}
		Effect::start(data, is_new_effect);
	}

	void update(int64_t delta_time) override {
		for (auto& dur : layer_durations) dur -= delta_time;
		layer_durations.erase(
			std::remove_if(layer_durations.begin(), layer_durations.end(),
				[](int64_t d) { return d <= 0; }),
			layer_durations.end());
		stack_count = static_cast<int>(layer_durations.size());

		// 所有层消失后标记效果结束
		if (stack_count == 0) {
			setLastDuration(0);
		}
	}
};