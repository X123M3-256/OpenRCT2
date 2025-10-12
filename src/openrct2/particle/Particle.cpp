/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/
#include "Particle.h"

#include "../GameState.h"
#include "../SpriteIds.h"
#include "../audio/Audio.h"
#include "../core/DataSerialiser.h"
#include "../paint/Paint.h"
#include "../profiling/Profiling.h"
#include "../ride/VehicleColour.h"
#include "../scenario/Scenario.h"
#include "../world/Map.h"
#include "../world/tile_element/SurfaceElement.h"
#include "../interface/Viewport.h"
#include "Particle.h"

#include <iterator>

using namespace OpenRCT2;

//TODO make invalidate function a member of ParticleList
//ideally we would invalidate twice
//Try building particle draw list with reduced size (8 byte) particles


enum
{
PARTICLE_DEAD=1,
PARTICLE_NEW=2
};


ParticleList::ParticleList()
{
//Initialize particle list
numParticles=0;
firstFreeIndex=0;

//Initialize effect list
   for(uint32_t i=0;i<=kMaxEffects;i++)effects[i].flags|=EFFECT_FREE;

//Initialize spatial index
std::fill(std::begin(gParticleSpatialIndex),std::end(gParticleSpatialIndex),0xFFFF);
}


//Effects

Effect* ParticleList::CreateEffect()
{
   for(uint32_t i=0;i<=kMaxEffects;i++)
   {
       if(effects[i].flags&EFFECT_FREE)
       {
       effects[i].flags=0;
       effects[i].id=EffectId::FromUnderlying(i);
       effects[i].children=nullptr;
       effects[i].next=nullptr;
       return &(effects[i]);
       }
   }
return nullptr;
}

Effect* ParticleList::GetEffect(EffectId id)
{
uint16_t index=id.ToUnderlying();
	if(index>=kMaxEffects)return nullptr;
	if(effects[index].flags&EFFECT_FREE)return nullptr;
return &(effects[index]);
}

bool ParticleList::RunEffect(uint8_t colour,Vec32 pos,Vec32 tangent,Vec32 normal,EffectId id)
{
//printf("Running effect ID %d\n",id.ToUnderlying());
//Check effect ID is valid
Effect* effect=GetEffect(id);
	if(effect==nullptr)return false;

CreateParticleWithEffect(colour,0,pos,Vec32(0,0,0),tangent.Normalize(),normal.Normalize(),tangent.Cross(normal), id);
RebuildSpatialIndex();
return true;
}

inline int32_t get_z(int32_t x)
{
return (static_cast<int64_t>(x)*627)>>20;
}

static CoordsXYZ ParticleLocation(Particle& particle)
{
const CoordsXYZ particlePos = {particle.position.x>>11,particle.position.y>>11,get_z(particle.position.z)};
return particlePos;
}




//Creates new particle somewhere in the list
uint16_t ParticleList::CreateParticle(uint16_t type,uint8_t colour,uint8_t frame,uint16_t lifetime,Vec32 pos,Vec32 vel,EffectId effect)
{
uint16_t particleIndex=0xFFFF;	//If there is free space at the end of the particle list, add it there to avoid searching for free index
	if(numParticles<kMaxParticles)
	{
	particleIndex=numParticles;
	numParticles++;
	}
	//If not, replace the first dead particle
	else if(firstFreeIndex!=0xFFFF)
	{
	particleIndex=firstFreeIndex;
	//Find next free index
		do
		{
		firstFreeIndex++;
		}while(firstFreeIndex<0xFFFF&&!(particles[firstFreeIndex].type&PARTICLE_DEAD));
	}
//If no free space, fail
	if(particleIndex==0xFFFF)return 0xFFFF;

//Write new particle
Particle& particle=particles[particleIndex];
particle.type=type|PARTICLE_NEW;
particle.lifetime=lifetime;
particle.ticks=0;
particle.frame=frame;
particle.colour=colour;
particle.position=pos;
particle.velocity=vel;
particle.effect=effect;
//Mark particle list dirty
//ParticleList::RebuildSpatialIndex();

return particleIndex;
}

//Kills the particle at index i. Does not decrement numParticles
void ParticleList::KillParticle(uint16_t i)
{
particles[i].type|=PARTICLE_DEAD;
	if(i<firstFreeIndex)firstFreeIndex=i;
}

