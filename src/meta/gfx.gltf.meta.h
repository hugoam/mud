#pragma once

#if !defined TWO_MODULES || defined TWO_TYPE_LIB
#include <refl/Module.h>
#endif

#include <gfx-gltf/Forward.h>
//#include <gfx-gltf/Types.h>

#ifndef TWO_GFX_GLTF_REFL_EXPORT
#define TWO_GFX_GLTF_REFL_EXPORT TWO_IMPORT
#endif

namespace two
{
	export_ class TWO_GFX_GLTF_REFL_EXPORT two_gfx_gltf : public two::Module
	{
	private:
		two_gfx_gltf();
		
	public:
		static two_gfx_gltf& m() { static two_gfx_gltf instance; return instance; }
	};
}

#ifdef TWO_GFX_GLTF_MODULE
extern "C"
TWO_GFX_GLTF_REFL_EXPORT Module& getModule();
#endif
