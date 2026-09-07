#include "SpawnerMod.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Symbol.h"
#include "ll/api/memory/Signature.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/io/Logger.h"

#include "mc/world/actor/Mob.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Spawner.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"

#include <string>
#include <regex>

namespace SpawnerSetting {

namespace {

void applyDensityMultiplier(Dimension* dim) {
    if (!dim) return;

    auto const& config     = SpawnerMod::getInstance().getConfig();
    float       multiplier = config.densityMultiplier;
    auto&       logger     = SpawnerMod::getInstance().getSelf().getLogger();

    if (multiplier == 1.0f) return;

    float originalVal = dim->mMobsPerChunkSurface[0];

    for (float& val : dim->mMobsPerChunkSurface) {
        val *= multiplier;
    }
    for (float& val : dim->mMobsPerChunkUnderground) {
        val *= multiplier;
    }

    logger.info(
        "维度 ID: {} | 密度倍率: {:.1f} | 地表密度上限: {:.1f} -> {:.1f}",
        static_cast<int>(dim->getDimensionId()),
        multiplier,
        originalVal,
        dim->mMobsPerChunkSurface[0]
    );
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    DimensionInitHook,
    ll::memory::HookPriority::Normal,
    Dimension,
    &Dimension::$init,
    void,
    ::br::worldgen::StructureSetRegistry const& structureSetRegistry
) {
    origin(structureSetRegistry);
    applyDensityMultiplier(this);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    CheckSpawnRulesHook,
    ll::memory::HookPriority::Normal,
    Mob,
    &Mob::$checkSpawnRules,
    bool,
    bool checkSpawnPosition
) {
    auto const& config = SpawnerMod::getInstance().getConfig();

    bool isFamilyMatch = false;
    bool isIdMatch     = false;

    if (config.enableFamilyFilter) {
        for (auto const& familyName : config.targetFamilies) {
            if (this->hasFamily(HashedString(familyName.c_str()))) {
                isFamilyMatch = true;
                break;
            }
        }
    }

    if (config.enableIdentifierFilter) {
        std::string const& myId = (std::string const&)this->getActorIdentifier().mFullName;
        for (auto const& targetId : config.targetMonsterIds) {
            if (config.useRegex) {
                try {
                    std::regex pattern(targetId);
                    if (std::regex_search(myId, pattern)) {
                        isIdMatch = true;
                        break;
                    }
                } catch (...) {
                }
            } else {
                if (myId == targetId) {
                    isIdMatch = true;
                    break;
                }
            }
        }
    }

    bool isTarget = isFamilyMatch || isIdMatch;

    if (config.whitelistMode) {
        if (!isTarget) return false;
    } else {
        if (isTarget) return false;
    }

    return origin(checkSpawnPosition);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    LevelTickHook,
    ll::memory::HookPriority::Normal,
    Level,
    &Level::$tick,
    void
) {
    origin();

    auto& spawnerRef = this->getSpawner();
    auto* spawner = &spawnerRef;
    if (!spawner) return;

    auto const& config = SpawnerMod::getInstance().getConfig();
    float multiplier = config.globalCapMultiplier;
    if (multiplier <= 0.0f || multiplier == 1.0f) return;

    auto& mobCount = ll::memory::dAccess<unsigned int>(spawner, config.mobCountOffset);
    if (mobCount > 0) {
        mobCount = static_cast<unsigned int>(static_cast<float>(mobCount) / multiplier);
    }
}

} // namespace
} // namespace SpawnerSetting
