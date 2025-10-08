/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once
#include "../paint/Paint.h"
#include "../Identifiers.h"

#include <cstdint>

enum
{
PARTICLE_TYPE_STAR=0,
PARTICLE_TYPE_SMOKE=1,
};

enum
{
EFFECT_FREE=1,
EFFECT_STATIC=2,
EFFECT_INHERIT_COLOUR=4,
EFFECT_RANDOMIZE_COLOUR=8,
EFFECT_HAS_COUNT=16,
EFFECT_REQUIRES_TANGENT=32,
EFFECT_REQUIRES_NORMAL=64
};

struct Vec32
{
int32_t x;
int32_t y;
int32_t z;
public:
    Vec32();
    Vec32(int32_t x,int32_t y,int32_t z);
    Vec32 Scale(int32_t num,int32_t denom);
    Vec32 Add(Vec32 v);
    Vec32 Normalize();
};

namespace OpenRCT2
{
const uint32_t kMaxParticles=65535;
const uint32_t kMaxEffects=1024;
constexpr const uint32_t kParticleSpatialIndexSize = (kMaximumMapSizeTechnical * kMaximumMapSizeTechnical);

    struct Effect //Total size 72 bytes
    {
    EffectId id;
    uint16_t flags;
    uint8_t type;
    uint8_t num_colours;
    uint8_t colours[8];
    uint16_t mass; //16 bytes

    uint16_t startTime;
    uint16_t endTime;
    uint16_t lifetimeMin;
    uint16_t lifetimeMax;
    uint16_t spawnRate;
    uint16_t gravity;
    uint16_t relative;
    uint8_t startSize;
    uint8_t endSize; //16 bytes

    int32_t velocityMin;
    int32_t velocityMax; //8 bytes
  
    uint16_t sphereMin;
    uint16_t sphereMax;
    int16_t azumithMin;
    int16_t azumithMax;
    int16_t elevationMin;
    int16_t elevationMax;
    uint8_t elevationJitter;
    uint8_t azumithJitter;
    uint16_t brownianMotion; //16 bytes

    Effect* children;
    Effect* next; //16 bytes
    };

    struct Particle //Total size 40 bytes
    {
        uint8_t flags;
        uint8_t type;
        uint8_t colour;
        uint8_t frame;
        uint16_t ticks;
        uint16_t lifetime;
        Vec32 position;
        Vec32 velocity;
	Effect* effect;
    };

    class ParticleList
    {
    private:
	uint16_t numParticles;
	uint16_t firstFreeIndex;
        bool particleListDirty;
        std::array<Particle, kMaxParticles> particles;
        std::array<uint16_t, kParticleSpatialIndexSize> gParticleSpatialIndex;
        std::array<Effect, kMaxEffects> effects;
        bool CreateParticle(uint16_t type,uint8_t colour,uint8_t frame,uint16_t lifetime,Vec32 pos,Vec32 vel,Effect& effect);
        void CreateParticleWithEffect(uint8_t color,Vec32 pos,Vec32 vel,Vec32 tangent,Vec32 normal,Effect& effect);
        void KillParticle(uint16_t i);
        void RebuildSpatialIndex();
    public:
	ParticleList();
        Effect* CreateEffect();
        Effect* GetEffect(EffectId);
        bool RunEffect(uint8_t color,Vec32 pos,Vec32 tangent,Vec32 normal,EffectId);
	void Update();
	void ParticlePaintSetup(PaintSession& session, const CoordsXY& pos);
    };

} // namespace OpenRCT2




