// Trigger.hpp
#pragma once
#include <vector>
#include <map>
#include <cstdint>
#include <algorithm>

class Effect;
using TriggerTiming = unsigned int;

namespace Timing {
	constexpr TriggerTiming ON_ATTACK = 1 << 0;
	constexpr TriggerTiming ON_DAMAGED = 1 << 1;
	constexpr TriggerTiming ON_HEAL = 1 << 2;
	constexpr TriggerTiming ON_TURN_START = 1 << 3;
	constexpr TriggerTiming ON_TURN_END = 1 << 4;
}

class TriggerSystem {
public:
	void registerEffect(TriggerTiming timing, Effect* effect) {
		auto it = listeners.find(timing);
		if (it == listeners.end()) listeners[timing] = { effect };
		else it->second.push_back(effect);
	}
	void unregisterEffect(Effect* effect) {
		for (auto& [t, vec] : listeners)
			vec.erase(std::remove(vec.begin(), vec.end(), effect), vec.end());
	}
	void trigger(TriggerTiming timing) {
		auto it = listeners.find(timing);
		if (it == listeners.end()) return;
		for (auto* eff : it->second) if (eff) eff->trigger();
	}
private:
	std::map<TriggerTiming, std::vector<Effect*>> listeners;
};