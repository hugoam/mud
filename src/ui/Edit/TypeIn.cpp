//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.


#include <ui/Edit/TypeIn.h>
#include <ui/Structs/Container.h>
#include <ui/ScrollSheet.h>

#include <obj/Vector.h>
#include <ui/Edit/Lang.h>

#include <ui/Frame/Frame.h>
#include <ui/Frame/Caption.h>
#include <ui/Structs/RootSheet.h>
#include <ui/Cursor.h>
#include <ui/Render/Renderer.h>
#include <ui/Style/Skin.h>

#include <cctype>
#include <locale>

#include <algorithm>
#include <chrono>
#include <string>
#include <regex>
#include <cmath>

#define MUD_TEXT_RECOLORIZE_ALL

namespace mud
{
	inline TextPaint style_text_paint(InkStyle& inkstyle)
	{
		return { inkstyle.m_text_font.c_str(), inkstyle.m_text_colour, inkstyle.m_text_size, inkstyle.m_align, inkstyle.m_text_break, inkstyle.m_text_wrap };
	}

	bool is_separator(char c)
	{
		string separators = " .,:;+-*=~!()[]{}'\"\t\n";
		return separators.find(c) != string::npos;
	}

	size_t word_begin(const string& text, size_t index)
	{
		size_t begin = SIZE_MAX;
		for(int pos = index; pos >= 0 && !is_separator(text[pos]); --pos)
			begin = size_t(pos);
		return begin;
	}

	size_t word_end(const string& text, size_t index)
	{
		size_t end = SIZE_MAX;
		for(size_t pos = index; pos < text.size() && !is_separator(text[pos]); ++pos)
			end = pos;
		return end;
	}

	string word_at(const string& text, size_t index)
	{
		size_t begin = word_begin(text, index);
		size_t end = word_end(text, index);
		return (begin != SIZE_MAX && end != SIZE_MAX) ? text.substr(begin, end + 1 - begin) : "";
	}

	size_t line_begin(const string& text, size_t index)
	{
		size_t begin = index;
		for(; text[begin - 1] != '\n' && begin > 0; --begin)
			;
		return begin;

	}

	size_t line_end(const string& text, size_t index)
	{
		size_t end = index;
		for(; text[end] != '\n' && end < text.size() - 1; ++end)
			;
		return end;
	}

	string text_line(const string& text, size_t index)
	{
		size_t begin = line_begin(text, index);
		size_t end = line_end(text, index);
		return text.substr(begin, end + 1 - begin);
	}

	size_t text_column(const string& text, size_t index)
	{
		return index - line_begin(text, index);
	}

	void replace_tabs(string& text, size_t tab_size)
	{
		for(size_t i = 0; i < text.size(); ++i)
			if(text[i] == '\t')
			{
				size_t num = tab_size - i % tab_size;
				for(size_t j = num; j > 0; --j)
					text.push_back(' ');
				i += num;
			}
	}

	VgRenderer* Text::s_renderer = nullptr;

	Text::Text(Frame& frame)
		: m_frame(frame)
		, m_text()
		, m_num_lines(0)
	{}

	void Text::set_lines(size_t lines)
	{
		m_num_lines = lines;
		//d_frame.mark_dirty(DIRTY_LAYOUT);
	}

	void Text::set_text(const string& text)
	{
		if(m_text == text) return;
		m_text = text;
		this->break_text_rows();
	}

	float Text::line_height() const
	{
		return s_renderer->line_height(m_text_paint);
	}

	vec2 Text::compute_text_size()
	{
		return { compute_width(), compute_height() };
	}
	
	float Text::compute_height() const
	{
		if(m_num_lines)
			return line_height() * m_num_lines;
		else if(!m_text_rows.empty())
			return m_text_rows.back().m_rect.y + rect_h(m_text_rows.back().m_rect);
		else
			return 0.f;
	}

	float Text::compute_width() const
	{
		float result = 0.f;
		for(const TextRow& row : m_text_rows)
			result = max(result, rect_w(row.m_rect));
		return result;
	}

	void Text::break_text_rows()
	{
		vec2 padded_size = floor(m_frame.m_size - rect_sum(m_frame.d_inkstyle->m_padding));

		if(!m_text.empty())
			s_renderer->break_text(m_text.c_str(), m_text.size(), padded_size, m_text_paint, m_text_rows);
		else
			m_text_rows.clear();
	}

