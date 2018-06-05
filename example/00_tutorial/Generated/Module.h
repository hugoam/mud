
#pragma once

#if !defined MUD_MODULES || defined MUD_OBJ_LIB
#include <obj/System/System.h>
#endif

#ifndef MUD_MODULES
#include <obj/Generated/Module.h>
#include <math/Generated/Module.h>
#include <ui/Generated/Module.h>
#include <uio/Generated/Module.h>
#include <gfx/Generated/Module.h>
#include <edit/Generated/Module.h>
#include <gen/Generated/Module.h>
#endif
        
#include <00_tutorial/Generated/Forward.h>
#include <00_tutorial/Generated/Types.h>

#include <00_tutorial/00_tutorial.h>

#include <00_tutorial/Generated/Convert.h>

	export_ class _00_TUTORIAL_EXPORT ex_00_tutorial : public Module
	{
	private:
		ex_00_tutorial();

	public:
		static ex_00_tutorial& m() { static ex_00_tutorial instance; return instance; }
	};

#ifdef _EX_00_TUTORIAL_MODULE
extern "C"
_00_TUTORIAL_EXPORT Module& getModule();
#endif
