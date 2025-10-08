/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#ifdef ENABLE_SCRIPTING

    #include "ScMap.hpp"

    #include "../../../GameState.h"
    #include "../../../entity/Balloon.h"
    #include "../../../entity/Duck.h"
    #include "../../../entity/EntityList.h"
    #include "../../../entity/Fountain.h"
    #include "../../../entity/Guest.h"
    #include "../../../entity/Litter.h"
    #include "../../../entity/MoneyEffect.h"
    #include "../../../entity/Particle.h"
    #include "../../../entity/Staff.h"
    #include "../../../ride/Ride.h"
    #include "../../../ride/RideManager.hpp"
    #include "../../../ride/TrainManager.h"
    #include "../../Duktape.hpp"
    #include "../entity/ScBalloon.hpp"
    #include "../entity/ScEntity.hpp"
    #include "../entity/ScGuest.hpp"
    #include "../entity/ScLitter.hpp"
    #include "../entity/ScMoneyEffect.hpp"
    #include "../entity/ScParticle.hpp"
    #include "../entity/ScStaff.hpp"
    #include "../entity/ScVehicle.hpp"
    #include "../ride/ScRide.hpp"
    #include "../ride/ScTrackIterator.h"
    #include "../world/ScTile.hpp"

namespace OpenRCT2::Scripting
{
    ScMap::ScMap(duk_context* ctx)
        : _context(ctx)
    {
    }

    DukValue ScMap::size_get() const
    {
        return ToDuk(_context, getGameState().mapSize);
    }

    int32_t ScMap::numRides_get() const
    {
        auto& gameState = getGameState();
        return static_cast<int32_t>(RideManager(gameState).size());
    }

    int32_t ScMap::numEntities_get() const
    {
        return kMaxEntities;
    }

    std::vector<std::shared_ptr<ScRide>> ScMap::rides_get() const
    {
        std::vector<std::shared_ptr<ScRide>> result;

        auto& gameState = getGameState();
        auto rideManager = RideManager(gameState);
        for (const auto& ride : rideManager)
        {
            result.push_back(std::make_shared<ScRide>(ride.id));
        }
        return result;
    }

    std::shared_ptr<ScRide> ScMap::getRide(int32_t id) const
    {
        auto& gameState = getGameState();
        auto rideManager = RideManager(gameState);
        auto ride = rideManager[RideId::FromUnderlying(id)];
        if (ride != nullptr)
        {
            return std::make_shared<ScRide>(ride->id);
        }
        return {};
    }

    std::shared_ptr<ScTile> ScMap::getTile(int32_t x, int32_t y) const
    {
        auto coords = TileCoordsXY(x, y).ToCoordsXY();
        return std::make_shared<ScTile>(coords);
    }

    DukValue ScMap::getEntity(int32_t id) const
    {
        if (id >= 0 && id < kMaxEntities)
        {
            auto spriteId = EntityId::FromUnderlying(id);
            auto sprite = getGameState().entities.GetEntity(spriteId);
            if (sprite != nullptr && sprite->Type != EntityType::Null)
            {
                return GetEntityAsDukValue(sprite);
            }
        }
        duk_push_null(_context);
        return DukValue::take_from_stack(_context);
    }