	size_t Text::char_at(const vec2& pos) const
	{
		const char* start = m_text.c_str();
		const char* end = start + m_text.size();// - 1;

		for(const TextRow& row : m_text_rows)
			if(pos.y < row.m_rect.y + rect_h(row.m_rect)) // pos.y >= row.m_rect.y && 
			{
				for(const TextGlyph& glyph : row.m_glyphs)
					if(pos.x < glyph.m_rect.x + rect_w(glyph.m_rect) * 0.5f) // pos.x >= glyph.m_rect.x &&
						return glyph.m_position - start;

				return row.m_end - start;
			}

		return end - start;
	}

	vec4 Text::interval_rect(const TextRow& row, size_t start, size_t end) const
	{
		const TextGlyph& start_glyph = row.m_glyphs[start - row.m_start_index];
		const TextGlyph& end_glyph = row.m_glyphs[end - row.m_start_index];

		return { start_glyph.m_rect.x, row.m_rect.y, end_glyph.m_rect.x + rect_w(end_glyph.m_rect) - start_glyph.m_rect.x, rect_h(row.m_rect) };
	}

	vec4 Text::interval_rect(size_t start, size_t end) const
	{
		const TextRow& row = this->text_row_at(start);
		return this->interval_rect(row, start, end);
	}

	TextCursor Text::cursor_at(const vec2& pos) const
	{
		size_t index = this->char_at(pos);
		return this->to_cursor(index);
	}

	vec4 Text::cursor_rect(size_t index) const
	{
		const TextRow& row = text_row_at(index);

		if(index != row.m_end_index)
		{
			size_t offset = row.m_start - m_text.c_str();
			return row.m_glyphs[index - offset].m_rect;
		}
		else
		{
			return { vec2{ row.m_rect.x + rect_w(row.m_rect), row.m_rect.y }, vec2{ 1.f, line_height() } };
		}
	}

	size_t Text::text_row_index(size_t index) const
	{
		for(size_t i = 0; i < m_text_rows.size(); ++i)
			if(index <= m_text_rows[i].m_end_index)
				return i;
		return 0;
	}

	const TextRow& Text::text_row_at(size_t index) const
	{
		return m_text_rows[text_row_index(index)];
	}

	TextCursor Text::to_cursor(size_t index) const
	{
		size_t row = this->text_row_index(index);
		size_t column = m_text_rows.empty() ? 0U : index - m_text_rows[row].m_start_index;
		return{ index, { column, row} };
	}

	TextCursor Text::to_cursor(const uvec2& grid_index) const
	{
		size_t index = m_text_rows[grid_index.y].m_start_index + grid_index.x;
		return { index, grid_index };
	}

	TextCursor Text::clamp_cursor(int index) const
	{
		int clamped = max(0, min(int(m_text.size()) - 1, index));
		return to_cursor(clamped);
	}

	TextCursor Text::clamp_cursor(const ivec2& grid_index) const
	{
		int line = max(0, min(int(m_text_rows.size()) - 1, grid_index.y));
		int column = m_text_rows.empty() ? 0 : min(int(m_text_rows[line].m_glyphs.size()), grid_index.x);
		return to_cursor({ uint(column), uint(line)});
	}

	struct Clipboard
	{
		string m_text;
		bool m_line_mode;
	};

	static Clipboard s_clipboard;

	TextEdit::TextEdit(Widget* parent, void* identity, bool editor, string allowed_chars)
		: Widget(parent, identity)
		, m_editor(editor)
		, m_text(m_frame)
		, m_string(m_text.m_text)
		, m_dirty(0, m_string.size())
		, m_allowed_chars(allowed_chars)
	{
		m_palette = to_array(GetDarkPalette());
		m_language = make_unique<LanguageDefinition>(LanguageLua());
		//m_language = LanguageGLSL();
	}

	TextEdit::~TextEdit()
	{}

	size_t TextEdit::visible_lines() const
	{
		const Frame& scroll_frame = m_frame;
		float height = scroll_frame.m_size.y - 20.0f;
		return size_t(floor(height / m_text.line_height()));
	}

	void TextEdit::update_style()
	{
		m_text.m_text_paint = style_text_paint(*m_frame.d_inkstyle);

		if(m_editor)
		{
			m_text.m_text_paint.m_font = "consolas";
			m_text.m_text_paint.m_size = 14.f;
		}

		vec2 padding = floor(rect_offset(m_frame.d_inkstyle->m_padding));
		if(m_editor)
			m_text_offset = padding + vec2{ m_text.line_height() * 6.f * 0.6f, 0.f };
		else
			m_text_offset = padding;
	}

