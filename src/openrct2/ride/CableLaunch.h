/*****************************************************************************
 * Copyright (c) 2014-2023 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <cstdint>

struct Vehicle;
struct Ride;

enum CableLaunchState
{
    CABLE_LAUNCH_FIN_STATE_LOWERED = 0x0,
    CABLE_LAUNCH_FIN_STATE_RAISING_0 = 0x1,
    CABLE_LAUNCH_FIN_STATE_RAISING_1 = 0x2,
    CABLE_LAUNCH_FIN_STATE_RAISING_2 = 0x3,
    CABLE_LAUNCH_FIN_STATE_RAISING_3 = 0x4,
    CABLE_LAUNCH_FIN_STATE_RAISED = 0x5,
    CABLE_LAUNCH_FIN_STATE_LOWERING_0 = 0x6,
    CABLE_LAUNCH_FIN_STATE_LOWERING_1 = 0x7,
    CABLE_LAUNCH_FIN_STATE_LOWERING_2 = 0x8,
    CABLE_LAUNCH_IS_BRAKE_SECTION = 0x10,
};

Vehicle* CableLaunchSegmentCreate(
    Ride& ride, int32_t x, int32_t y, int32_t z, int32_t direction, uint16_t var_44, int32_t remaining_distance, int index);
