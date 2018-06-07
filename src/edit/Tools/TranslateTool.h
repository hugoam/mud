//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#pragma once

#ifndef MUD_MODULES
#include <math/Vec.h>
#include <math/Axis.h>
#endif
#include <edit/Generated/Forward.h>
#include <edit/Tool.h>

#ifndef MUD_CPP_20
#include <vector>
#endif

namespace mud
{
	export_ class refl_ MUD_EDIT_EXPORT TranslateAction : public TransformAction
	{
	public:
		TranslateAction(const std::vector<Transform*>& targets);

		virtual void apply(Transform& transform) override;
		virtual void undo(Transform& transform) override;

		virtual void update(const vec3& start, const vec3& end) override;

	protected:
		vec3 m_translation;
	};

	export_ class refl_ MUD_EDIT_EXPORT TranslateTool : public TransformTool
	{
	public:
		TranslateTool(ToolContext& context);

		Gizmo linear_gizmo(Axis axis, float hue);
		Gizmo planar_gizmo(Axis normal, float hue);

		virtual object_ptr<TransformAction> create_action(const std::vector<Transform*>& transforms);
	};
}