	void TextEdit::set_text(const string& text)
	{
		if(m_string == text) return;

		m_text.set_text(text);

		m_selection.m_cursor = m_text.to_cursor(min(size_t(m_selection.m_cursor), text.size()));
		m_selection.m_start = min(size_t(m_selection.m_start), text.size());
		m_selection.m_end = min(size_t(m_selection.m_end), text.size());

		//mUndoBuffer.clear();

		mark_dirty(0, m_string.size());
	}

	void TextEdit::shift(size_t start, size_t offset)
	{
		const char* begin = &m_string.front() + start;
		for(Text::ColorSection& section : m_text.m_sections)
			if(section.m_start >= begin)
			{
				section.m_start += offset;
				section.m_end += offset;
			}
#if 0
		for(auto& marker : m_markers)
			if(marker.first >= first)
				marker.first += shift;
#endif
	}

	void TextEdit::changed()
	{
		m_changed = true;
		//if(m_on_type)
		//	m_string = m_on_type(m_string);
	}

	void TextEdit::insert(size_t index, const string& text)
	{
		m_string.insert(index, text);
#ifdef MUD_TEXT_RECOLORIZE_ALL
		mark_dirty(0, m_string.length());
#else
		shift(index, text.length());
		mark_dirty(index, index + text.length());
#endif
		this->changed();
	}

	void TextEdit::insert(size_t index, const string& text, size_t cursor, Action& action)
	{
		action.mAdded = text;
		action.mAddedStart = index;
		action.mAddedEnd = index + text.size();

		this->insert(index, text);
		this->cursor(cursor);

		//shift_lines(num_lines);
	}

	void TextEdit::erase(size_t start, size_t end)
	{
		if(end == start) return;
		m_string.erase(start, end - start);
#ifdef MUD_TEXT_RECOLORIZE_ALL
		mark_dirty(0, m_string.length());
#else
		shift(start, start - end);
		mark_dirty(start, start);
#endif
		this->changed();
		//Colorize(mAddedStart.y - 1, mAddedEnd.y - mAddedStart.y + 2);
	}

	void TextEdit::erase(size_t start, size_t end, size_t cursor, Action& action)
	{
		action.mRemoved = m_string.substr(start, end - start);
		action.mRemovedStart = start;
		action.mRemovedEnd = end;

		this->erase(start, end);
		this->cursor(cursor);

		//shift_lines(-num_lines);
	}

	void TextEdit::enter()
	{
		//if(m_on_enter)
		//	this->set_text(m_on_enter(m_string));
		if(allowed('\n'))
			this->insert('\n');
	}

	void TextEdit::erase()
	{
		if(m_string.empty()) return;

		CommitAction([&](Action& action)
		{
			if(has_selection())
				erase_selected(action);
			else
				erase(m_selection.m_cursor, m_selection.m_cursor + 1, m_selection.m_cursor, action);
		});
	}

	void TextEdit::backspace()
	{
		if(m_string.empty()) return;

		CommitAction([&](Action& action)
		{
			if(has_selection())
				erase_selected(action);
			else
				erase(m_selection.m_cursor - 1, m_selection.m_cursor, m_selection.m_cursor - 1, action);
		});

		m_completing = m_completing && !is_separator(m_string[m_selection.m_cursor - 1]);
	}

	void TextEdit::insert(unsigned char c)
	{
		if(c == 0 || (!isprint(c) && !isspace(c)) || !allowed(c)) return;
		if(c == '\r') c = '\n';

		char last = m_selection.m_cursor < m_string.size() ? m_string[m_selection.m_cursor - 1] : 0;
		m_completing |= (last == 0 || is_separator(last)) && !is_separator(c);

		CommitAction([&](Action& action)
		{
			if(has_selection())
				erase_selected(action);

			insert(m_selection.m_cursor, string(1, c), m_selection.m_cursor + 1, action);
		});
	}

	void TextEdit::insert(const string& text)
	{
		CommitAction([&](Action& action)
		{
			if(has_selection())
				erase_selected(action);

			insert(m_selection.m_cursor, text, m_selection.m_cursor + text.size(), action);
		});
	}

	void TextEdit::copy()
	{
		if(has_selection())
			s_clipboard = { selected_text(), false };
		else
			s_clipboard = { text_line(m_string, m_selection.m_cursor), true };
	}

	void TextEdit::cut()
	{
		if(m_read_only) return;

		CommitAction([&](Action& action)
		{
			copy();
			if(has_selection())
				erase_selected(action);
			else
				erase(line_begin(m_string, m_selection.m_cursor), line_end(m_string, m_selection.m_cursor) + 1, m_selection.m_cursor, action);
		});
	}

