/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#ifdef ENABLE_SCRIPTING

    #include "../../Duktape.hpp"
    #include "../../../GameState.h"
    #include "../../../particle/Particle.h"
namespace OpenRCT2::Scripting
{
    class ScEffect
    {
    protected:
        EffectId _id{ EffectId::GetNull() };


    private:
        DukValue id_get() const
        {
            auto ctx = GetContext()->GetScriptEngine().GetContext();

            auto effect = GetEffect();
            if (effect == nullptr)
                return ToDuk(ctx, nullptr);

            return ToDuk(ctx, effect->id.ToUnderlying());
        }

        Effect* GetEffect() const
        {
            return OpenRCT2::getGameState().particles.GetEffect(_id);
        }

    public:
        ScEffect(EffectId id)
            : _id(id)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_register_property(ctx, &ScEffect::id_get, nullptr, "id");
        }

    };

} // namespace OpenRCT2::Scripting

#endif

