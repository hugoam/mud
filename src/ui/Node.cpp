//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#include <ui/Node.h>
#include <ui/Structs/Node.h>
#include <ui/Structs/Container.h>

#include <obj/Vector.h>
#include <ui/ScrollSheet.h>
#include <ui/Cursor.h>
#include <ui/Style/Paint.h>

#include <ui/Render/Renderer.h>
#include <ui/Frame/Solver.h>

#include <ui/Style/Layout.h>

namespace mud
{
namespace ui
{
	template <class T, class U>
	array<T> to_array(std::vector<U>& vector) { return{ (T*)&vector[0], vector.size() }; }

	void draw_knob(const Frame& frame, const Colour& colour, bool connected, VgRenderer& renderer)
	{
		float radius = connected ? 5.f : 4.f;
		renderer.path_circle(frame.m_size / 2.f, radius);
		if(connected)
			renderer.fill({ colour });
		else
			renderer.stroke({ colour, 2.f });
	}


	void canvas_autolayout(Canvas& canvas)
	{
		Frame& plan = canvas.m_plan->m_frame;

		int min_index = 0;
		int max_index = 0;
		for(Node* node : canvas.m_nodes)
		{
			min_index = min(min_index, node->m_order);
			max_index = max(max_index, node->m_order);
		}

		int shift = -min(0, min_index);
		
		static Layout layout_overlay = [](Layout& l) { l.m_space = BOARD; };
		static Layout layout_line = [](Layout& l) { l.m_space = ITEM; l.m_align = { CENTER, CENTER }; l.m_padding = vec4(20.f); l.m_spacing = vec2(100.f); };
		static Layout layout_column = [](Layout& l) { l.m_space = UNIT; l.m_align = { LEFT, CENTER }; l.m_padding = vec4(20.f); l.m_spacing = vec2(20.f); };
		static Layout layout_node = [](Layout& l) { l.m_space = BLOCK; };

		SolverVector solvers;
		
		RowSolver overlay(plan.m_solver.get(), &layout_overlay);
		overlay.m_size = plan.m_size;
		solvers.push_back(&overlay);

		RowSolver line(&overlay, &layout_line);
		//RowSolver line(plan.m_solver.get(), &layout_line);
		solvers.push_back(&line);

		std::vector<RowSolver> columns; columns.reserve(canvas.m_nodes.size());
		std::vector<FrameSolver> elements; elements.reserve(canvas.m_nodes.size());

		for(int i = 0; i < max_index + shift + 1; ++i)
		{
			columns.emplace_back(&line, &layout_column);
			solvers.push_back(&columns.back());
		}

		for(size_t i = 0; i < canvas.m_nodes.size(); ++i)
		{
			elements.emplace_back(&columns[canvas.m_nodes[i]->m_order + shift], &layout_node, &canvas.m_nodes[i]->m_frame);
			elements.back().sync();
			solvers.push_back(&elements.back());
		}

		relayout(solvers);
	}

	void draw_node_cable(vec2 pos_out, vec2 pos_in, const Colour& colour_out, const Colour& colour_in, bool straight, VgRenderer& renderer)
	{
		float distance = straight ? 20.f : 100.f;
		Gradient paint = { colour_out, colour_in };
		renderer.path_bezier(pos_out, pos_out + vec2{ distance, 0.f }, pos_in - vec2{ distance, 0.f }, pos_in, straight);
		renderer.stroke_gradient(paint, 1.f, pos_out, pos_in);
	}

	Widget& node_knob(Widget& parent, Style& style, const Colour& colour, bool active, bool connected)
	{
		Widget& self = widget(parent, style);
		static Colour disabled_colour = Colour::DarkGrey;
		self.m_custom_draw = [=](const Frame& frame, const vec4& rect, VgRenderer& renderer) {  UNUSED(rect); draw_knob(frame, active ? colour : disabled_colour, connected, renderer); };
		return self;
	}

	Widget& canvas_cable(Widget& parent, vec2 out, vec2 in, const Colour& colour_out, const Colour& colour_in, bool straight = false)
	{
		Widget& self = widget(parent, node_styles().cable);
		self.m_frame.m_position = min(out, in);
		self.m_frame.m_size = max(out, in) - self.m_frame.m_position;
		self.m_custom_draw = [=](const Frame& frame, const vec4& rect, VgRenderer& renderer) {  UNUSED(rect); draw_node_cable(out - frame.m_position, in - frame.m_position, colour_out, colour_in, straight, renderer); };
		return self;
	}