	void TextEdit::paste()
	{
		if(s_clipboard.m_text.empty()) return;

		CommitAction([&](Action& action)
		{
			if(has_selection())
				erase_selected(action);

			if(s_clipboard.m_line_mode)
				insert(line_begin(m_string, m_selection.m_cursor), s_clipboard.m_text, m_selection.m_cursor + s_clipboard.m_text.size(), action);
			else
				insert(m_selection.m_cursor, s_clipboard.m_text, m_selection.m_cursor + s_clipboard.m_text.size(), action);
		});
	}

	void TextEdit::cursor(size_t index)
	{
		m_selection = { m_text.to_cursor(index), index, index, m_selection.m_insert_mode };
		m_frame.mark_dirty(DIRTY_REDRAW);
	}

	void TextEdit::select(size_t first, size_t second, bool word_mode)
	{
		m_selection = { m_text.to_cursor(second), min(first, second), max(first, second), m_selection.m_insert_mode };
		m_completing = false;

		if(word_mode)
		{
			m_selection.m_start = word_begin(m_string, m_selection.m_start);
			m_selection.m_end = word_end(m_string, m_selection.m_end);
		}

		if(m_selection.m_cursor != SIZE_MAX)
			m_follow_cursor = true;
		m_frame.mark_dirty(DIRTY_REDRAW);
	}

	void TextEdit::select(size_t caret, bool word_mode)
	{
		this->select(caret, caret, word_mode);
	}

	void TextEdit::move_select(TextCursor dest, bool select, bool word_mode)
	{
		if(select)
		{
			if(m_select_from.m_index == SIZE_MAX)
				m_select_from = m_selection.m_cursor;
			this->select(m_select_from, dest, word_mode);
		}
		else
		{
			m_select_from = {};
			this->select(dest, false);
		}

		m_follow_cursor = true;
	}

	void TextEdit::move_up(bool select)
	{
		move_select(m_text.clamp_cursor(ivec2(m_selection.m_cursor.m_grid_index) - ivec2(0, 1)), select);
	}

	void TextEdit::move_down(bool select)
	{
		move_select(m_text.clamp_cursor(ivec2(m_selection.m_cursor.m_grid_index) + ivec2(0, 1)), select);
	}

	void TextEdit::move_page_up(bool select)
	{
		move_select(m_text.clamp_cursor(ivec2(m_selection.m_cursor.m_grid_index) - ivec2(0, visible_lines() - 4)), select);
	}

	void TextEdit::move_page_down(bool select)
	{
		move_select(m_text.clamp_cursor(ivec2(m_selection.m_cursor.m_grid_index) + ivec2(0, visible_lines() - 4)), select);
	}

	void TextEdit::move_left(size_t count, bool select, bool word_mode)
	{
		move_select(m_text.clamp_cursor(m_selection.m_cursor - count), select, select && word_mode);
	}

	void TextEdit::move_right(size_t count, bool select, bool word_mode)
	{
		move_select(m_text.clamp_cursor(m_selection.m_cursor + count), select, select && word_mode);
	}
	
	void TextEdit::move_top(bool select)
	{
		move_select(m_text.to_cursor(0U), select);
	}

	void TextEdit::move_bottom(bool select)
	{
		move_select(m_text.to_cursor(m_string.size()), select);
	}

	void TextEdit::move_home(bool select)
	{
		move_select(m_text.to_cursor(line_begin(m_string, m_selection.m_cursor)), select);
	}

	void TextEdit::move_end(bool select)
	{
		move_select(m_text.to_cursor(line_end(m_string, m_selection.m_cursor)), select);
	}

	void TextEdit::select_none()
	{
		select(SIZE_MAX);
	}

	void TextEdit::select_all()
	{
		select(0U, m_string.size());
	}

	void TextEdit::select_word()
	{
		select(word_begin(m_string, m_selection.m_cursor), word_end(m_string, m_selection.m_cursor));
	}

	void TextEdit::AddUndo(Action& aValue)
	{
		mUndoStack.resize(mUndoIndex + 1);
		mUndoStack.back() = aValue;
		++mUndoIndex;
	}