uint16_t ParticleList::CreateParticleWithEffect(uint8_t colour,uint32_t u,Vec32 position,Vec32 velocity,Vec32 tangent,Vec32 normal,Vec32 binormal,EffectId effectId)
{
//PROFILED_FUNCTION();

Effect& effect=effects[effectId.ToUnderlying()];

//Compute child colour
    if(!(effect.flags&EFFECT_INHERIT_COLOUR))
    {
    int index=0;
	if(effect.flags&EFFECT_RANDOMIZE_COLOUR)index=ScenarioRandMax(effect.num_colours);
	else index=LerpShift(0,effect.num_colours-1,u,15); 
    colour=effect.colours[index];
    }
//Compute child lifetime
uint16_t lifetime=effect.lifetimeMin;
	if(effect.lifetimeMin!=effect.lifetimeMax)lifetime+=ScenarioRandMax(1+effect.lifetimeMax-effect.lifetimeMin);

//Compute initial frame
uint8_t frame=effect.startSize;

//Compute child velocity
velocity=velocity.ScaleShift(effect.relative,8);

	if(effect.velocityMax!=0)
	{
	int32_t velocityMag=effect.velocityMin;
            if(effect.velocityMin!=effect.velocityMax)velocityMag+=ScenarioRandMax(1+effect.velocityMax-effect.velocityMin);
	
            if(effect.elevationMin!=0||effect.elevationMax!=0||effect.elevationJitter!=0)
	    {
            Vec32 localVelocity=Vec32(0,0,velocityMag*256);

            int32_t angle=LerpShift(effect.elevationMin*128,effect.elevationMax*128,u,15);
                if(effect.elevationJitter!=0)angle+=static_cast<int32_t>(ScenarioRandMax(256*effect.elevationJitter+1))-128*effect.elevationJitter;
	    localVelocity=localVelocity.RotateX(angle);
            int32_t azumith=LerpShift(effect.azumithMin*128,effect.azumithMax*128,u,15);
                if(effect.azumithJitter!=0)azumith+=static_cast<int32_t>(ScenarioRandMax(256*effect.azumithJitter+1))-128*effect.azumithJitter;
	    localVelocity=localVelocity.RotateZ(azumith);

            velocity=velocity.Add(tangent.ScaleShift(localVelocity.z,15).Add(normal.ScaleShift(localVelocity.y,15).Add(binormal.ScaleShift(localVelocity.x,15))));
	    }
            else velocity=velocity.Add(tangent.ScaleShift(velocityMag,7));
        }

	if(effect.sphereMax!=0)
	{
	//Compute random spherical velocity vector
	uint16_t sphereMag=effect.sphereMin;
		if(effect.sphereMin!=effect.sphereMax)sphereMag+=ScenarioRandMax(1+effect.sphereMax-effect.sphereMin);
	Vec32 sphere=GetRandomDirection();
	velocity=velocity.Add(sphere.ScaleShift(sphereMag,7));
	}

uint16_t index=CreateParticle(0,colour,frame,lifetime,position,velocity,effectId);
	if(index==0xFFFF)return 0xFFFF;

return index;
}


//We divide each tick into 32768 sub-ticks, in order to permit particles to spawn more than one particle per tick	
const int32_t subTicksPerTick=32768;
const int32_t subTicksPerSpawnRate=subTicksPerTick/256;

