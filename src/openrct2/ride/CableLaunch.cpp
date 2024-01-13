/*****************************************************************************
 * Copyright (c) 2014-2023 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "CableLaunch.h"

#include "../entity/EntityList.h"
#include "../rct12/RCT12.h"
#include "../world/tile_element/TileElement.h"
#include "../world/tile_element/TrackElement.h"
#include "Ride.h"
#include "RideData.h"
#include "Track.h"
#include "Vehicle.h"
#include "VehicleData.h"

#include <algorithm>

using namespace OpenRCT2;

void Vehicle::CableLaunchUpdate()
{
    switch (status)
    {
        case Vehicle::Status::MovingToEndOfStation:
            CableLaunchUpdateMovingToEndOfStation();
            break;
        case Vehicle::Status::WaitingForPassengers:
            // Stays in this state until a train puts it into next state
            break;
        case Vehicle::Status::WaitingToDepart:
            CableLaunchUpdateWaitingToDepart();
            break;
        case Vehicle::Status::Departing:
            CableLaunchUpdateDeparting();
            break;
        case Vehicle::Status::Travelling:
            CableLaunchUpdateTravelling();
            break;
        case Vehicle::Status::Arriving:
            CableLaunchUpdateArriving();
            break;
            //    case Vehicle::Status::PreparingForCableLaunch:
            //        CableLaunchUpdatePreparingForCableLaunch();
            // break;
        default:
            break;
    }
}

void Vehicle::CableLaunchUpdateMovingToEndOfStation()
{
    // Wait a few seconds before returning
    if (sub_state != 0 && sub_state != 127)
    {
        sub_state++;
        return;
    }

    if (velocity >= -300000) // TODO determine final speed
    {
        acceleration = -2932;
    }

    if (velocity < -300000)
    {
        velocity -= velocity / 16;
        acceleration = 0;
    }

    if (!(CableLiftUpdateTrackMotion() & VEHICLE_UPDATE_MOTION_TRACK_FLAG_VEHICLE_AT_STATION))
        return;

    SetState(Vehicle::Status::Arriving, 0);
}

void Vehicle::CableLaunchUpdateArriving()
{
    // Slow catch car down
    if (velocity < -50000)
    {
        acceleration = 10000;
    }

    // Continue moving until it is in the right position

    CableLiftUpdateTrackMotion();

    if (track_progress == 3)
    {
        velocity = 0;
        acceleration = 0;
        SetState(Vehicle::Status::WaitingForPassengers, sub_state);
    }
    //    sub_state++;
    //    if (sub_state >= 64)
    //        SetState(Vehicle::Status::MovingToEndOfStation, sub_state);
}

static TileElement* CableLaunchGetTileElement(const Ride& ride)
{
    TileElement* cableLaunchTileElement = MapGetFirstElementAt(ride.cableLiftLoc);
    if (cableLaunchTileElement == nullptr)
        return nullptr;

    do
    {
        if (cableLaunchTileElement->GetType() != TileElementType::Track)
            continue;
        if (cableLaunchTileElement->GetBaseZ() != ride.cableLiftLoc.z)
            continue;

        return cableLaunchTileElement;
    } while (!(cableLaunchTileElement++)->IsLastForTile());

    return nullptr;
}

static void CableLaunchUpdateFinStateForTile(CoordsXYZ loc, TrackElement* tileElement, bool set_lowered)
{
    uint8_t state = tileElement->GetCableLaunchFinState();
    uint8_t new_state = state;
    // TODO misses tiles when train is going faster than 230mph - code should not allow tiles to be set in a state lower than
    // the tile ahead
    switch (state)
    {
        case CABLE_LAUNCH_FIN_STATE_RAISING_0:
            new_state = CABLE_LAUNCH_FIN_STATE_RAISING_1;
            break;
        case CABLE_LAUNCH_FIN_STATE_RAISING_1:
            new_state = CABLE_LAUNCH_FIN_STATE_RAISING_2;
            break;
        case CABLE_LAUNCH_FIN_STATE_RAISING_2:
            new_state = CABLE_LAUNCH_FIN_STATE_RAISING_3;
            break;
        case CABLE_LAUNCH_FIN_STATE_RAISING_3:
            new_state = CABLE_LAUNCH_FIN_STATE_RAISED;
            break;
        case CABLE_LAUNCH_FIN_STATE_LOWERING_0:
            new_state = CABLE_LAUNCH_FIN_STATE_LOWERING_1;
            break;
        case CABLE_LAUNCH_FIN_STATE_LOWERING_1:
            new_state = CABLE_LAUNCH_FIN_STATE_LOWERING_2;
            break;
        case CABLE_LAUNCH_FIN_STATE_LOWERING_2:
            new_state = CABLE_LAUNCH_FIN_STATE_LOWERED;
            break;
        default:
            break;
    }
    if (set_lowered)
        new_state = CABLE_LAUNCH_FIN_STATE_LOWERING_0;
    if (new_state != state)
    {
        tileElement->SetCableLaunchFinState(new_state);
        MapInvalidateElement(loc, reinterpret_cast<TileElement*>(tileElement));
    }
}

static void CableLaunchUpdateFinState(const Ride& ride, bool set_lowered)
{
    TileElement* cableLaunchTileElement = CableLaunchGetTileElement(ride);

    // First launch tile
    CableLaunchUpdateFinStateForTile(ride.cableLiftLoc, cableLaunchTileElement->AsTrack(), set_lowered);

    // Second launch tile
    auto type = cableLaunchTileElement->AsTrack()->GetTrackType();
    uint8_t rotation = cableLaunchTileElement->GetDirection();
    CoordsXY offsets = { -32, 0 };
    CoordsXYZD elem = { ride.cableLiftLoc.x, ride.cableLiftLoc.y, ride.cableLiftLoc.z, rotation };
    elem += offsets.Rotate(rotation);
    TrackElement* trackElement = MapGetTrackElementAtOfTypeSeq(elem, type, 1);
    if (trackElement != nullptr)
    {
        CableLaunchUpdateFinStateForTile({ elem.x, elem.y, elem.z }, trackElement, set_lowered);
    }

    // Rest of the tiles
    bool success = false;
    TrackCircuitIterator it;
    TrackCircuitIteratorBegin(&it, { ride.cableLiftLoc, cableLaunchTileElement });
    while (TrackCircuitIteratorPrevious(&it) && !success)
    {
        TileElement* tileElement = it.current.element;
        CoordsXYZ loc = { it.current.x, it.current.y, it.currentZ };
        auto trackType = tileElement->AsTrack()->GetTrackType();
        if (tileElement->AsTrack()->HasCableLift() && trackType == TrackElemType::Flat)
            CableLaunchUpdateFinStateForTile(loc, tileElement->AsTrack(), set_lowered);
        else
            break;
    }
}

/*
static bool CableLaunchCheckBlockClosed(const Ride& ride)
{
    TileElement* cableLaunchTileElement = CableLaunchGetTileElement(ride);

    TrackCircuitIterator it;
    TrackCircuitIteratorBegin(&it, { ride.cableLiftLoc, cableLaunchTileElement });
    while (TrackCircuitIteratorPrevious(&it))
    {
        TrackElement* trackElement = it.current.element->AsTrack();
        if (trackElement->IsBlockStart())
            return trackElement->IsBrakeClosed();
    }
    return true;
}*/

