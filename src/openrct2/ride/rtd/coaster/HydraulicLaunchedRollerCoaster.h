/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../../../SpriteIds.h"
#include "../../RideData.h"
#include "../../ShopItem.h"
#include "../../Track.h"

// clang-format off
constexpr RideTypeDescriptor HydraulicLaunchedRollerCoasterRTD =
{
    .Category = RideCategory::rollerCoaster,
    .StartTrackPiece = OpenRCT2::TrackElemType::EndStation,
    .TrackPaintFunctions = TrackDrawerDescriptor({
        .trackStyle = TrackStyle::hydraulicLaunchedCoasterQuad,
        .supportType = MetalSupportType::Tubes,
        .enabledTrackGroups = { TrackGroup::flat, TrackGroup::straight, TrackGroup::stationEnd, TrackGroup::flatRollBanking, TrackGroup::slope, TrackGroup::slopeSteepLong, TrackGroup::slopeSteepUp, TrackGroup::slopeSteepDown, TrackGroup::slopeCurve, TrackGroup::slopeCurveSteep, TrackGroup::sBend, TrackGroup::curveSmall, TrackGroup::curve, TrackGroup::curveLarge, TrackGroup::corkscrew, TrackGroup::helixDownBankedHalf, TrackGroup::helixUpBankedHalf, TrackGroup::brakes, TrackGroup::onridePhoto, TrackGroup::slopeVertical, TrackGroup::barrelRoll, TrackGroup::slopeCurveBanked, TrackGroup::blockBrakes, TrackGroup::slopeRollBanking, TrackGroup::curveVertical, TrackGroup::quarterLoop, TrackGroup::halfLoop, TrackGroup::halfLoopMedium, TrackGroup::halfLoopLarge, TrackGroup::corkscrewLarge, TrackGroup::zeroGRoll, TrackGroup::zeroGRollLarge, TrackGroup::slopeCurveLarge, TrackGroup::diagBrakes, TrackGroup::diagBlockBrakes, TrackGroup::inclinedBrakes, TrackGroup::diagSlopeSteepLong, TrackGroup::diveLoop, TrackGroup::cableLaunch, TrackGroup::magneticBrake},
        .extraTrackGroups =  {TrackGroup::flatToSteepSlope},
    }),
    .InvertedTrackPaintFunctions = {},
    .Flags = kRtdFlagsHasThreeColours | kRtdFlagsCommonCoaster | kRtdFlagsCommonCoasterNonAlt|
                 EnumsToFlags(RtdFlag::hasTrackColourSupports, RtdFlag::hasLeaveWhenAnotherVehicleArrivesAtStation,
                              RtdFlag::checkGForces, RtdFlag::allowMultipleCircuits, RtdFlag::allowReversedTrains, RtdFlag::hasAlternateTrack, RtdFlag::allowCableLaunch),
    .RideModes = EnumsToFlags(RideMode::continuousCircuit, RideMode::continuousCircuitBlockSectioned),
    .DefaultMode = RideMode::continuousCircuit,
    .OperatingSettings = { 10, 70 },
    .BoosterSettings = { 15, 52 },
    .LegacyBoosterSettings = { 15, 52 },
    .Naming = { STR_RIDE_NAME_HYDRAULIC_LAUNCHED_ROLLER_COASTER, STR_RIDE_DESCRIPTION_HYDRAULIC_LAUNCHED_ROLLER_COASTER },
    .NameConvention = { RideComponentType::Train, RideComponentType::Track, RideComponentType::Station },
    .AvailableBreakdowns = (1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),
    .Heights = { 255, 24, 8, 7},
    .MaxMass = 18,
    .LiftData = { OpenRCT2::Audio::SoundId::Null, 5, 8 },
    .RatingsMultipliers = { 52, 36, 10 },
    .UpkeepCosts = { 40, 20, 80, 10, 3, 10 },
    .BuildCosts = { 37.50_GBP, 2.50_GBP, 50},
    .DefaultPrices = { 20, 20 },
    .DefaultMusic = kMusicObjectRock1,
    .PhotoItem = ShopItem::Photo,
    .BonusValue = 80,
    .ColourPresets = TRACK_COLOUR_PRESETS(
        { COLOUR_LIGHT_ORANGE, COLOUR_LIGHT_ORANGE, COLOUR_WHITE },
        { COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_BLACK },
        { COLOUR_LIGHT_BLUE, COLOUR_LIGHT_BLUE, COLOUR_DARK_GREEN }
    ),
    .ColourPreview = { SPR_RIDE_DESIGN_PREVIEW_HYDRAULIC_LAUNCHED_ROLLER_COASTER_TRACK, SPR_RIDE_DESIGN_PREVIEW_HYDRAULIC_LAUNCHED_ROLLER_COASTER_SUPPORTS },
    .ColourKey = RideColourKey::Ride,
    .Name = "hydraulic_launched_rc",
    .RatingsData = 
    {
        RatingsCalculationType::Normal,
        { MakeRideRating(3, 50), MakeRideRating(0, 60), MakeRideRating(0, 40) },
        16,
        -1,
        false,
        {
            { RatingsModifierType::BonusLength,           6000,             764, 0, 0 },
            { RatingsModifierType::BonusSynchronisation,  0,                MakeRideRating(0, 40), MakeRideRating(0, 05), 0 },
            { RatingsModifierType::BonusTrainLength,      0,                187245, 0, 0 },
            { RatingsModifierType::BonusMaxSpeed,         0,                44281, 88562, 35424 },
            { RatingsModifierType::BonusAverageSpeed,     0,                364088, 436906, 0 },
            { RatingsModifierType::BonusDuration,         150,              26214, 0, 0 },
            { RatingsModifierType::BonusGForces,          0,                36864, 35746, 49648 },
            { RatingsModifierType::BonusTurns,            0,                26749, 34767, 45749 },
            { RatingsModifierType::BonusDrops,            0,                29127, 46811, 49152 },
            { RatingsModifierType::BonusSheltered,        0,                15420, 32768, 35108 },
            { RatingsModifierType::BonusReversedTrains,   0,                2, 15, 25 },
            { RatingsModifierType::BonusProximity,        0,                22367, 0, 0 },
            { RatingsModifierType::BonusScenery,          0,                6693, 0, 0 },
            { RatingsModifierType::RequirementDropHeight, 14,               2, 2, 2 },
            { RatingsModifierType::RequirementMaxSpeed,   0xA0000,          2, 2, 2 },
            { RatingsModifierType::RequirementNegativeGs, MakeFixed16_2dp(0, 40), 2, 2, 2 },
            { RatingsModifierType::RequirementNumDrops,   2,                2, 2, 2 },
            { RatingsModifierType::PenaltyLateralGs,      0,                24576, 35746, 49648 },
        },
    },
    .CatchCarParameters = {&kCableLaunchVehicle,4},
    .AlternateTrackList = {3,{{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER_ALT_2,SPR_RIDE_CONSTRUCTION_BI_RAIL_TRACK},{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER_ALT_1,SPR_RIDE_CONSTRUCTION_TRI_RAIL_TRACK},{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER,SPR_RIDE_CONSTRUCTION_QUAD_RAIL_TRACK}}},
};