	vec2 plug_at_out(Canvas& canvas, NodePlug& plug)
	{
		Frame& knob = plug.m_knob->m_frame;
		return knob.derive_position({ knob.m_size.x, knob.m_size.y / 2 }, canvas.m_plan->m_frame);
	}

	vec2 plug_at_in(Canvas& canvas, NodePlug& plug)
	{
		Frame& knob = plug.m_knob->m_frame;
		return knob.derive_position({ 0.f, knob.m_size.y / 2 }, canvas.m_plan->m_frame);
	}

	Widget& node_cable(Canvas& canvas, NodePlug& plug_out, NodePlug& plug_in)
	{
		return canvas_cable(*canvas.m_plan, plug_at_out(canvas, plug_out), plug_at_in(canvas, plug_in), plug_out.m_colour, plug_in.m_colour, !canvas.m_rounded_links);
	}

	NodePlug& node_plug(Node& node, cstring name, cstring icon, const Colour& colour, bool input, bool active, bool connected)
	{
		NodePlug& self = input ? twidget<NodePlug>(*node.m_inputs, node_styles().plug)
							   : twidget<NodePlug>(*node.m_outputs, node_styles().plug);
		self.m_node = &node;
		self.m_colour = colour;

		if(input)
			self.m_knob = &node_knob(self, node_styles().knob, colour, active, connected);

		label(self, name).setState(DISABLED, !active);
		UNUSED(icon);
		//item(self, icon);

		if(!input)
			self.m_knob = &node_knob(self, node_styles().knob_output, colour, active, connected);

		Canvas& canvas = *node.m_canvas;

		if(MouseEvent mouse_event = self.mouse_event(DeviceType::MouseLeft, EventType::Dragged))
		{
			Widget* target = static_cast<Widget*>(mouse_event.m_target);
			NodePlug* target_plug = nullptr;
			if(target && target->m_frame.d_style == &node_styles().plug && target != &self)
				target_plug = static_cast<NodePlug*>(target);

			canvas.m_connect.m_origin = &self;
			canvas.m_connect.m_in = input ? &self : target_plug;
			canvas.m_connect.m_out = input ? target_plug : &self;
			canvas.m_connect.m_position = mouse_event.m_pos;
		}

		if(MouseEvent mouse_event = self.mouse_event(DeviceType::MouseLeft, EventType::DragEnded))
		{
			canvas.m_connect.m_done = true;
		}

		return self;
	}

	Widget& node_header(Widget& parent, array<cstring> title)
	{
		Widget& self = multi_item(parent, node_styles().header, title);
		spacer(self);
		return self;
	}

	void canvas_clear_select(Canvas& canvas)
	{
		for(Node* selected : canvas.m_selection)
			selected->disableState(SELECTED);
		canvas.m_selection.clear();
	}

	void canvas_select(Canvas& canvas, Node& node)
	{
		canvas_clear_select(canvas);
		vector_select(canvas.m_selection, &node);
		node.enableState(SELECTED);
	}

	void canvas_swap_select(Canvas& canvas, Node& node)
	{
		bool selected = vector_swap(canvas.m_selection, &node);
		node.setState(SELECTED, selected);
	}

	template <class T>
	inline T& ttwidget(Widget& parent, Style& style, void* identity)
	{
		T& self = parent.sub<T>(identity); self.init(style); return self;
	}

