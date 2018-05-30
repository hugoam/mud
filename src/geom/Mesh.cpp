//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.


#include <geom/Mesh.h>
#include <geom/Geom.h>

#include <obj/Vector.h>
#include <math/VecOps.h>

#include <mikktspace.h>

#include <unordered_map>
#include <stdio.h>

namespace mud
{
	uint32_t joints(const ivec4& j)
	{
		return uint32_t(j[0]) | uint32_t(j[1]) << 8 | uint32_t(j[2]) << 16 | uint32_t(j[3]) << 24;
	}

	inline void vec_to_array(const vec3& in, float out[]) { out[0] = in.x; out[1] = in.y; out[2] = in.z; }
	inline void vec_to_array(const vec2& in, float out[]) { out[0] = in.x; out[1] = in.y; }

	Geometry::Geometry()
		: Shape(type<Geometry>())
		, m_vertices()
		, m_triangles()
	{}

	void Geometry::allocate(size_t vertCount, size_t triCount)
	{
		m_vertices.resize(vertCount);
		m_triangles.resize(triCount);
	}

	object_ptr<Shape> Geometry::clone() const { return make_object<Geometry>(*this); }

	MeshPacker::MeshPacker()
	{}

	void MeshPacker::bake(bool normals, bool tangents)
	{
		if(normals)
			this->generate_normals();

		if(tangents)
			this->generate_tangents();

		this->generate_indices();
	}

	void MeshPacker::pack_vertices()
	{
		for(size_t i = 0; i < m_positions.size(); ++i)
		{
			ShapeVertex& vertex = this->vertex();
			if(!m_positions.empty()) vertex.m_position = m_positions[i];
			if(!m_normals.empty())   vertex.m_normal = m_normals[i];
			if(!m_tangents.empty())  vertex.m_tangent = m_tangents[i];
			if(!m_uv0s.empty())      vertex.m_uv0 = m_uv0s[i];
			//if(!m_uv1s.empty())      vertex.m_uv1 = m_uv1s[i];
			if(!m_bones.empty())	 vertex.m_joints = joints(m_bones[i]);
			if(!m_weights.empty())	 vertex.m_weights = m_weights[i];
		}

		if(m_normals.empty())
			this->generate_normals();

		if(m_tangents.empty())
			this->generate_tangents();
	}

	void MeshPacker::pack_vertices(const mat4& transform)
	{
		for(size_t i = 0; i < m_positions.size(); ++i)
		{
			ShapeVertex& vertex = this->vertex();
			if(!m_positions.empty()) vertex.m_position = vec3(transform * vec4(m_positions[i], 1.f));
			if(!m_normals.empty())   vertex.m_normal = vec3(transform * vec4(m_normals[i], 0.f));
			if(!m_tangents.empty())  vertex.m_tangent = vec4(vec3(transform * vec4(vec3(m_tangents[i]), 0.f)), m_tangents[i].w);
			if(!m_uv0s.empty())      vertex.m_uv0 = m_uv0s[i];
			//if(!m_uv1s.empty())      vertex.m_uv1 = m_uv1s[i];
			if(!m_bones.empty())	 vertex.m_joints = joints(m_bones[i]);
			if(!m_weights.empty())	 vertex.m_weights = m_weights[i];
		}

		if(m_normals.empty())
			this->generate_normals();

		if(m_tangents.empty())
			this->generate_tangents();
	}