	void TextEdit::update()
	{
		if(!focused())
			select_none();

		EventType event_focus = m_focus_mode == TextFocusMode::Press ? EventType::Pressed
																	 : EventType::Stroked;
		if(!this->focused())
			if(MouseEvent mouse_event = this->mouse_event(DeviceType::MouseLeft, event_focus, InputModifier::None, false))
			{
				take_focus();
				select(0);
			}

		auto shift = this->root_sheet().m_keyboard.m_shift;
		auto ctrl = this->root_sheet().m_keyboard.m_ctrl;
		auto alt = this->root_sheet().m_keyboard.m_alt;

		if(MouseEvent mouse_event = this->mouse_event(DeviceType::Mouse, EventType::Heartbeat))
		{
			size_t index = m_text.char_at(mouse_event.m_relative - m_text_offset);
			m_hovered_word = word_at(m_string, index);
			if(m_hovered_word != "")
			{
				m_hovered_word_rect = m_text.interval_rect(word_begin(m_string, index), word_end(m_string, index));
				m_hovered_word_rect = { rect_offset(m_hovered_word_rect) + m_text_offset, rect_size(m_hovered_word_rect) };
			}

			this->root_sheet().m_cursor_style = &ui::cursor_styles().caret;
		}

		if(!m_completing && !ctrl && !alt && this->key_event(KC_UP))
			move_up(shift);
		else if(!m_completing && !ctrl && !alt && this->key_event(KC_DOWN))
			move_down(shift);
		else if(!alt && this->key_event(KC_LEFT))
			move_left(1U, shift, ctrl);
		else if(!alt && this->key_event(KC_RIGHT))
			move_right(1U, shift, ctrl);
		else if(!alt && this->key_event(KC_PGUP))
			move_page_up(shift);
		else if(!alt && this->key_event(KC_PGDOWN))
			move_page_down(shift);
		else if(!alt && ctrl && this->key_event(KC_HOME))
			move_top(shift);
		else if(ctrl && !alt && this->key_event(KC_END))
			move_bottom(shift);
		else if(!ctrl && !alt && this->key_event(KC_HOME))
			move_home(shift);
		else if(!ctrl && !alt && this->key_event(KC_END))
			move_end(shift);
		else if(!m_read_only && this->key_event(KC_DELETE))
			erase();
		else if(!m_read_only && this->key_event(KC_BACK))
			backspace();
		else if(this->key_event(KC_INSERT, InputModifier::None))
			m_selection.m_insert_mode = !m_selection.m_insert_mode;
		else if(this->key_event(KC_INSERT, InputModifier::Ctrl)
			|| this->key_event(KC_C, InputModifier::Ctrl))
			copy();
		else if(!m_read_only && (this->key_event(KC_INSERT, InputModifier::Shift)
			|| this->key_event(KC_V, InputModifier::Ctrl)))
			paste();
		else if(this->key_event(KC_X, InputModifier::Ctrl)
			|| this->key_event(KC_DELETE, InputModifier::Shift))
			cut();
		else if(this->key_event(KC_A, InputModifier::Ctrl))
			select_all();
		else if(key_event(KC_RETURN))
			enter();
		else if(key_event(KC_ESCAPE))
			yield_focus();
		else if(key_event(KC_TAB) && !m_completing)
			insert(string(m_tab_size, ' '));
		else if(!m_read_only && !alt && !ctrl)
		{
			KeyEvent key_stroke = key_event(KC_UNASSIGNED);
			if(key_stroke)
				insert(key_stroke.m_char);
		}

		if(!m_read_only && (this->key_event(KC_Z, InputModifier::Ctrl)
						||  this->key_event(KC_BACK, InputModifier::Alt)))
			undo();
		if(!m_read_only && this->key_event(KC_Y, InputModifier::Ctrl))
			redo();

		if(this->focused() && !shift && !alt)
		{
			if(MouseEvent mouse_event = this->mouse_event(DeviceType::MouseLeft, EventType::Pressed))
			{
				m_select_from = m_text.cursor_at(mouse_event.m_relative - m_text_offset);
				select(m_select_from, ctrl);
			}
			if(MouseEvent mouse_event = this->mouse_event(DeviceType::MouseLeft, EventType::Pressed, InputModifier::Ctrl))
			{
				m_select_from = m_text.cursor_at(mouse_event.m_relative - m_text_offset);
				m_word_selection_mode = true;
				select(m_select_from, true);
			}
			if(MouseEvent mouse_event = this->mouse_event(DeviceType::MouseLeft, EventType::Released))
			{
				if(!focused())
					select_all();
			}
			if(MouseEvent mouse_event = this->mouse_event(DeviceType::MouseLeft, EventType::DoubleStroked))
			{
				TextCursor cursor = m_text.cursor_at(mouse_event.m_relative - m_text_offset);
				select(cursor, true);
			}
			if(MouseEvent mouse_event = this->mouse_event(DeviceType::MouseLeft, EventType::Dragged))
			{
				TextCursor cursor = m_text.cursor_at(mouse_event.m_relative - m_text_offset);
				select(m_select_from, cursor, m_word_selection_mode);
			}
		}

		if(!this->mouse_event(DeviceType::MouseLeft, EventType::Pressed))
			m_word_selection_mode = false;

		if(m_follow_cursor)
		{
			//scroll_to_cursor();
			m_follow_cursor = false;
		}

		if(m_dirty[0] != UINT32_MAX)
		{
			m_text.break_text_rows();
		}
	}

