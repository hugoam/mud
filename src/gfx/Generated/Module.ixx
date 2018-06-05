#include <assert.h>
#include <stdint.h>
#include <float.h>

export module mud.gfx;
export import std.core;
export import std.memory;
export import std.threading;

export import mud.obj;
export import mud.math;
export import mud.geom;

#include <gfx/Generated/Module.h>

//#include <gfx/Animated.h>
//#include <gfx/Animation.h>
//#include <gfx/Asset.h>
//#include <gfx/Asset.impl.h>
//#include <gfx/Camera.h>
//#include <gfx/Draw.h>
//#include <gfx/Effects.h>
//#include <gfx/Filter.h>
//#include <gfx/Frustum.h>
//#include <gfx/Generator.h>
//#include <gfx/Gfx.h>
//#include <gfx/GfxSystem.h>
//#include <gfx/Graph.h>
//#include <gfx/Item.h>
//#include <gfx/Light.h>
//#include <gfx/ManualRender.h>
//#include <gfx/Material.h>
//#include <gfx/Mesh.h>
//#include <gfx/Model.h>
//#include <gfx/Node3.h>
//#include <gfx/Particles.h>
//#include <gfx/Picker.h>
//#include <gfx/Pipeline.h>
//#include <gfx/Prefab.h>
//#include <gfx/Program.h>
//#include <gfx/Renderer.h>
//#include <gfx/RenderTarget.h>
//#include <gfx/Scene.h>
//#include <gfx/Shader.h>
//#include <gfx/Shot.h>
//#include <gfx/Skeleton.h>
//#include <gfx/Texture.h>
//#include <gfx/Uniform.h>
//#include <gfx/Viewport.h>
//#include <gfx/Blocks/Sky.h>
