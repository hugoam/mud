#ifdef TWO_MODULES
module;
#include <infra/Cpp20.h>
module TWO(infra);
#else
#include <refl/Module.h>
#include <meta/infra.meta.h>
#endif

namespace two
{
	two_infra::two_infra()
		: Module("two::infra", {  })
	{}
}

#ifdef TWO_INFRA_MODULE
extern "C"
Module& getModule()
{
	return two_infra::m();
}
#endif