	Colour palette_colour(const ColourPalette& palette, PaletteIndex color_index)
	{
		return from_rgba(palette[color_index]);
	}

	Paint palette_paint(const ColourPalette& palette, PaletteIndex color_index)
	{
		return { from_rgba(palette[color_index]) };
	}

	TextPaint palette_text_paint(const Text& text, const ColourPalette& palette, PaletteIndex color_index)
	{
		TextPaint paint = text.m_text_paint;
		paint.m_colour = from_rgba(palette[color_index]);
		return paint;
	}

	void draw_text(VgRenderer& renderer, const vec2& padding, const Text& text)
	{
		for(const TextRow& row : text.m_text_rows)
			renderer.draw_text(padding + rect_offset(row.m_rect), row.m_start, row.m_end, text.m_text_paint);
	}

	void draw_editor_text(VgRenderer& renderer, const vec2& padding, const vec2& text_offset, const Text& text, const ColourPalette& palette)
	{
		char line_number[16];

		size_t line = 0;
		size_t current_section = 0;
		for(const TextRow& row : text.m_text_rows)
		{
			while(current_section < text.m_sections.size() && text.m_sections[current_section].m_start < row.m_start)
				current_section++;

			snprintf(line_number, 16, "%6d", int(++line));
			renderer.draw_text(padding + rect_offset(row.m_rect), line_number, nullptr, palette_text_paint(text, palette, Text::LineNumber));

			while(current_section < text.m_sections.size() && text.m_sections[current_section].m_start < row.m_end)
			{
				const Text::ColorSection& section = text.m_sections[current_section];

				vec2 position = text_offset + rect_offset(row.m_glyphs[section.m_start - row.m_start].m_rect);
				renderer.draw_text(floor(position) + vec2(0.f, 0.5f), section.m_start, section.m_end, palette_text_paint(text, palette, section.m_colour));

				current_section++;
			}
		}
	}

	void draw_text_selection(VgRenderer& renderer, const Frame& frame, const vec2& padding, const vec2& text_offset, const Text& text, const TextSelection& selection, const ColourPalette& palette, bool current_line)
	{
		if(text.m_text_rows.empty())
			return;

		for(const TextRow& row : text.m_text_rows)
		{
			if(selection.m_start != selection.m_end && row.m_end_index > selection.m_start && row.m_start_index < selection.m_end)
			{
				if(row.m_glyphs.empty())
				{
					renderer.draw_rect({ text_offset + rect_offset(row.m_rect), vec2{ 5.f, rect_h(row.m_rect) } }, palette_paint(palette, Text::Selection));
					continue;
				}

				size_t select_start = max(row.m_start_index, size_t(selection.m_start));
				size_t select_end = min(row.m_end_index, size_t(selection.m_end));

				vec4 row_rect = text.interval_rect(row, select_start, select_end - 1);
				renderer.draw_rect({ text_offset + rect_offset(row_rect), rect_size(row_rect) }, palette_paint(palette, Text::Selection));
			}

			if(selection.m_cursor >= row.m_start_index && selection.m_cursor <= row.m_end_index)
			{
				if(current_line && selection.m_start == selection.m_end)
				{
					vec4 rect = { padding + vec2{ 0.f, row.m_rect.y }, vec2{ frame.m_size.x, text.line_height() } };
					renderer.draw_rect(rect, { palette_colour(palette, Text::CurrentLineFillInactive),
											  palette_colour(palette, Text::CurrentLineEdge), 1.0f });
				}
				
				static Clock blink_clock;
				static bool caret_visible = true;
				if(blink_clock.read() > 0.4f)
				{
					caret_visible = !caret_visible;
					blink_clock.step();
				}
				
				if(caret_visible)
				{
					vec4 cursor_rect = text.cursor_rect(selection.m_cursor);
					if(!selection.m_insert_mode)
					{
						cursor_rect.x -= 1.f;
						cursor_rect.z = 1.f;
					}
					renderer.draw_rect({ text_offset + rect_offset(cursor_rect), rect_size(cursor_rect) }, palette_paint(palette, Text::Cursor));
				}
			}
		}
	}

