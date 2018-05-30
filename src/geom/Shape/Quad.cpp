//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.


#include <geom/Shape/Quad.h>
#include <geom/Shapes.h>
#include <geom/Primitive.h>

//#define PK_GLITCH

namespace mud
{
	static vec2 quadUVs[4] = { { 1.f, 1.f }, { 1.f, 0.f }, { 0.f, 0.f }, { 0.f, 1.f } };

	vec3 quad_normal(const Quad& quad)
	{
#ifndef PK_GLITCH
		vec3 x = quad.m_vertices[1] - quad.m_vertices[0];
		vec3 y = quad.m_vertices[2] - quad.m_vertices[0];
		return normalize(cross(x, y));
#else
		return 1.f;
#endif
	}

	void quad_vertices(const ProcShape& shape, const Quad& quad, bool fill, MeshData& data)
	{
		vec3 normal = quad_normal(quad);
		for(int i = 0; i < 4; i++)
		{
			data.position(shape.m_position + quad.m_vertices[i])
				.colour(fill ? shape.m_symbol.m_fill : shape.m_symbol.m_outline)
				.normal(normal)
				.textureCoord(quadUVs[i]);
		}
	}

	ShapeSize size_shape_lines(const ProcShape& shape, const Quad& quad)
	{
		UNUSED(shape); UNUSED(quad);
		return { 4, 8 };
	}

	void draw_shape_lines(const ProcShape& shape, const Quad& quad, MeshData& data)
	{
		quad_vertices(shape, quad, false, data);
		for(uint16_t i = 0; i < 4; i++)
			data.line(i, (i + 1) % 4);
	}

	ShapeSize size_shape_triangles(const ProcShape& shape, const Quad& quad)
	{
		UNUSED(shape); UNUSED(quad);
		return { 4, 6 };
	}

	void draw_shape_triangles(const ProcShape& shape, const Quad& quad, MeshData& data)
	{
		quad_vertices(shape, quad, true, data);
		data.quad(0, 1, 2, 3);
	}
}
