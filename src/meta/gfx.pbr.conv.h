#pragma once

#include <gfx-pbr/Types.h>

#if !defined TWO_MODULES || defined TWO_TYPE_LIB
#include <refl/Meta.h>
#include <refl/Enum.h>
#include <infra/StringOps.h>
#endif

namespace two
{
	export_ template <> inline void to_value(const string& str, two::TonemapMode& val) { val = two::TonemapMode(enu<two::TonemapMode>().value(str.c_str())); };
	export_ template <> inline void to_string(const two::TonemapMode& val, string& str) { str = enu<two::TonemapMode>().name(uint32_t(val)); };
	
	
}
