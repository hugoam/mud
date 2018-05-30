//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.


#include <geom/Shape/Circle.h>
#include <math/Axes.h>
#include <geom/Primitive.h>

namespace mud
{
	uint16_t circle_subdiv(uint lod) { return uint16_t(6 + 12 * lod); }
	
	vec3 flip_point_axis(const vec3& point, SignedAxis axis)
	{
		if(axis == SignedAxis::PlusX || axis == SignedAxis::MinusX)
			return { point.y, point.x, point.z };
		else if(axis == SignedAxis::PlusZ || axis == SignedAxis::MinusZ)
			return { point.z, point.x, point.y };
		else
			return point;
	}

	uint16_t circle_vertices(const ProcShape& shape, const vec3& position, vec2 radius, SignedAxis axis, bool lines, MeshData& data, bool outward_normals)
	{
		uint16_t subdiv = circle_subdiv(uint(shape.m_symbol.m_detail));

		float increment = 2 * float(M_PI) / subdiv;
		float angle = 0.0f;

		for(uint16_t i = 0; i < subdiv; i++)
		{
			vec3 point = flip_point_axis({ radius.x * cos(angle), 0.f, radius.y * sin(angle) }, axis);
			data.position(position + point)
				.normal(outward_normals ? point : to_vec3(axis))
				.colour(lines ? shape.m_symbol.m_outline : shape.m_symbol.m_fill);
			angle += increment;
		}

		if(!lines)
			data.position(position)
				.normal(to_vec3(axis))
				.colour(shape.m_symbol.m_outline);

		return subdiv;
	}

	ShapeSize size_shape_lines(const ProcShape& shape, const Circle& circle)
	{
		UNUSED(circle);
		uint16_t subdiv = circle_subdiv(uint(shape.m_symbol.m_detail));
		return { subdiv, subdiv * 2 };
	}

	void draw_shape_lines(const ProcShape& shape, const Circle& circle, MeshData& data)
	{
		uint16_t subdiv = circle_vertices(shape, shape.m_position, vec2{ circle.m_radius }, to_signed_axis(circle.m_axis, true), true, data);

		for (uint16_t i = 0; i < subdiv; i++)
			data.line(i, i + 1 < subdiv ? i + 1 : 0);
	}

	ShapeSize size_shape_triangles(const ProcShape& shape, const Circle& circle)
	{
		UNUSED(circle);
		uint16_t subdiv = circle_subdiv(uint(shape.m_symbol.m_detail));
		return { subdiv + 1, subdiv * 3 };
	}

	void draw_shape_triangles(const ProcShape& shape, const Circle& circle, MeshData& data)
	{
		uint16_t subdiv = circle_vertices(shape, shape.m_position, vec2{ circle.m_radius }, to_signed_axis(circle.m_axis, true), false, data);

		for (uint16_t i = 0; i < subdiv; i++)
			data.tri(i + 1 < subdiv ? i + 1 : 0, i,  subdiv);
	}

	
	ShapeSize size_shape_lines(const ProcShape& shape, const Ellipsis& ellipsis)
	{
		UNUSED(ellipsis);
		uint16_t subdiv = circle_subdiv(uint(shape.m_symbol.m_detail));
		return { subdiv, subdiv * 2 };
	}

	void draw_shape_lines(const ProcShape& shape, const Ellipsis& ellipsis, MeshData& data)
	{
		uint16_t subdiv = circle_vertices(shape, shape.m_position, ellipsis.m_radius, to_signed_axis(ellipsis.m_axis, true), true, data);

		for (uint16_t i = 0; i < subdiv; i++)
			data.line(i, i + 1 < subdiv ? i + 1 : 0);
	}

	ShapeSize size_shape_triangles(const ProcShape& shape, const Ellipsis& ellipsis)
	{
		UNUSED(ellipsis);
		uint16_t subdiv = circle_subdiv(uint(shape.m_symbol.m_detail));
		return { subdiv + 1, subdiv * 3 };
	}

	void draw_shape_triangles(const ProcShape& shape, const Ellipsis& ellipsis, MeshData& data)
	{
		uint16_t subdiv = circle_vertices(shape, shape.m_position, ellipsis.m_radius, to_signed_axis(ellipsis.m_axis, true), false, data);

		for (uint16_t i = 0; i < subdiv; i++)
			data.tri(i + 1 < subdiv ? i + 1 : 0, i,  subdiv);
	}

	uint16_t torus_sides(uint lod) { return uint16_t(12 + 12 * lod); }
	uint16_t torus_rings(uint lod) { return uint16_t(12 + 12 * lod); }

	ShapeSize size_shape_lines(const ProcShape& shape, const Torus& torus)
	{
		UNUSED(shape); UNUSED(torus);
		return { 0, 0 };
	}

	void draw_shape_lines(const ProcShape& shape, const Torus& torus, MeshData& data)
	{
		UNUSED(shape); UNUSED(torus); UNUSED(data);
	}

	ShapeSize size_shape_triangles(const ProcShape& shape, const Torus& torus)
	{
		UNUSED(torus);
		uint16_t sides_subdiv = torus_sides(uint(shape.m_symbol.m_detail)) + 1;
		uint16_t rings_subdiv = torus_rings(uint(shape.m_symbol.m_detail)) + 1;
		return { int(sides_subdiv * rings_subdiv), int((sides_subdiv-1) * (rings_subdiv-1) * 6) };
	}

	void draw_shape_triangles(const ProcShape& shape, const Torus& torus, MeshData& data)
	{
		uint16_t sides = torus_sides(uint(shape.m_symbol.m_detail));
		uint16_t rings = torus_rings(uint(shape.m_symbol.m_detail));

		uint16_t sides_subdiv = sides + 1;
		uint16_t rings_subdiv = rings + 1;

		float vertical_delta = float(M_PI) * 2.0f / float(rings);
		float horizontal_delta = float(M_PI) * 2.0f / float(sides);

		for(uint16_t v = 0; v < sides_subdiv; v++)
			for(uint16_t h = 0; h < rings_subdiv; h++)
			{
				const float theta = vertical_delta * v;
				const float phi = horizontal_delta * h;

				const float x = cos(theta) * (torus.m_radius + torus.m_solid_radius * cos(phi));
				const float y = sin(theta) * (torus.m_radius + torus.m_solid_radius * cos(phi));
				const float z = torus.m_solid_radius * sin(phi);

				vec3 point = flip_point_axis({ x, z, y }, to_signed_axis(torus.m_axis, true));

				data.position(point)
					.colour(shape.m_symbol.m_fill);
			}

		for(uint16_t v = 0; v < sides; v++)
			for(uint16_t h = 0; h < rings; h++)
			{
				const ShapeIndex lt = h + v * sides_subdiv;
				const ShapeIndex rt = (h + 1) + v * sides_subdiv;

				const ShapeIndex lb = h + (v + 1) * sides_subdiv;
				const ShapeIndex rb = (h + 1) + (v + 1) * sides_subdiv;

				data.tri(lt, rt, lb);
				data.tri(rt, rb, lb);
			}
	}
}