//Tri rail track
constexpr RideTypeDescriptor HydraulicLaunchedRollerCoasterAlt1RTD =
{
    .Category = RideCategory::rollerCoaster,
    .StartTrackPiece = OpenRCT2::TrackElemType::EndStation,
    .TrackPaintFunctions = TrackDrawerDescriptor({
        .trackStyle = TrackStyle::hydraulicLaunchedCoasterBi,
        .supportType = MetalSupportType::Tubes,
        .enabledTrackGroups = { TrackGroup::flat, TrackGroup::straight, TrackGroup::stationEnd, TrackGroup::flatRollBanking, TrackGroup::slope, TrackGroup::slopeSteepLong, TrackGroup::slopeSteepUp, TrackGroup::slopeSteepDown, TrackGroup::slopeCurve, TrackGroup::slopeCurveSteep, TrackGroup::sBend, TrackGroup::curveSmall, TrackGroup::curve, TrackGroup::curveLarge, TrackGroup::corkscrew, TrackGroup::helixDownBankedHalf, TrackGroup::helixUpBankedHalf, TrackGroup::brakes, TrackGroup::onridePhoto, TrackGroup::slopeVertical, TrackGroup::barrelRoll, TrackGroup::slopeCurveBanked, TrackGroup::blockBrakes, TrackGroup::slopeRollBanking, TrackGroup::curveVertical, TrackGroup::quarterLoop, TrackGroup::halfLoop, TrackGroup::halfLoopMedium, TrackGroup::halfLoopLarge, TrackGroup::corkscrewLarge, TrackGroup::zeroGRoll, TrackGroup::zeroGRollLarge, TrackGroup::slopeCurveLarge, TrackGroup::diagBrakes, TrackGroup::diagBlockBrakes, TrackGroup::inclinedBrakes, TrackGroup::diagSlopeSteepLong, TrackGroup::diveLoop ,TrackGroup::cableLaunch, TrackGroup::magneticBrake},
        .extraTrackGroups =  {TrackGroup::flatToSteepSlope},
    }),
    .InvertedTrackPaintFunctions = {},
    .Flags = kRtdFlagsHasThreeColours | kRtdFlagsCommonCoaster | kRtdFlagsCommonCoasterNonAlt|
                 EnumsToFlags(RtdFlag::hasTrackColourSupports, RtdFlag::hasLeaveWhenAnotherVehicleArrivesAtStation,
                              RtdFlag::checkGForces, RtdFlag::allowMultipleCircuits, RtdFlag::allowReversedTrains, RtdFlag::hasAlternateTrack, RtdFlag::allowCableLaunch),
    .RideModes = EnumsToFlags(RideMode::continuousCircuit, RideMode::continuousCircuitBlockSectioned),
    .DefaultMode = RideMode::continuousCircuit,
    .BoosterSettings = { 15, 52 },
    .LegacyBoosterSettings = { 15, 52 },
    .Naming = { STR_RIDE_NAME_HYDRAULIC_LAUNCHED_ROLLER_COASTER, STR_RIDE_DESCRIPTION_HYDRAULIC_LAUNCHED_ROLLER_COASTER },
    .NameConvention = { RideComponentType::Train, RideComponentType::Track, RideComponentType::Station },
    .AvailableBreakdowns = (1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),
    .Heights = { 86, 24, 8, 7},
    .MaxMass = 18,
    .LiftData = { OpenRCT2::Audio::SoundId::Null, 5, 8 },
    .RatingsMultipliers = { 52, 36, 10 },
    .UpkeepCosts = { 40, 20, 80, 10, 3, 10 },
    .BuildCosts = { 37.50_GBP, 2.50_GBP, 50},
    .DefaultPrices = { 20, 20 },
    .DefaultMusic = kMusicObjectRock1,
    .PhotoItem = ShopItem::Photo,
    .BonusValue = 80,
    .ColourPresets = TRACK_COLOUR_PRESETS(
        { COLOUR_LIGHT_ORANGE, COLOUR_LIGHT_ORANGE, COLOUR_WHITE },
        { COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_BLACK },
        { COLOUR_LIGHT_BLUE, COLOUR_LIGHT_BLUE, COLOUR_DARK_GREEN }
    ),
    .ColourPreview = { SPR_RIDE_DESIGN_PREVIEW_HYDRAULIC_LAUNCHED_ROLLER_COASTER_TRACK, SPR_RIDE_DESIGN_PREVIEW_HYDRAULIC_LAUNCHED_ROLLER_COASTER_SUPPORTS },
    .ColourKey = RideColourKey::Ride,
    .Name = "hydraulic_launched_rc",
    .RatingsData = 
    {
        RatingsCalculationType::Normal,
        { MakeRideRating(3, 50), MakeRideRating(0, 60), MakeRideRating(0, 40) },
        16,
        -1,
        false,
        {
            { RatingsModifierType::BonusLength,           6000,             764, 0, 0 },
            { RatingsModifierType::BonusSynchronisation,  0,                MakeRideRating(0, 40), MakeRideRating(0, 05), 0 },
            { RatingsModifierType::BonusTrainLength,      0,                187245, 0, 0 },
            { RatingsModifierType::BonusMaxSpeed,         0,                44281, 88562, 35424 },
            { RatingsModifierType::BonusAverageSpeed,     0,                364088, 436906, 0 },
            { RatingsModifierType::BonusDuration,         150,              26214, 0, 0 },
            { RatingsModifierType::BonusGForces,          0,                36864, 35746, 49648 },
            { RatingsModifierType::BonusTurns,            0,                26749, 34767, 45749 },
            { RatingsModifierType::BonusDrops,            0,                29127, 46811, 49152 },
            { RatingsModifierType::BonusSheltered,        0,                15420, 32768, 35108 },
            { RatingsModifierType::BonusReversedTrains,   0,                2, 15, 25 },
            { RatingsModifierType::BonusProximity,        0,                22367, 0, 0 },
            { RatingsModifierType::BonusScenery,          0,                6693, 0, 0 },
            { RatingsModifierType::RequirementDropHeight, 14,               2, 2, 2 },
            { RatingsModifierType::RequirementMaxSpeed,   0xA0000,          2, 2, 2 },
            { RatingsModifierType::RequirementNegativeGs, MakeFixed16_2dp(0, 40), 2, 2, 2 },
            { RatingsModifierType::RequirementNumDrops,   2,                2, 2, 2 },
            { RatingsModifierType::PenaltyLateralGs,      0,                24576, 35746, 49648 },
        },
    },
    .CatchCarParameters = {&kCableLaunchVehicle,4},
    .AlternateTrackList = {3,{{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER_ALT_2,SPR_RIDE_CONSTRUCTION_BI_RAIL_TRACK},{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER_ALT_1,SPR_RIDE_CONSTRUCTION_TRI_RAIL_TRACK},{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER,SPR_RIDE_CONSTRUCTION_QUAD_RAIL_TRACK}}},
};