	void TextEdit::render(VgRenderer& renderer)
	{
		if(m_editor)
			recolorize();

		if(m_editor)
			renderer.draw_rect({ m_frame.m_position, m_frame.m_size }, { palette_paint(m_palette, Text::Background) });
		else
			renderer.draw_background(m_frame, { m_frame.m_position, m_frame.m_size }, {}, {});

		vec2 padding = floor(rect_offset(m_frame.d_inkstyle->m_padding));

		draw_text_selection(renderer, m_frame, padding, m_text_offset, m_text, m_selection, m_palette, m_editor);
		if(m_editor)
			draw_editor_text(renderer, padding, m_text_offset, m_text, m_palette);
		else
			draw_text(renderer, padding, m_text);
	}

	void TextEdit::undo()
	{
		if(CanUndo())
			mUndoStack[--mUndoIndex].Undo(this);
	}

	void TextEdit::redo()
	{
		if(CanRedo())
			mUndoStack[mUndoIndex++].Redo(this);
	}

	void TextEdit::mark_dirty(size_t start, size_t end)
	{
		m_dirty[0] = min<uint>(m_dirty[0], start);
		m_dirty[1] = max<uint>(m_dirty[1], end);
	}

	void TextEdit::recolorize()
	{
		if(m_dirty[0] == UINT32_MAX) return;
		//colorize(0, m_string.size());
		colorize(m_dirty[0], m_dirty[1]);
		m_dirty = { UINT32_MAX, 0 };
	}

	void TextEdit::colorize(size_t from, size_t to)
	{
		if(m_string.empty())
			return;

		std::match_results<const char*> results;

		size_t begin = line_begin(m_string, from);
		size_t end = line_end(m_string, to);

		bool preproc = false;

		const char* start = &m_string.front() + begin;
		const char* last = &m_string.front() + end;

#ifdef MUD_TEXT_RECOLORIZE_ALL
		m_text.m_sections.clear();
		auto start_section = m_text.m_sections.end();
#else
		// remove all sections that overlap range to colorize, and get iterator to insert the new ones
		auto start_section = vector_remove_if(m_text.m_sections, [=](Text::ColorSection& section) { return section.m_start >= start && section.m_end < last; });
#endif

		for(const char* current = start; current != last; ++current)
		{
			for(auto& token_color : m_language->m_regex_tokens)
			{
				if(std::regex_search<const char*>(current, last, results, token_color.first, std::regex_constants::match_continuous))
				{
					auto match = *results.begin();
					string name = m_string.substr(match.first - start, match.second - match.first);
					PaletteIndex color = token_color.second;

					if(color == uint16_t(CodePalette::Identifier))
					{
						if(!m_language->m_case_sensitive)
							std::transform(name.begin(), name.end(), name.begin(), ::toupper);
						
						if(!preproc)
						{
							if(m_language->m_keywords.find(name) != m_language->m_keywords.end())
								color = uint16_t(CodePalette::Keyword);
							else if(m_language->m_identifiers.find(name) != m_language->m_identifiers.end())
								color = uint16_t(CodePalette::KnownIdentifier);
							else if(m_language->m_preproc_identifiers.find(name) != m_language->m_preproc_identifiers.end())
								color = uint16_t(CodePalette::PreprocIdentifier);
						}
						else
						{
							if(m_language->m_preproc_identifiers.find(name) != m_language->m_preproc_identifiers.end())
								color = uint16_t(CodePalette::PreprocIdentifier);
							else
								color = uint16_t(CodePalette::Identifier);
						}
					}
					else if(color == uint16_t(CodePalette::Preprocessor))
					{
						preproc = true;
					}

					Text::ColorSection section = { match.first, match.second, color };
					start_section = m_text.m_sections.insert(start_section, section) + 1;
					//m_text.m_sections.push_back(section);

					current += name.size() - 1;
					break;
				}
			}

			if(*current == '\n')
				preproc = false;
		}
	}

	void TextEdit::scroll_to_cursor()
	{
		//vec2 scroll = -m_frame.m_position;
		//vec2 size = m_frame.m_size;
		vec4 cursor_rect = m_text.cursor_rect(m_selection.m_cursor);
		vec2 cursor = { cursor_rect.x, cursor_rect.y };
		
		m_frame.m_position = min(m_frame.m_position, -cursor);
		//m_frame.m_position = max(m_frame.m_position, size - cursor);
	}

