#include "hack.h"
#include "readline.h"

bool is_space(glyph_t g) {
	return g == ' ' || g == '\t';
}

usize word_back(struct readline_state *s) {
	usize nc = s->cursor;
	if (nc) nc--;
	while (nc && is_space(s->str.str[nc])) nc--;
	while (nc && !is_space(s->str.str[nc-1])) nc--;
	return nc;
}
usize word_forward(struct readline_state *s) {
	usize nc = s->cursor;
	if (nc < s->str.len) nc++;
	while (nc < s->str.len && is_space(s->str.str[nc])) nc++;
	while (nc < s->str.len && !is_space(s->str.str[nc])) nc++;
	return nc;
}

void readline_process(struct readline_state *s, int k, bool *beep) {
	nhstr kill_buffer = s->just_killed ? s->kill_buffer : new_nhs();
	s->just_killed = false;
	switch (k) {
		case K_CURSORLEFT:
		case CTRL_FLG|'b':
			if (s->cursor) s->cursor--;
			else *beep = true;
			break;
		case K_CURSORRIGHT:
		case CTRL_FLG|'f':
			if (s->cursor < s->str.len) s->cursor++;
			else *beep = true;
			break;
		case CTRL_FLG|'a':
			s->cursor = 0;
			break;
		case CTRL_FLG|'e':
			s->cursor = s->str.len;
			break;
		case META_FLG|'b':
			if (!s->cursor) *beep = true;
			else s->cursor = word_back(s);
			break;
		case META_FLG|'f':
			if (s->cursor >= s->str.len) *beep = true;
			else s->cursor = word_forward(s);
			break;
		case '\b':
		case CTRL_FLG|'h':
			if (s->cursor) {
				s->str = nhscat(nhstrim(s->str, s->cursor-1), nhslice(s->str, s->cursor));
				s->cursor--;
			}
			break;
		case K_DELETE:
		case CTRL_FLG|'d':
			if (s->cursor >= s->str.len || !s->str.len) {
				*beep = true;
				break;
			}
			s->str = nhscat(nhstrim(s->str, s->cursor), nhslice(s->str, s->cursor+1));
			break;
		case CTRL_FLG|'u':
			s->kill_buffer = nhscat(nhstrim(s->str, s->cursor), kill_buffer);
			s->str = nhslice(s->str, s->cursor);
			s->cursor = 0;
			s->just_killed = true;
			break;
		case CTRL_FLG|'k':
			s->kill_buffer = nhscat(kill_buffer, nhslice(s->str, s->cursor));
			s->str = nhstrim(s->str, s->cursor);
			s->cursor = s->str.len;
			s->just_killed = true;
			break;
		case CTRL_FLG|'w': {
			usize nc = word_back(s);
			s->kill_buffer = nhscat(nhslice(nhstrim(s->str, s->cursor), nc), kill_buffer);
			s->str = nhscat(nhstrim(s->str, nc), nhslice(s->str, s->cursor));
			s->cursor = nc;
			s->just_killed = true;
			break;
		}
		case META_FLG|'d': {
			usize nc = word_forward(s);
			s->kill_buffer = nhscat(kill_buffer, nhslice(nhstrim(s->str, nc), s->cursor));
			s->str = nhscat(nhstrim(s->str, s->cursor), nhslice(s->str, nc));
			s->just_killed = true;
			break;
		}

		case CTRL_FLG|'y':
			s->str = nhsins(s->str, s->cursor, s->kill_buffer);
			s->cursor += s->kill_buffer.len;
			break;


		default:
			if (k < 0) break;
			if (keyflags(k)) {
				*beep = true;
				break;
			}
			s->str = nhsinsg(s->str, s->cursor++, k);
			break;
	}
}
