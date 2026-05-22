// EffectTarget.hpp
#pragma once

#include "Effect.hpp"
#include "Trigger.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

class EffectTarget {
protected:
	std::map<std::string, std::unique_ptr<Effect>> effects;
	std::vector<Effect*> updateList;
	TriggerSystem eventSystem;

	void registerEffectInternal(Effect* eff) {
		if (eff->needsUpdate()) updateList.push_back(eff);
		uint32_t timings = eff->getTriggerTimings();
		for (uint32_t bit = 1; bit != 0; bit <<= 1)
			if (timings & bit) eventSystem.registerEffect(bit, eff);
	}
	void unregisterEffectInternal(Effect* eff) {
		if (eff->needsUpdate())
			updateList.erase(std::remove(updateList.begin(), updateList.end(), eff), updateList.end());
		eventSystem.unregisterEffect(eff);
	}

public:
	EffectTarget() = default;
	virtual ~EffectTarget() = default;
	EffectTarget(const EffectTarget&) = delete;
	EffectTarget& operator=(const EffectTarget&) = delete;

	template <typename EffectType, typename... Args>
		requires std::derived_from<EffectType, Effect>
	void addEffect(EffectData* data, const std::string& name, Args&&... args) {
		auto it = effects.find(name);
		if (it != effects.end()) {
			it->second->start(data, false);
		}
		else {
			auto eff = std::make_unique<EffectType>(this, name, std::forward<Args>(args)...);
			eff->start(data, true);
			registerEffectInternal(eff.get());
			effects.emplace(name, std::move(eff));
		}
	}

	void removeEffect(const std::string& name) {
		auto it = effects.find(name);
		if (it != effects.end() && it->second->end()) {
			unregisterEffectInternal(it->second.get());
			effects.erase(it);
		}
	}

	void updateAllEffects(int64_t deltaTime) {
		std::vector<std::string> toRemove;
		for (auto* eff : updateList) {
			eff->update(deltaTime);
			if (eff->getLastDuration() <= 0 && eff->end()) {
				toRemove.push_back(eff->getName());
			}
		}
		for (const auto& name : toRemove) removeEffect(name);
	}

	void triggerEvent(TriggerTiming timing) { eventSystem.trigger(timing); }
	bool hasEffect(const std::string& name) const { return effects.contains(name); }
	Effect* getEffect(const std::string& name) const {
		auto it = effects.find(name);
		return (it != effects.end()) ? it->second.get() : nullptr;
	}
	const auto& getAllEffects() const { return effects; }
	template <typename T> T* getEffectAs(const std::string& name) const
		requires std::derived_from<T, Effect>
	{ return dynamic_cast<T*>(getEffect(name)); }
};

using EffectOwner = EffectTarget;