    std::vector<DukValue> ScMap::getAllEntities(const std::string& type) const
    {
        std::vector<DukValue> result;
        if (type == "balloon")
        {
            for (auto sprite : EntityList<Balloon>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScBalloon>(sprite->Id)));
            }
        }
        else if (type == "car")
        {
            for (auto trainHead : TrainManager::View())
            {
                for (auto carId = trainHead->Id; !carId.IsNull();)
                {
                    auto car = getGameState().entities.GetEntity<Vehicle>(carId);

                    if (car == nullptr)
                    {
                        break;
                    }

                    result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScVehicle>(carId)));

                    // Prevent infinite loops: Ensure next_vehicle_on_train is valid and not self-referencing
                    auto nextCarId = car->next_vehicle_on_train;
                    if (nextCarId == carId)
                    {
                        break;
                    }

                    carId = nextCarId;
                }
            }
        }
        else if (type == "litter")
        {
            for (auto sprite : EntityList<Litter>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScLitter>(sprite->Id)));
            }
        }
        else if (type == "money_effect")
        {
            for (auto sprite : EntityList<MoneyEffect>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScMoneyEffect>(sprite->Id)));
            }
        }
        else if (type == "duck")
        {
            for (auto sprite : EntityList<Duck>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScEntity>(sprite->Id)));
            }
        }
        else if (type == "peep")
        {
            for (auto sprite : EntityList<Guest>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScGuest>(sprite->Id)));
            }
            for (auto sprite : EntityList<Staff>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScStaff>(sprite->Id)));
            }
        }
        else if (type == "guest")
        {
            for (auto sprite : EntityList<Guest>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScGuest>(sprite->Id)));
            }
        }
        else if (type == "staff")
        {
            for (auto sprite : EntityList<Staff>())
            {
                auto staff = getGameState().entities.GetEntity<Staff>(sprite->Id);
                if (staff != nullptr)
                {
                    switch (staff->AssignedStaffType)
                    {
                        case StaffType::Handyman:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScHandyman>(sprite->Id)));
                            break;
                        case StaffType::Mechanic:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScMechanic>(sprite->Id)));
                            break;
                        case StaffType::Security:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScSecurity>(sprite->Id)));
                            break;
                        default:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScStaff>(sprite->Id)));
                            break;
                    }
                }
                else
                {
                    result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScStaff>(sprite->Id)));
                }
            }
        }
        else if (type == "crashed_vehicle_particle")
        {
            for (auto sprite : EntityList<VehicleCrashParticle>())
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScCrashedVehicleParticle>(sprite->Id)));
            }
        }
        else
        {
            duk_error(_context, DUK_ERR_ERROR, "Invalid entity type.");
        }

        return result;
    }

    std::vector<DukValue> OpenRCT2::Scripting::ScMap::getAllEntitiesOnTile(
        const std::string& type, const DukValue& tilePos) const
    {
        // Get the tile position
        const auto pos = FromDuk<CoordsXY>(tilePos);

        // Declare a vector that will hold the result to return
        std::vector<DukValue> result;

        // Use EntityTileList to iterate all entities of the given type on the tile, and push them to result
        if (type == "balloon")
        {
            for (auto sprite : EntityTileList<Balloon>(pos))
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScBalloon>(sprite->Id)));
            }
        }
        else if (type == "car")
        {
            for (auto sprite : EntityTileList<Vehicle>(pos))
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScVehicle>(sprite->Id)));
            }
        }
        else if (type == "litter")
        {
            for (auto sprite : EntityTileList<Litter>(pos))
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScLitter>(sprite->Id)));
            }
        }
        else if (type == "duck")
        {
            for (auto sprite : EntityTileList<Duck>(pos))
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScEntity>(sprite->Id)));
            }
        }
        else if (type == "guest")
        {
            for (auto sprite : EntityTileList<Guest>(pos))
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScGuest>(sprite->Id)));
            }
        }
        else if (type == "money_effect")
        {
            for (auto sprite : EntityTileList<MoneyEffect>(pos))
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScMoneyEffect>(sprite->Id)));
            }
        }
        else if (type == "staff")
        {
            for (auto sprite : EntityTileList<Staff>(pos))
            {
                auto staff = getGameState().entities.GetEntity<Staff>(sprite->Id);
                if (staff != nullptr)
                {
                    switch (staff->AssignedStaffType)
                    {
                        case StaffType::Handyman:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScHandyman>(sprite->Id)));
                            break;
                        case StaffType::Mechanic:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScMechanic>(sprite->Id)));
                            break;
                        case StaffType::Security:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScSecurity>(sprite->Id)));
                            break;
                        default:
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScStaff>(sprite->Id)));
                            break;
                    }
                }
                else
                {
                    result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScStaff>(sprite->Id)));
                }
            }
        }
        else if (type == "crashed_vehicle_particle")
        {
            for (auto sprite : EntityTileList<VehicleCrashParticle>(pos))
            {
                result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScCrashedVehicleParticle>(sprite->Id)));
            }
        }
        else
        {
            // If the given type isn't valid, throw an error
            duk_error(_context, DUK_ERR_ERROR, "Invalid entity type: %s", type.c_str());
        }

        return result;
    }

    template<typename TEntityType, typename TScriptType>
    DukValue createEntityType(duk_context* ctx, const DukValue& initializer)
    {
        TEntityType* entity = getGameState().entities.CreateEntity<TEntityType>();
        if (entity == nullptr)
        {
            // Probably no more space for entities for this specified entity type.
            return ToDuk(ctx, undefined);
        }

        auto entityPos = CoordsXYZ{ AsOrDefault(initializer["x"], 0), AsOrDefault(initializer["y"], 0),
                                    AsOrDefault(initializer["z"], 0) };
        entity->MoveTo(entityPos);

        return GetObjectAsDukValue(ctx, std::make_shared<TScriptType>(entity->Id));
    }

    DukValue ScMap::createEntity(const std::string& type, const DukValue& initializer)
    {
        DukValue res;
        if (type == "car")
        {
            Vehicle* entity = getGameState().entities.CreateEntity<Vehicle>();
            if (entity == nullptr)
            {
                // Probably no more space for entities for this specified entity type.
                res = ToDuk(_context, undefined);
            }
            else
            {
                auto entityPos = CoordsXYZ{ AsOrDefault(initializer["x"], 0), AsOrDefault(initializer["y"], 0),
                                            AsOrDefault(initializer["z"], 0) };
                entity->MoveTo(entityPos);

                // Reset some important vehicle vars to their null values
                entity->sound1_id = OpenRCT2::Audio::SoundId::Null;
                entity->sound2_id = OpenRCT2::Audio::SoundId::Null;
                entity->next_vehicle_on_train = EntityId::GetNull();
                entity->scream_sound_id = OpenRCT2::Audio::SoundId::Null;
                for (size_t i = 0; i < std::size(entity->peep); i++)
                {
                    entity->peep[i] = EntityId::GetNull();
                }
                entity->BoatLocation.SetNull();

                res = GetObjectAsDukValue(_context, std::make_shared<ScVehicle>(entity->Id));
            }
        }
        else if (type == "staff")
        {
            res = createEntityType<Staff, ScStaff>(_context, initializer);
        }
        else if (type == "guest")
        {
            res = createEntityType<Guest, ScGuest>(_context, initializer);
        }
        else if (type == "steam_particle")
        {
            res = createEntityType<SteamParticle, ScEntity>(_context, initializer);
        }
        else if (type == "money_effect")
        {
            res = createEntityType<MoneyEffect, ScMoneyEffect>(_context, initializer);
        }
        else if (type == "crashed_vehicle_particle")
        {
            res = createEntityType<VehicleCrashParticle, ScCrashedVehicleParticle>(_context, initializer);
        }
        else if (type == "explosion_cloud")
        {
            res = createEntityType<ExplosionCloud, ScEntity>(_context, initializer);
        }
        else if (type == "crash_splash")
        {
            res = createEntityType<CrashSplashParticle, ScEntity>(_context, initializer);
        }
        else if (type == "explosion_flare")
        {
            res = createEntityType<ExplosionFlare, ScEntity>(_context, initializer);
        }
        else if (type == "balloon")
        {
            res = createEntityType<Balloon, ScBalloon>(_context, initializer);
        }
        else if (type == "duck")
        {
            res = createEntityType<Duck, ScEntity>(_context, initializer);
        }
        else if (type == "jumping_fountain")
        {
            res = createEntityType<JumpingFountain, ScEntity>(_context, initializer);
        }
        else if (type == "litter")
        {
            res = createEntityType<Litter, ScLitter>(_context, initializer);
        }
        else
        {
            duk_error(_context, DUK_ERR_ERROR, "Invalid entity type.");
        }

        return res;
    }

    DukValue ScMap::runEffect(uint8_t colour,int32_t pos_x,int32_t pos_y,int32_t pos_z,int32_t tan_x,int32_t tan_y,int32_t tan_z,const DukValue& effectDuk)
    {
    uint16_t id = FromDuk<uint16_t>(effectDuk["id"]);
    Effect* effect=getGameState().particles.GetEffect(EffectId::FromUnderlying(id));
    bool result=getGameState().particles.RunEffect(colour,Vec32(pos_x<<11,pos_y<<11,pos_z*1672),Vec32(tan_x,tan_y,tan_z).Normalize(),Vec32(1,0,0),effect->id);
    return ToDuk(_context,result);
    }

    static Effect* CreateEffectFromDukValue(const DukValue& initializer)
    {
    printf("Creating effect\n");
    Effect* effect=getGameState().particles.CreateEffect();
        if (effect == nullptr)return nullptr;
    printf("Got ID %d\n",effect->id.ToUnderlying());
    effect->flags=0;
        if(AsOrDefault(initializer["fixed"],0))effect->flags|=EFFECT_STATIC;
        if(AsOrDefault(initializer["inheritColour"],0))effect->flags|=EFFECT_INHERIT_COLOUR;
        if(AsOrDefault(initializer["randomizeColour"],0))effect->flags|=EFFECT_RANDOMIZE_COLOUR;
    effect->colours[0]=AsOrDefault(initializer["colour"],0);
    effect->spawnRate=AsOrDefault(initializer["spawnRate"],0);
        if(effect->spawnRate==0)
        {
        effect->flags|=EFFECT_HAS_COUNT;
        effect->spawnRate=AsOrDefault(initializer["count"],1);
        }
    effect->gravity=AsOrDefault(initializer["gravity"],7793);
    effect->mass=AsOrDefault(initializer["mass"],256);
    //TODO check that min is not greater than max
    effect->lifetimeMin=AsOrDefault(initializer["lifetimeMin"],0);
    effect->lifetimeMax=AsOrDefault(initializer["lifetimeMax"],0);
        if(effect->lifetimeMax==0)
        {    
        effect->lifetimeMin=AsOrDefault(initializer["lifetime"],0);
        effect->lifetimeMax=effect->lifetimeMin;
        }
    effect->startTime=AsOrDefault(initializer["startTime"],0);
    effect->endTime=AsOrDefault(initializer["endTime"],255);
    effect->startSize=AsOrDefault(initializer["startSize"],4);
    effect->endSize=AsOrDefault(initializer["endSize"],4);
    effect->relative=AsOrDefault(initializer["relative"],255);
    effect->brownianMotion=AsOrDefault(initializer["brownianMotion"],0);
    //TODO check that min is not greater than max
    effect->velocityMin=AsOrDefault(initializer["velocityMin"],0);
    effect->velocityMax=AsOrDefault(initializer["velocityMax"],0);
        if(effect->velocityMax==0)
        {    
        effect->velocityMin=AsOrDefault(initializer["velocity"],0);
        effect->velocityMax=effect->velocityMin;
        }
    //TODO check that min is not greater than max
    effect->sphereMin=AsOrDefault(initializer["sphereMin"],0);
    effect->sphereMax=AsOrDefault(initializer["sphereMax"],0);
        if(effect->sphereMax==0)
        {    
        effect->sphereMin=AsOrDefault(initializer["sphere"],0);
        effect->sphereMax=effect->sphereMin;
        }

    //Load children
    DukValue children=initializer["children"];
            if(children.is_array())
            {
	    std::vector<DukValue> childrenArray = children.as_array();
            Effect* prevChild=nullptr;
                for (size_t i=0;i<childrenArray.size();i++)
                {
		DukValue item=childrenArray[i];
                    if (item.type() == DukValue::Type::OBJECT)
                    {
                    Effect* child=CreateEffectFromDukValue(item);
                        if(child == nullptr)
                        {
                        //TODO raise error and free already created effects if it fails
                        printf("Failing because child failed to be created\n");
			return nullptr;
                        }
			if(prevChild==nullptr)effect->children=child;
			else
                        {
				if(prevChild==child)
				{
				printf("Epic fail\n");
				exit(0);
				return nullptr;
				}
                        prevChild->next=child;
    			printf("Set effect %d next pointer to %d\n",prevChild->id.ToUnderlying(),child->id.ToUnderlying());
                        }
                    prevChild=child;
                    }
		    else
                    {
                    //TODO raise error and free already created effects if it fails
                    printf("Failing because children contains an element which is not an object\n");
                    return nullptr;
                    }
                }
            }
    printf("Created effect with ID %d\n",effect->id.ToUnderlying());
    return effect;
    }

    DukValue ScMap::createEffect(const DukValue& initializer)
    {
        DukValue res;
	Effect* effect=CreateEffectFromDukValue(initializer);
        if (effect == nullptr)
        {
            return ToDuk(_context, undefined);
        }
        else
        {
            return GetObjectAsDukValue(_context, std::make_shared<ScEffect>(effect->id));
        }
    }

    DukValue ScMap::getTrackIterator(const DukValue& dukPosition, int32_t elementIndex) const
    {
        auto position = FromDuk<CoordsXY>(dukPosition);
        auto trackIterator = ScTrackIterator::FromElement(position, elementIndex);
        if (trackIterator == nullptr)
            return ToDuk(_context, undefined);

        return GetObjectAsDukValue(_context, trackIterator);
    }

    void ScMap::Register(duk_context* ctx)
    {
        dukglue_register_property(ctx, &ScMap::size_get, nullptr, "size");
        dukglue_register_property(ctx, &ScMap::numRides_get, nullptr, "numRides");
        dukglue_register_property(ctx, &ScMap::numEntities_get, nullptr, "numEntities");
        dukglue_register_property(ctx, &ScMap::rides_get, nullptr, "rides");
        dukglue_register_method(ctx, &ScMap::getRide, "getRide");
        dukglue_register_method(ctx, &ScMap::getTile, "getTile");
        dukglue_register_method(ctx, &ScMap::getEntity, "getEntity");
        dukglue_register_method(ctx, &ScMap::getAllEntities, "getAllEntities");
        dukglue_register_method(ctx, &ScMap::getAllEntitiesOnTile, "getAllEntitiesOnTile");
        dukglue_register_method(ctx, &ScMap::createEntity, "createEntity");
        dukglue_register_method(ctx, &ScMap::createEffect, "createEffect");
        dukglue_register_method(ctx, &ScMap::runEffect, "runEffect");
        dukglue_register_method(ctx, &ScMap::getTrackIterator, "getTrackIterator");
    }

    DukValue ScMap::GetEntityAsDukValue(const EntityBase* sprite) const
    {
        auto spriteId = sprite->Id;
        switch (sprite->Type)
        {
            case EntityType::Vehicle:
                return GetObjectAsDukValue(_context, std::make_shared<ScVehicle>(spriteId));
            case EntityType::Staff:
            {
                auto staff = getGameState().entities.GetEntity<Staff>(spriteId);
                if (staff != nullptr)
                {
                    switch (staff->AssignedStaffType)
                    {
                        case StaffType::Handyman:
                            return GetObjectAsDukValue(_context, std::make_shared<ScHandyman>(spriteId));
                        case StaffType::Mechanic:
                            return GetObjectAsDukValue(_context, std::make_shared<ScMechanic>(spriteId));
                        case StaffType::Security:
                            return GetObjectAsDukValue(_context, std::make_shared<ScSecurity>(spriteId));
                        default:
                            return GetObjectAsDukValue(_context, std::make_shared<ScStaff>(spriteId));
                    }
                }
                else
                {
                    return GetObjectAsDukValue(_context, std::make_shared<ScStaff>(spriteId));
                }
            }
            case EntityType::Guest:
                return GetObjectAsDukValue(_context, std::make_shared<ScGuest>(spriteId));
            case EntityType::Litter:
                return GetObjectAsDukValue(_context, std::make_shared<ScLitter>(spriteId));
            case EntityType::Balloon:
                return GetObjectAsDukValue(_context, std::make_shared<ScBalloon>(spriteId));
            case EntityType::MoneyEffect:
                return GetObjectAsDukValue(_context, std::make_shared<ScMoneyEffect>(spriteId));
            case EntityType::CrashedVehicleParticle:
                return GetObjectAsDukValue(_context, std::make_shared<ScCrashedVehicleParticle>(spriteId));
            default:
                return GetObjectAsDukValue(_context, std::make_shared<ScEntity>(spriteId));
        }
    }









} // namespace OpenRCT2::Scripting

#endif
