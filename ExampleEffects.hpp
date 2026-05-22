// ExampleEffects.hpp
#pragma once
#include "EffectSystem.hpp"
#include <string>
#include <iostream>

class ShieldEffect : public StackEffect {
public:
	explicit ShieldEffect(EffectOwner* owner, const std::string& name)
		: Effect(owner, name), StackEffect(owner, name) {
	}

	uint32_t getTriggerTimings() const override { return Timing::ON_DAMAGED; }
	bool needsUpdate() const override { return false; }

	void start(EffectData* data, bool is_new_effect) override {
		StackEffect::start(data, is_new_effect);
		absorb_per_layer = 50;
		if (data && data->contains("absorb"))
			if (auto* p = std::any_cast<int>(&(*data)["absorb"])) absorb_per_layer = *p;
	}

	void trigger() override {
		std::cout << "  [ShieldTrigger] " << getStackCount() << " stacks absorb damage.\n";
	}

	std::string description() override {
		return "Shield: " + std::to_string(getStackCount()) + " stacks, "
			+ std::to_string(absorb_per_layer) + " each";
	}
	void update(int64_t) override {}

private:
	int absorb_per_layer = 50;
};

class AttackBonusEffect : public ForeverEffect {
public:
	AttackBonusEffect(EffectOwner* owner, const std::string& name, int bonus)
		: Effect(owner, name), ForeverEffect(owner, name), attack_bonus(bonus) {
	}
	void trigger() override {}
	std::string description() override { return "Attack +" + std::to_string(attack_bonus); }
	void update(int64_t) override {}
private:
	int attack_bonus;
};

class PoisonEffect : public SeparateStackEffect {
public:
	PoisonEffect(EffectOwner* owner, const std::string& name)
		: Effect(owner, name), SeparateStackEffect(owner, name) {
		setLastDuration(3000);
		damage_per_tick = 10;
	}
	bool needsUpdate() const override { return true; }

	void start(EffectData* data, bool is_new_effect) override {
		SeparateStackEffect::start(data, is_new_effect);
		if (data && data->contains("damage"))
			if (auto* p = std::any_cast<int>(&(*data)["damage"])) damage_per_tick = *p;
	}

	void trigger() override {}

	std::string description() override {
		return "Poison: " + std::to_string(getStackCount()) + " stacks, "
			+ std::to_string(damage_per_tick) + " dmg/tick";
	}

	void update(int64_t delta_time) override {
		SeparateStackEffect::update(delta_time);
		if (getStackCount() > 0)
			std::cout << "  [PoisonTick] " << getStackCount() << " stacks → "
			<< damage_per_tick * getStackCount() << " dmg.\n";
	}

private:
	int damage_per_tick;
};