	void MeshPacker::generate_normals()
	{
		/*
		std::map<int, bool> smooth_groups;

		bool was_indexed = index_array.size();

		deindex();

		HashMap<Vertex, Vector3, VertexHasher> vertex_hash;

		int count = 0;
		bool smooth = false;
		if(smooth_groups.has(0))
			smooth = smooth_groups[0];

		List<Vertex>::Element *B = vertex_array.front();
		for(List<Vertex>::Element *E = B; E;) {

			List<Vertex>::Element *v[3];
			v[0] = E;
			v[1] = v[0]->next();
			ERR_FAIL_COND(!v[1]);
			v[2] = v[1]->next();
			ERR_FAIL_COND(!v[2]);
			E = v[2]->next();

			Vector3 normal = Plane(v[0]->get().vertex, v[1]->get().vertex, v[2]->get().vertex).normal;

			if(smooth) {

				for(int i = 0; i < 3; i++) {

					Vector3 *lv = vertex_hash.getptr(v[i]->get());
					if(!lv) {
						vertex_hash.set(v[i]->get(), normal);
					}
					else {
						(*lv) += normal;
					}
				}
			}
			else {

				for(int i = 0; i < 3; i++) {

					v[i]->get().normal = normal;
				}
			}
			count += 3;

			if(smooth_groups.has(count) || !E) {

				if(vertex_hash.size()) {

					while(B != E) {

						Vector3 *lv = vertex_hash.getptr(B->get());
						if(lv) {
							B->get().normal = lv->normalized();
						}

						B = B->next();
					}

				}
				else {
					B = E;
				}

				vertex_hash.clear();
				if(E) {
					smooth = smooth_groups[count];
				}
			}
		}

		format |= Mesh::ARRAY_FORMAT_NORMAL;

		if(was_indexed) {
			index();
			smooth_groups.clear();
		}
		*/

		//std::unordered_map<vec3, vec3> smooth_normals;

		std::vector<int> smooth_groups;

		for(int smooth_group : smooth_groups)
		{
			for(size_t i = 0; i < m_vertices.size(); i += 3)
			{
				if(smooth_group != 0)
					continue;

				ShapeVertex& v0 = m_vertices[i + 0];
				ShapeVertex& v1 = m_vertices[i + 1];
				ShapeVertex& v2 = m_vertices[i + 2];

				vec3 normal = Plane(v0.m_position, v1.m_position, v2.m_position).m_normal;

				v0.m_normal = normal;
				v1.m_normal = normal;
				v2.m_normal = normal;
			}
		
			if(smooth_group == 0)
				continue;

			/*for(auto& kv : smooth_normals)
				kv.second = normalize(kv.second);

			for(size_t i = 0; i < vertices.size(); i++)
			{
				ShapeVertex& vertex = vertices[i];
				vertex.m_normal = smooth_normals[vertex.m_position];
			}

			smooth_normals.clear();*/
		}
	}

	struct ShapeData
	{
		array<ShapeIndex> m_indices;
		array<ShapeVertex> m_vertices;
	};

	inline ShapeData& mikkt_mesh(const SMikkTSpaceContext* context) { return *static_cast<ShapeData*>(context->m_pUserData); }
	inline ShapeVertex& mikkt_vertex(const SMikkTSpaceContext* context, int face, int vert)
	{
		ShapeData& shape_data = *static_cast<ShapeData*>(context->m_pUserData);
		ShapeIndex index = shape_data.m_indices[face * 3 + vert];
		return shape_data.m_vertices[index];
	}

	void generate_mikkt_tangents(array<ShapeIndex> indices, array<ShapeVertex> vertices)
	{
		ShapeData shape_data = { indices, vertices };
		std::vector<ShapeVertex> verts = to_vector(vertices);

		using Context = SMikkTSpaceContext;

		SMikkTSpaceInterface intf = {};
		intf.m_getNumFaces = [](const Context* c) -> int { return mikkt_mesh(c).m_indices.m_count / 3; };
		intf.m_getNumVerticesOfFace = [](const Context* c, const int face) -> int { UNUSED(c); UNUSED(face); return 3; };
		intf.m_getPosition = [](const Context* c, float pos[], const int face, const int vert) { vec_to_array(mikkt_vertex(c, face, vert).m_position, pos); };
		intf.m_getNormal = [](const Context* c, float norm[], const int face, const int vert) { vec_to_array(mikkt_vertex(c, face, vert).m_normal, norm); };
		intf.m_getTexCoord = [](const Context* c, float uv[], const int face, const int vert) { vec_to_array(mikkt_vertex(c, face, vert).m_uv0, uv); };
		
		intf.m_setTSpaceBasic = [](const SMikkTSpaceContext * c, const float tangent[], const float sign, const int face, const int vert)
		{
			ShapeVertex& vertex = mikkt_vertex(c, face, vert);
			vertex.m_tangent = { tangent[0], tangent[1], tangent[2], sign };
		};

		SMikkTSpaceContext context;
		context.m_pInterface = &intf;
		context.m_pUserData = &shape_data;

		bool success = genTangSpaceDefault(&context) != 0;
		if(!success)
			printf("WARNING: Couldn't generate mikktspace tangents\n");
	}

	void MeshPacker::generate_tangents()
	{
		generate_mikkt_tangents(to_array(m_indices), to_array(m_vertices));
	}

	void MeshPacker::generate_indices()
	{
		m_indices.resize(m_vertices.size());

		for(ShapeIndex i = 0; i < ShapeIndex(m_vertices.size()); ++i)
			m_indices[i] = i;
	}
}