void ParticleList::UpdateParticle(uint16_t i,uint16_t timestep)
{
//PROFILED_FUNCTION();

Particle& particle=particles[i];
Effect& effect=effects[particle.effect.ToUnderlying()];
    //Check if particles lifetime has expired
        if(particle.ticks>=particle.lifetime)
	{
	KillParticle(i);
	return;
	}

    //Calculate new velocity update
        if(!(effect.flags&EFFECT_STATIC))
        {
	Vec32 accel=Vec32(0,0,-effect.gravity);
        
        uint16_t mass=effect.mass;
        Vec32 wind=Vec32(0,63356/4,0);
        accel=accel.Add(particle.velocity.Add(wind).ScaleShift(-(mass),16));
        particle.velocity=particle.velocity.Add(accel);
        }

    //Calculate new size
    particle.frame=effect.startSize;
        if(particle.lifetime!=0)particle.frame+=((effect.endSize-effect.startSize)*particle.ticks+particle.lifetime/2)/particle.lifetime;

    Vec32 tangent=Vec32(0,0,32768);
    Vec32 normal=Vec32(32768,0,0);
    Vec32 binormal=Vec32(0,32768,0);
    bool tangent_computed=false;
    bool normal_computed=false;

    //Spawn children
    Effect* childEffect=effect.children;
        while(childEffect!=nullptr)
        {
	int32_t startTime=particle.lifetime*childEffect->startTime*subTicksPerSpawnRate;
	int32_t endTime=particle.lifetime*childEffect->endTime*subTicksPerSpawnRate;
        int32_t t=particle.ticks*subTicksPerTick-startTime;
            if(t+subTicksPerTick>=0&&t<=endTime-startTime&&childEffect->spawnRate!=0&&(!(childEffect->flags&EFFECT_HAS_COUNT)||childEffect->spawnRate!=1||startTime==endTime))
            {
            //If start and end times are the same then spawnRate is interpreted as a count of particles to spawn regardless of HAS_COUNT flag
            uint32_t countToSpawn=childEffect->spawnRate;
            bool noInterp=startTime==endTime;
            int32_t subTicksPerSpawn=0;
            uint32_t curSpawnCount=0;

            //Calculate spawn rate and the number of children previously spawned. If start time and end time are equal these values cannot be computed, but also aren't needed
                if(startTime!=endTime)
                {
                subTicksPerSpawn=childEffect->spawnRate*subTicksPerSpawnRate;
                    if(childEffect->flags&EFFECT_HAS_COUNT)subTicksPerSpawn=(endTime-startTime)/(childEffect->spawnRate-1);
                    if(subTicksPerSpawn!=0)
                    {
                    //Without 4.8ms
                    //Number of children which this particle has already spawned
                    std::div_t dv=std::div(t,subTicksPerSpawn);
                    curSpawnCount=dv.quot+(t>=0&&dv.rem!=0);
                        //if(t>=0&&dv.rem!=0)curSpawnCount++;
                    //Number of children which should have been spawned upon completion of this tick
                    dv=std::div(t+subTicksPerTick,subTicksPerSpawn);
                    uint32_t nextSpawnCount=dv.quot+(dv.rem!=0);//(t+subTicksPerTick)/subTicksPerSpawn;
                        //if(dv.rem!=0)nextSpawnCount++;
                        if((childEffect->flags&EFFECT_HAS_COUNT)&&nextSpawnCount>childEffect->spawnRate)nextSpawnCount=childEffect->spawnRate;
                    //Number of children to spawn this tick
                    countToSpawn=nextSpawnCount-curSpawnCount;
                    //printf("cSC %d\n nSC %d cTS %d t %d\n",curSpawnCount,nextSpawnCount,countToSpawn,t);
                    }else noInterp=true;
                }

	        for(uint32_t j=0;j<countToSpawn;j++)
       	        {
                Vec32 pos=particle.position;
                uint32_t u=0;
		uint16_t sub_tick=0;
                    //Interpolate between current and next position
                    if(!noInterp)
                    {
                    //Determine sub-tick at which this particle should spawn
                    sub_tick=(curSpawnCount+j)*subTicksPerSpawn-t;
                    //Determine time at which this particle spawns (relative to startTime)
                    int32_t sub_t=t+sub_tick;
	        	if(sub_t<0||sub_t>endTime-startTime)continue;
                    //Determine position of parent at spawn time
                    pos=pos.Add(particle.velocity.Scale(sub_tick,40*subTicksPerTick));
                    //Determine fraction of total emission duration elapsed
                    u=static_cast<uint32_t>((32768*static_cast<uint64_t>(t+sub_tick)+16384)/(endTime-startTime));
                    }
                //Spawn child particle
                    if(!tangent_computed&&childEffect->velocityMax!=0)
		    {
		        if(particle.velocity.x!=0||particle.velocity.y!=0||particle.velocity.z!=0)tangent=particle.velocity.Normalize();
		    tangent_computed=true;
		    }
                    if(!normal_computed&&childEffect->velocityMax!=0&&(childEffect->elevationMin!=0||childEffect->elevationMax!=0||childEffect->elevationJitter!=0))
		    {
		        if(tangent.x!=0||tangent.y!=0)normal=tangent.Cross(Vec32(0,0,1)).Normalize();
                    binormal=tangent.Cross(normal).ScaleShift(1,15);
		    normal_computed=true;
		    }
                uint16_t newIndex=CreateParticleWithEffect(particle.colour,u,pos,particle.velocity,tangent,normal,binormal,childEffect->id);
                UpdateParticle(newIndex,subTicksPerTick-sub_tick);
                }
            }
        childEffect=childEffect->next;
        }
    //Position update
        if(!(effect.flags&EFFECT_STATIC))
        {
        //particle.position=particle.position.Add(particle.velocity.Scale(timestep,40*subTicksPerTick));
        particle.position=particle.position.Add(particle.velocity.ScaleShift(timestep,20));
        //particle.position=particle.position.Add(Vec32((static_cast<int64_t>(particle.velocity.x)*timestep)>>20,(static_cast<int64_t>(particle.velocity.y)*timestep)>>20,(static_cast<int64_t>(particle.velocity.z)*timestep)>>20));

            if(effect.brownianMotion!=0)
            {
            //To beat 6.7ms
            //To avoid cost of excessive calls to ScenarioRand we replace it with a simple XOR based hash function here
            uint32_t x=i^particle.ticks^static_cast<uint32_t>(particle.position.z);
            x^=x<<13;
            x^=x>>17;
            x^=x<<5;
            int32_t x_rand=((x&0x1F)-16); 
            int32_t y_rand=((x&0x1F00)>>8)-16;
            int32_t z_rand=((x&0x1F0000)>>16)-16;
            int32_t num=effect.brownianMotion*timestep;
	    //printf("%d\n",(num*z_rand)>>15);
            Vec32 disp=Vec32((num*x_rand)>>15,(num*y_rand)>>15,(num*z_rand)>>15);
            particle.position=particle.position.Add(disp);
            }
        }

    particle.ticks++;


    // Check collision with land / water TODO do this in separate pass it can be made more efficient

}


