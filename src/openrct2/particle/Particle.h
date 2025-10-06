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
};

namespace OpenRCT2
{
const uint32_t kMaxParticles=65535;
const uint32_t kMaxEffects=1024;
constexpr const uint32_t kParticleSpatialIndexSize = (kMaximumMapSizeTechnical * kMaximumMapSizeTechnical);

    struct Effect
    {
    EffectId id;
    uint16_t flags;
    uint8_t colours[8];
    uint8_t num_colours;
    uint8_t type;
    uint16_t lifetimeMin;
    uint16_t lifetimeMax;//16 bytes

    uint16_t spawnRate;

    uint8_t startTime;
    uint8_t endTime;
    uint8_t startSize;
    uint8_t endSize;
    uint16_t mass;
    uint16_t gravity; //8 bytes

    int16_t velocityMin;
    int16_t velocityMax;
  
    uint16_t sphereMin;
    uint16_t sphereMax;
    uint16_t circleMin;
    uint16_t circleMax;//8 bytes

    uint8_t relative;
    uint8_t brownianMotion;

    Effect* children;
    Effect* next;
    };


    struct Particle
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
        void CreateChildParticle(Particle& particle,Vec32 pos,Effect& effect);
        void KillParticle(uint16_t i);
        void RebuildSpatialIndex();
    public:
	ParticleList();
        bool CreateParticle(uint16_t type,uint16_t colour,uint16_t lifetime,int32_t pos_x,int32_t pos_y,int32_t pos_z,int32_t vel_x,int32_t vel_y,int32_t vel_z,EffectId effect);
        Effect* CreateEffect();
        Effect* GetEffect(EffectId);
	void Update();
	void ParticlePaintSetup(PaintSession& session, const CoordsXY& pos);
    };

} // namespace OpenRCT2




