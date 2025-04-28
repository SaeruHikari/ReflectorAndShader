#pragma once
#include "attributes.hpp"
#include "type_traits.hpp"
#include "numeric.hpp"

#include "types.hpp"
#include "resources.hpp"

#include "functions.hpp"
#include "raytracing.hpp"
namespace skr::shader {

template <typename Resource, typename T>
static void store_2d(Resource& r, uint32 row_pitch, uint2 pos, T val)
{
    using ResourceType = remove_cvref_t<Resource>;
    if constexpr (is_same_v<ResourceType, Buffer<T>>)
        r.store(pos.x + pos.y * row_pitch, val);
    else if constexpr (is_same_v<ResourceType, Image<scalar_type<T>>>)
        r.store(pos, val);
}
}