void Vehicle::CableLaunchUpdateWaitingToDepart()
{
    auto curRide = GetRide();

    if (sub_state == 0)
    {
        CableLaunchUpdateFinState(*curRide, true);
    }
    else if (sub_state % 8 == 0)
    {
        CableLaunchUpdateFinState(*curRide, false);
    }
    sub_state++;

    if (sub_state < 40)
        return;

    velocity = 0;
    acceleration = 0;
    SetState(Vehicle::Status::Departing, 0);
}

void Vehicle::CableLaunchUpdateDeparting()
{
    sub_state++;
    if (sub_state < 16)
        return;

    Vehicle* passengerVehicle = GetEntity<Vehicle>(cable_lift_target);
    if (passengerVehicle == nullptr)
    {
        return;
    }

    // Calculate speed reduction based on number of riders
    /*
    int num_peeps_on_train = 0;
    int num_seats_on_train = 0;
    int num_cars_on_train = 0;
    int random = 0;
    for (const Vehicle* trainCar = passengerVehicle; trainCar != nullptr;
         trainCar = GetEntity<Vehicle>(trainCar->next_vehicle_on_train))
    {
        num_cars_on_train += 1;
        num_peeps_on_train += trainCar->num_peeps;
        num_seats_on_train += trainCar->num_seats & kVehicleSeatNumMask;
        // Use peep IDs as a random number to make the launch a bit unpredictable
        for (auto i = 0; i < num_peeps_on_train; i++)
        {
            random ^= trainCar->peep[i].ToUnderlying() & 0x3F;
        }
    }

    if (num_seats_on_train != 0)
        speed = (8 * num_cars_on_train - 4) * num_peeps_on_train / num_seats_on_train;
    else
        speed = 0;

    // Apply nondeterministic speed reduction. This generates a skewed distribution

    int8_t distribution[64] = { -8, -8, -8, -7, -7, -7, -7, -6, -6, -6, -6, -5, -5, -5, -5, -5, -4, -4, -4, -4, -3, -3,
                                -3, -3, -2, -2, -2, -1, -1, -1, 0,  0,  0,  1,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,
                                6,  7,  8,  9,  10, 11, 12, 14, 15, 17, 19, 21, 24, 27, 31, 35, 41, 49, 56, 64 };

    // if(distribution[random]>30)printf("Ride %d random %d\n",GetRide()->id.ToUnderlying(),distribution[random]);
    random = (distribution[random] * powered_acceleration) / (8 * GetRide()->launch_speed);
    if (speed + random < 0)
        speed = 0;
    else if (speed + random > 255)
        speed = 255;
    else
        speed += random;
    //       1 notch       2 notch  3 notch   4 notch
    // Yellow strata - 17mph at top empty 21 full    19            17       12        5
    // Red giga - 11mph at top empty 11 full         6  (-5) (-5)  rb       rb        rb
    // Blue xcelclone - 11mph at top empty 11 full   9       (-2)  4        rb        rb
    // Red terrain - 15mph at top empty 14 full      11 (-3) (-4)  8        5         rb
    // Red ritaclone - 13mph at top empty 12 full    10 (-1) (-3)  6        rb        rb
    // Brown mini -  13mph at top empty 11 full      10 (-1) (-3)  8        6         rb

    // These are fast over their hills
    // Purple speed- 25mph at the top empty 27 full
    // Black mini 19 empty 19 full

    // Multiple laps aren't allowed with the cable launch, so if NumLaps is greater than 1 the train rolled back. In this case
    // full power is applied to ensure the next launch is successful.
    if (passengerVehicle->NumLaps > 0)
    {
        printf("Cancelling speed\n");
        speed = 0;
    }
    */
    speed = 0;

    SetState(Vehicle::Status::Travelling, 0);
    passengerVehicle->SetState(Vehicle::Status::TravellingCableLift, passengerVehicle->sub_state);
}