//Bi rail track
constexpr const RideTypeDescriptor HydraulicLaunchedRollerCoasterAlt2RTD =
{
    .Category = RideCategory::rollerCoaster,
    .StartTrackPiece = OpenRCT2::TrackElemType::EndStation,
    .TrackPaintFunctions = TrackDrawerDescriptor({
        .trackStyle = TrackStyle::hydraulicLaunchedCoasterTri,
        .supportType = MetalSupportType::Tubes,
        .enabledTrackGroups = { TrackGroup::flat, TrackGroup::straight, TrackGroup::stationEnd, TrackGroup::flatRollBanking, TrackGroup::slope, TrackGroup::slopeSteepLong, TrackGroup::slopeSteepUp, TrackGroup::slopeSteepDown, TrackGroup::slopeCurve, TrackGroup::slopeCurveSteep, TrackGroup::sBend, TrackGroup::curveSmall, TrackGroup::curve, TrackGroup::curveLarge, TrackGroup::corkscrew, TrackGroup::helixDownBankedHalf, TrackGroup::helixUpBankedHalf, TrackGroup::brakes, TrackGroup::onridePhoto, TrackGroup::slopeVertical, TrackGroup::barrelRoll, TrackGroup::slopeCurveBanked, TrackGroup::blockBrakes, TrackGroup::slopeRollBanking, TrackGroup::curveVertical, TrackGroup::quarterLoop, TrackGroup::halfLoop, TrackGroup::halfLoopMedium, TrackGroup::halfLoopLarge, TrackGroup::corkscrewLarge, TrackGroup::zeroGRoll, TrackGroup::zeroGRollLarge, TrackGroup::slopeCurveLarge, TrackGroup::diagBrakes, TrackGroup::diagBlockBrakes, TrackGroup::inclinedBrakes, TrackGroup::diagSlopeSteepLong, TrackGroup::diveLoop ,TrackGroup::cableLaunch, TrackGroup::magneticBrake},
        .extraTrackGroups =  {TrackGroup::flatToSteepSlope},
    }),
    .InvertedTrackPaintFunctions = {},
    .Flags = kRtdFlagsHasThreeColours | kRtdFlagsCommonCoaster | kRtdFlagsCommonCoasterNonAlt|
                 EnumsToFlags(RtdFlag::hasTrackColourSupports, RtdFlag::hasLeaveWhenAnotherVehicleArrivesAtStation,
                              RtdFlag::checkGForces, RtdFlag::allowMultipleCircuits, RtdFlag::allowReversedTrains, RtdFlag::hasAlternateTrack, RtdFlag::allowCableLaunch),
    .RideModes = EnumsToFlags(RideMode::continuousCircuit, RideMode::continuousCircuitBlockSectioned),
    .DefaultMode = RideMode::continuousCircuit,
    .BoosterSettings = { 15, 52 },
    .LegacyBoosterSettings = { 15, 52 },
    .Naming = { STR_RIDE_NAME_HYDRAULIC_LAUNCHED_ROLLER_COASTER, STR_RIDE_DESCRIPTION_HYDRAULIC_LAUNCHED_ROLLER_COASTER },
    .NameConvention = { RideComponentType::Train, RideComponentType::Track, RideComponentType::Station },
    .AvailableBreakdowns = (1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),
    .Heights = { 16, 24, 8, 7},
    .MaxMass = 18,
    .LiftData = { OpenRCT2::Audio::SoundId::Null, 5, 8 },
    .RatingsMultipliers = { 52, 36, 10 },
    .UpkeepCosts = { 40, 20, 80, 10, 3, 10 },
    .BuildCosts = { 37.50_GBP, 2.50_GBP, 50},
    .DefaultPrices = { 20, 20 },
    .DefaultMusic = kMusicObjectRock1,
    .PhotoItem = ShopItem::Photo,
    .BonusValue = 80,
    .ColourPresets = TRACK_COLOUR_PRESETS(
        { COLOUR_LIGHT_ORANGE, COLOUR_LIGHT_ORANGE, COLOUR_WHITE },
        { COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_BLACK },
        { COLOUR_LIGHT_BLUE, COLOUR_LIGHT_BLUE, COLOUR_DARK_GREEN }
    ),
    .ColourPreview = { SPR_RIDE_DESIGN_PREVIEW_HYDRAULIC_LAUNCHED_ROLLER_COASTER_TRACK, SPR_RIDE_DESIGN_PREVIEW_HYDRAULIC_LAUNCHED_ROLLER_COASTER_SUPPORTS },
    .ColourKey = RideColourKey::Ride,
    .Name = "hydraulic_launched_rc",
    .RatingsData = 
    {
        RatingsCalculationType::Normal,
        { MakeRideRating(3, 50), MakeRideRating(0, 60), MakeRideRating(0, 40) },
        16,
        -1,
        false,
        {
            { RatingsModifierType::BonusLength,           6000,             764, 0, 0 },
            { RatingsModifierType::BonusSynchronisation,  0,                MakeRideRating(0, 40), MakeRideRating(0, 05), 0 },
            { RatingsModifierType::BonusTrainLength,      0,                187245, 0, 0 },
            { RatingsModifierType::BonusMaxSpeed,         0,                44281, 88562, 35424 },
            { RatingsModifierType::BonusAverageSpeed,     0,                364088, 436906, 0 },
            { RatingsModifierType::BonusDuration,         150,              26214, 0, 0 },
            { RatingsModifierType::BonusGForces,          0,                36864, 35746, 49648 },
            { RatingsModifierType::BonusTurns,            0,                26749, 34767, 45749 },
            { RatingsModifierType::BonusDrops,            0,                29127, 46811, 49152 },
            { RatingsModifierType::BonusSheltered,        0,                15420, 32768, 35108 },
            { RatingsModifierType::BonusReversedTrains,   0,                2, 15, 25 },
            { RatingsModifierType::BonusProximity,        0,                22367, 0, 0 },
            { RatingsModifierType::BonusScenery,          0,                6693, 0, 0 },
            { RatingsModifierType::RequirementDropHeight, 14,               2, 2, 2 },
            { RatingsModifierType::RequirementMaxSpeed,   0xA0000,          2, 2, 2 },
            { RatingsModifierType::RequirementNegativeGs, MakeFixed16_2dp(0, 40), 2, 2, 2 },
            { RatingsModifierType::RequirementNumDrops,   2,                2, 2, 2 },
            { RatingsModifierType::PenaltyLateralGs,      0,                24576, 35746, 49648 },
        },
    },
    .CatchCarParameters = {&kCableLaunchVehicle,4},
    .AlternateTrackList = {3,{{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER_ALT_2,SPR_RIDE_CONSTRUCTION_BI_RAIL_TRACK},{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER_ALT_1,SPR_RIDE_CONSTRUCTION_TRI_RAIL_TRACK},{RIDE_TYPE_HYDRAULIC_LAUNCHED_ROLLER_COASTER,SPR_RIDE_CONSTRUCTION_QUAD_RAIL_TRACK}}},
};
// clang-format on