void ParticleList::Invalidate()
{
PROFILED_FUNCTION();
uint32_t i=0;
    while(i<numParticles)
    {
    const CoordsXYZ particlePos = ParticleLocation(particles[i]);
    int tileX=particlePos.x>>5;
    int tileY=particlePos.y>>5;
        do
	{
	i++;
	}while(i<numParticles&&particles[i].position.x>>16==tileX&&particles[i].position.y>>16==tileY);
    MapInvalidateTile({ particlePos.x&0xFFFFE0,particlePos.y&0xFFFFE0, particlePos.z, get_z(particles[i-1].position.z)});
        if(i==numParticles)break;
    }
}

void ParticleList::DetectCollisions()
{
uint32_t i=0;
    while(i<numParticles)
    {
    //Check first particle on tile is within map bounds and not below ground
    const CoordsXYZ particlePos = ParticleLocation(particles[i]);

    int tileX=particlePos.x>>5;
    int tileY=particlePos.y>>5;

    //If tile out of map bounds kill everything on this tile
        if(tileX<0||tileX>=kMaximumMapSizeTechnical||tileY<0||tileY>=kMaximumMapSizeTechnical)
        {
            do
            {
            KillParticle(i);
	    i++;
            }while(i<numParticles&&particles[i].position.x>>16==tileX&&particles[i].position.y>>16==tileY);
        gParticleSpatialIndex[tileY*kMaximumMapSizeTechnical+tileX]=0xFFFF;
        continue;
	}

    //Kill all particles that are below terrain height
    int16_t minZ = TileElementHeight(particlePos);
    int16_t waterZ = TileElementWaterHeight(particlePos);
        if(waterZ!=0)minZ=waterZ;

        do
	{
            if(get_z(particles[i].position.z)<=minZ)
            {
            KillParticle(i);
            gParticleSpatialIndex[tileY*kMaximumMapSizeTechnical+tileX]++;
            }else break;
	i++;
	}while(i<numParticles&&particles[i].position.x>>16==tileX&&particles[i].position.y>>16==tileY);
    //Advance to next tile
        while(i<numParticles&&particles[i].position.x>>16==tileX&&particles[i].position.y>>16==tileY)i++;
    }
}