void Vehicle::CableLaunchUpdateTravelling()
{
    Vehicle* passengerVehicle = GetEntity<Vehicle>(cable_lift_target);
    if (passengerVehicle == nullptr)
    {
        return;
    }

    auto curRide = GetRide();
    if (sub_state % 4 == 0)
    {
        CableLaunchUpdateFinState(*curRide, false);
        if (!(sub_state & 0x80))
            sub_state = 0;
    }
    if ((sub_state & 0x7F) != 0x7F)
        sub_state++;

    // Get rearmost vehicle in train
    Vehicle* rearVehicle = passengerVehicle;
    Vehicle* nextVehicle = GetEntity<Vehicle>(rearVehicle->next_vehicle_on_train);
    while (nextVehicle != nullptr)
    {
        rearVehicle = nextVehicle;
        nextVehicle = GetEntity<Vehicle>(rearVehicle->next_vehicle_on_train);
    }

    // If on the launch end piece and more than halfway through, then raise the first tile of brakes
    TileElement* trackElement = MapGetTrackElementAtOfTypeSeq(rearVehicle->TrackLocation, rearVehicle->GetTrackType(), 0);
    if (rearVehicle->GetTrackType() == TrackElemType::CableLaunch && rearVehicle->track_progress > 32
        && trackElement->AsTrack()->GetCableLaunchFinState() == CABLE_LAUNCH_FIN_STATE_LOWERED)
    {
        trackElement->AsTrack()->SetCableLaunchFinState(CABLE_LAUNCH_FIN_STATE_RAISING_0);
    }

    // Get track element previous to the one it is currently on
    if (trackElement != nullptr)
    {
        TrackBeginEnd trackBeginEnd;
        if (TrackBlockGetPrevious({ rearVehicle->TrackLocation, trackElement }, &trackBeginEnd)
            && trackBeginEnd.begin_element != nullptr && trackBeginEnd.begin_element->AsTrack()->HasCableLift()
            && trackBeginEnd.begin_element->AsTrack()->GetCableLaunchFinState() == CABLE_LAUNCH_FIN_STATE_LOWERED)
        {
            // TODO this seems to return the location of the first tile of the cable launch end piece, but the track element
            // corresponding to the last tile, which is confusing
            trackBeginEnd.begin_element->AsTrack()->SetCableLaunchFinState(CABLE_LAUNCH_FIN_STATE_RAISING_0);
        }
    }

    // If the catch car has already stopped, don't run the motion update
    if (sub_state & 0x80)
    {
        // if (CableLaunchCheckBlockClosed(*curRide))
        //{
        SetState(Vehicle::Status::MovingToEndOfStation, sub_state & 0x7F);
        //    return;
        //}
        return;
    }

    // Update motion
    if (passengerVehicle->status == Vehicle::Status::TravellingCableLift)
    {
        velocity = passengerVehicle->velocity;
        acceleration = 0;
    }
    else
    {
        int brake = std::min(velocity / 8, 127500);
        velocity -= brake;
        if (velocity < 16)
        {
            velocity = 0;
            acceleration = 0;
            sub_state |= 0x80;
        }
    }

    if (passengerVehicle->HasFlag(VehicleFlags::TrainIsBroken))
        return;

    // If catch car has reached the end of the launch track, stop it completely
    if (CableLiftUpdateTrackMotion() & VEHICLE_UPDATE_MOTION_TRACK_FLAG_1)
    {
        velocity = 0;
        acceleration = 0;
        sub_state |= 0x80;
    }
}