	void TextEdit::Action::Undo(TextEdit * aEditor)
	{
		if(!mAdded.empty())
			aEditor->erase(mAddedStart, mAddedEnd);

		if(!mRemoved.empty())
			aEditor->insert(mRemovedStart, mRemoved);
		
		aEditor->m_selection = mBefore;
		aEditor->m_follow_cursor = true;

	}

	void TextEdit::Action::Redo(TextEdit * aEditor)
	{
		if(!mRemoved.empty())
			aEditor->erase(mRemovedStart, mRemovedEnd);

		if(!mAdded.empty())
			aEditor->insert(mAddedStart, mAdded);

		aEditor->m_selection = mAfter;
		aEditor->m_follow_cursor = true;
	}


namespace ui
{
	bool filter(const string& filter, const string& value)
	{
		for(size_t i = 0; i < filter.size(); ++i)
			if(std::tolower(filter[i], std::locale()) != std::tolower(value[i], std::locale()))
				return false;
		return true;
	}

	TextEdit& text_box(Widget& parent, Style& style, string& text, bool editor, size_t lines, const string& allowed_chars)
	{
		TextEdit& self = parent.child_args<TextEdit, bool, string>(editor, allowed_chars);
		self.init(style);

		if(lines > 0)
			self.m_text.set_lines(lines);

		self.update_style();

		self.set_text(text);
		self.update();
		text = self.m_string;

		vec2 size = self.m_text.compute_text_size() + rect_sum(self.m_frame.d_inkstyle->m_padding);
		ui::dummy(self, size);
		//self.m_frame.m_content = self.m_text.compute_text_size();
		//self.m_frame.mark_dirty(DIRTY_LAYOUT);

		self.m_custom_draw = [&](const Frame& frame, const vec4& rect, VgRenderer& renderer) { UNUSED(frame); UNUSED(rect); self.render(renderer); };

		return self;
	}

	TextEdit& type_in(Widget& parent, string& text, size_t lines, const string& allowed_chars)
	{
		Widget& self = widget(parent, styles().type_in);
		TextEdit& edit = text_box(self, styles().type_in, text, false, lines, allowed_chars);
		return edit;
	}

	void autocomplete_popup(TextEdit& edit, string& text, const string& current_word, size_t cursor, size_t word_start, array<cstring> completions)
	{
		vec4 word_rect = edit.m_text.interval_rect(word_start, cursor - 1);
		vec2 popup_position = edit.m_text_offset + rect_offset(word_rect) + vec2(0.f, rect_h(word_rect));

		static size_t current = 0;

		bool selected = ui::popdown(edit, completions, current, popup_position, PopupFlags::None); //auto_complete_style

		if(edit.key_event(KC_UP))
			current = max(current - 1, size_t(0));

		if(edit.key_event(KC_DOWN))
			current = min(current + 1, completions.size() - 1);

		if(edit.key_event(KC_TAB) || selected)
		{
			edit.insert(string(completions[current]).substr(current_word.size()));
			edit.m_text.break_text_rows();
			edit.m_completing = false;
			text = edit.m_string;
		}
	}

	TextEdit& text_edit(Widget& parent, string& text, size_t lines, std::vector<string>* vocabulary)
	{
		Widget& self = widget(parent, styles().text_edit);
		ScrollSheet& scroll_sheet = ui::scroll_sheet(self);
		TextEdit& edit = text_box(*scroll_sheet.m_body, styles().type_zone, text, true, lines);

		if(vocabulary && edit.m_completing && !edit.has_selection())
		{
			size_t cursor = edit.m_selection.m_cursor;
			size_t begin = word_begin(edit.m_string, cursor - 1);

			string current_word = begin == SIZE_MAX ? "" : edit.m_string.substr(begin, cursor - begin);
			if(current_word != "")
			{
				std::vector<cstring> completions;
				for(const string& word : *vocabulary)
				{
					//if(word.find(current_word) != string::npos)
					// if current word is fits the beginning of word
					if(word.compare(0, current_word.size(), current_word) == 0)
						completions.push_back(word.c_str());
				}

				if(!completions.empty())
					autocomplete_popup(edit, text, current_word, cursor, begin, to_array(completions));
			}
		}

		return edit;
	}

	TextEdit& code_edit(Widget& parent, string& text, size_t lines, std::vector<string>* vocabulary)
	{
		return text_edit(parent, text, lines, vocabulary);
	}
}
}