void ParticleList::Update()
{
PROFILED_FUNCTION();
Invalidate();
/*
printf("Printing effect list\n");
   for(uint32_t i=0;i<=kMaxEffects;i++)
   {
       if(!(effects[i].flags&EFFECT_FREE))
       {
           if(effects[i].children==nullptr)
           {
               if(effects[i].next==nullptr)printf("Effect ID %d null Children null Next null\n",i);
               else printf("Effect ID %d Children null Next %d\n",i,effects[i].next->id.ToUnderlying());
           }
           else
           {
               if(effects[i].next==nullptr)printf("Effect ID %d Children %d Next null\n",i,effects[i].children->id.ToUnderlying());
               else printf("Effect ID %d Children %d Next %d\n",i,effects[i].children->id.ToUnderlying(),effects[i].next->id.ToUnderlying());
           }
       }
   }*/
uint32_t n=numParticles;
    for (uint32_t i=0;i<n;i++)
    {
        //Do not re-update particles that were added during update
	if(particles[i].type&PARTICLE_NEW)continue;
    UpdateParticle(i,subTicksPerTick);
    }

RebuildSpatialIndex();
DetectCollisions();
Invalidate();
printf("Total particles %d\n",numParticles);
}



//Sort particles by the tile they are on and then by Z height
//TODO work out WTF this means
struct compare {
    bool operator()(const Particle& a, const Particle& b) const 
	{
		if(a.type&PARTICLE_DEAD)return false;
		else if(b.type&PARTICLE_DEAD) return true;
        int aX=a.position.x>>16;
	int bX=b.position.x>>16;
		if(aX==bX)
		{
		int aY=a.position.y>>16;
		int bY=b.position.y>>16;
			if(aY==bY)
			{
			return a.position.z<b.position.z;
			}else return aY<bY;
		}else return aX<bX;
    	}
};

void ParticleList::RebuildSpatialIndex()
{
PROFILED_FUNCTION();

//Sort particle list by tile
std::sort(particles.begin(),particles.begin()+numParticles,compare());

//Build spatial index
//printf("Building index\n");
std::fill(std::begin(gParticleSpatialIndex),std::end(gParticleSpatialIndex),0xFFFF);
uint32_t i=0;
    while(i<numParticles&&!(particles[i].type&PARTICLE_DEAD))
    {
    int tileX=particles[i].position.x>>16;
    int tileY=particles[i].position.y>>16;

    gParticleSpatialIndex[tileY*kMaximumMapSizeTechnical+tileX]=i;

    //printf("Adding on tile %d %d\n",tileX,tileY);
        do
	{
	particles[i].type&=~PARTICLE_NEW;
	i++;
	}while(i<particles.size()&&!(particles[i].type&PARTICLE_DEAD)&&particles[i].position.x>>16==tileX&&particles[i].position.y>>16==tileY);
    }
numParticles=i;
/*
printf("Final list post rebuild\n");
    for(int j=0;j<numParticles;j++)
    {
    printf("X %d Y %d flags %d\n",particles[j].position.x>>11,particles[j].position.y>>11,particles[j].type&PARTICLE_DEAD);
    }
*/
}





static BoundBoxXYZ GetBB(CoordsXY pos,CoordsXYZ particlePos,uint32_t dir,int32_t minZ,int32_t maxZ)
{
BoundBoxXYZ bb={{0,0,0},{0,0,0}};
    switch(dir)
    {
    case 0:	
        bb={{2+pos.x-particlePos.x,2+pos.y-particlePos.y,minZ},{ 28, 28,maxZ-minZ}};
        break;
    case 1:	
        bb={{2+pos.y-particlePos.y,2+(particlePos.x-pos.x)-32,minZ},{ 28, 28,maxZ-minZ}};
        break;
    case 2:	
        bb={{(2+particlePos.x-pos.x)-32,2+(particlePos.y-pos.y)-32,minZ},{ 28, 28,maxZ-minZ}};
        break;
    case 3:	
        bb={{(2+particlePos.y-pos.y)-32,2+pos.x-particlePos.x,minZ},{ 28, 28,maxZ-minZ}};
        break;
    }
return bb;
}