	Node& node(Canvas& parent, array<cstring> title, int order, Ref identity)
	{
		Node& self = ttwidget<Node>(*parent.m_plan, node_styles().node, identity.m_value);
		self.layer();
		self.m_canvas = &parent;
		self.m_order = order;
		self.m_header = &node_header(self, title);

		Widget& plugs = widget(self, node_styles().plugs);
		self.m_inputs = &widget(plugs, node_styles().inputs);
		self.m_outputs = &widget(plugs, node_styles().outputs);

		self.m_body = &self;

		if(MouseEvent mouse_event = self.mouse_event(DeviceType::MouseLeft, EventType::Stroked, InputModifier::Shift))
			canvas_swap_select(parent, self);
		if(MouseEvent mouse_event = self.mouse_event(DeviceType::MouseLeft, EventType::Stroked))
			canvas_select(parent, self);
		if(MouseEvent mouse_event = self.mouse_event(DeviceType::MouseRight, EventType::Stroked))
			canvas_select(parent, self);

		if(MouseEvent mouse_event = self.mouse_event(DeviceType::MouseLeft, EventType::Dragged))
		{
			if(!vector_has(parent.m_selection, &self))
				canvas_select(parent, self);

			for(Node* node : parent.m_selection)
				node->m_frame.set_position(node->m_frame.m_position + mouse_event.m_delta / node->m_frame.absolute_scale());
		}

		self.m_index = parent.m_nodes.size();
		parent.m_nodes.push_back(&self);

		return self;
	}

	Node& node(Canvas& parent, array<cstring> title, float* position, int order, Ref identity)
	{
		Node& self = node(parent, title, order, identity);
		if(self.once())// && position != Zero2)
			self.m_frame.set_position({ position[0], position[1] });
		else
		{
			position[0] = self.m_frame.m_position.x;
			position[1] = self.m_frame.m_position.y;
		}
		return self;
	}

	Node& node(Canvas& parent, array<cstring> title, vec2& position, int order, Ref identity)
	{
		return node(parent, title, &position[0], order, identity);
	}

	Node& node(Canvas& parent, cstring title, vec2& position, int order, Ref identity)
	{
		return node(parent, carray<cstring, 1>{ title }, &position[0], order, identity);
	}

	Canvas& canvas(Widget& parent, size_t num_nodes) // , const Callback& context_trigger
	{
		Canvas& self = twidget<Canvas>(parent, canvas_styles().canvas);
		self.layer();

		self.m_scroll_plan = &scroll_plan(self);
		self.m_plan = self.m_scroll_plan->m_body;

		autofit_scroll_plan(*self.m_scroll_plan, to_array<Widget*>(self.m_nodes));

		//if(mouse_click_right(self) && context_trigger)
		//	context_trigger(self);

		if(MouseEvent mouse_event = self.m_scroll_plan->mouse_event(DeviceType::MouseLeft, EventType::Dragged))
		{
			for(Node* node : self.m_selection)
				node->m_frame.set_position(node->m_frame.m_position + mouse_event.m_delta / node->m_frame.absolute_scale());
		}

		if(MouseEvent mouse_event = self.m_scroll_plan->mouse_event(DeviceType::MouseLeft, EventType::Stroked))
			canvas_clear_select(self);

		if(MouseEvent mouse_event = self.m_scroll_plan->mouse_event(DeviceType::MouseMiddle, EventType::Stroked))
			canvas_autolayout(self);

		self.m_nodes.reserve(num_nodes);
		self.m_nodes.clear();

		return self;
	}

	NodeConnection canvas_connect(Canvas& canvas)
	{
		NodeConnection connection = {};
		
		CanvasConnect& connect = canvas.m_connect;
		if(connect.m_origin && connect.m_in && connect.m_out)
		{
			vec2 target = canvas.m_plan->m_frame.local_position(connect.m_position);
			Colour out_colour = connect.m_out ? connect.m_out->m_colour : connect.m_in->m_colour;
			Colour in_colour = connect.m_in ? connect.m_in->m_colour : connect.m_out->m_colour;

			vec2 out = connect.m_out ? plug_at_out(canvas, *connect.m_out) : target;
			vec2 in = connect.m_in ? plug_at_in(canvas, *connect.m_in) : target;

			canvas_cable(*canvas.m_plan, out, in, out_colour, in_colour);

			if(connect.m_done)
			{
				if(connect.m_out && connect.m_in)
					connection = { connect.m_out->m_node->m_index, connect.m_out->m_index,
								   connect.m_in->m_node->m_index,  connect.m_in->m_index };

				connect = {};
			}
		}
		else
		{
			canvas_cable(*canvas.m_plan, Zero2, Zero2, Colour::None, Colour::None);
			connect = {};
		}

		return connection;
	}
}
}