void ParticleList::ParticlePaintSetup(PaintSession& session, const CoordsXY& pos,int8_t side)
{
PROFILED_FUNCTION();


    if (!MapIsLocationValid(pos))
    {
        return;
    }

//Find particles for this tile
    int tileX=pos.x>>5;
    int tileY=pos.y>>5;
    uint16_t firstParticle=gParticleSpatialIndex[tileY*kMaximumMapSizeTechnical+tileX];
	if(firstParticle==0xFFFF)return;//There are no particles on this tile

//printf("Paint setup tile %d %d side %d\n",pos.x/32,pos.y/32,side);




//Calculate min and max Z
int32_t minZ=get_z(particles[firstParticle].position.z);
int32_t maxZ=0;
    for (uint32_t i=firstParticle;i<particles.size();i++)
    {
    const CoordsXYZ particlePos = ParticleLocation(particles[i]);
	    if((particlePos.x>>5)!=tileX||(particlePos.y>>5)!=tileY)break;
            maxZ=particlePos.z;
    }


ScreenCoordsXY screenOrigin;
bool firstParticleDrawn=false;
    for (uint32_t i=firstParticle;i<particles.size();i++)
    {
    const CoordsXYZ particlePos = ParticleLocation(particles[i]);
	if(particles[i].type&PARTICLE_DEAD||particlePos.x>>5!=tileX||particlePos.y>>5!=tileY)break;
        if(particles[i].frame==0)continue;//A particle of zero size has no sprite

    CoordsXY tilePos={(particlePos.x&0x1F)-16,(particlePos.y&0x1F)-16};
    tilePos=tilePos.Rotate(session.CurrentRotation+(side&2));
    int highlight=tilePos.y-tilePos.x>((side&1)<<5)-((particles[i].frame)/2);
        if(!highlight)continue;
    //printf("%d %d %d\n",(particlePos.x-103)/8,(particlePos.y-101)/8,tilePos.y-tilePos.x);


    //printf("Drawing particle %d at %d %d %d\n",i,particlePos.x,particlePos.y,particlePos.z);
    session.SpritePosition.x = particlePos.x;
    session.SpritePosition.y = particlePos.y;
    session.InteractionType = ViewportInteractionItem::none;
    ScreenCoordsXY screenCoords = Translate3DTo2DWithZ(session.CurrentRotation, particlePos);
    uint32_t imageId = SPR_G2_PARTICLE_STAR + (particles[i].frame-1);
    auto image = ImageId(imageId, particles[i].colour, particles[i].colour);
	if(!firstParticleDrawn)
	{
        screenOrigin =screenCoords;
	PaintAddImageAsParent(session, image, { 0, 0, particlePos.z }, GetBB(pos,particlePos,session.CurrentRotation,minZ,maxZ));
        firstParticleDrawn=true;
	}
	//else PaintAddImageAsChild(session, image, { 0, 0, particlePos.z }, GetBB(pos,particlePos,session.CurrentRotation,minZ,maxZ));
	else PaintAttachToPreviousAttach(session, image,screenCoords.x-screenOrigin.x,screenCoords.y-screenOrigin.y);
    }
}

/*
        // Only paint sprites that are below the clip height and inside the clip selection.
        // Here converting from land/path/etc height scale to pixel height scale.
        // Note: peeps/scenery on slopes will be above the base
        // height of the slope element, and consequently clipped.
        if (session.ViewFlags & VIEWPORT_FLAG_CLIP_VIEW)
        {
            if (particlePos.z > (gClipHeight * kCoordsZStep))
            {
                // see-through off: don't paint this particle at all
                // see-through on: paint this particle as partial or hidden later on
                if ((session.ViewFlags & VIEWPORT_FLAG_CLIP_VIEW_SEE_THROUGH) == 0)
                {
                    continue;
                }
            }
            if (particlePos.x < gClipSelectionA.x || particlePos.x > (gClipSelectionB.x + kCoordsXYStep - 1))
            {
                continue;
            }
            if (particlePos.y < gClipSelectionA.y || particlePos.y > (gClipSelectionB.y + kCoordsXYStep - 1))
            {
                continue;
            }
        }


        //ScreenCoordsXY screenCoords = Translate3DTo2DWithZ(session.CurrentRotation, particlePos);
        auto spriteRect = ScreenRect(
            screenCoords - ScreenCoordsXY{ particle->SpriteData.Width, particle->SpriteData.HeightMin },
            screenCoords + ScreenCoordsXY{ particle->SpriteData.Width, particle->SpriteData.HeightMax });

        const ZoomLevel zoom = session.DPI.zoom_level;
        if (session.DPI.y + session.DPI.height <= zoom.ApplyInversedTo(spriteRect.GetTop())
            || zoom.ApplyInversedTo(spriteRect.GetBottom()) <= session.DPI.y
            || session.DPI.x + session.DPI.width <= zoom.ApplyInversedTo(spriteRect.GetLeft())
            || zoom.ApplyInversedTo(spriteRect.GetRight()) <= session.DPI.x)
        {
            continue;
        }


        //session.CurrentlyDrawnEntity = particle;
*/

