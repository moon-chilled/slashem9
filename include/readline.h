#ifndef READLINE_H
#define READLINE_H

struct readline_state {
	nhstr str;
	uint cursor;
	nhstr kill_buffer;
	bool just_killed;
};

void readline_process(struct readline_state*, int, bool*);

#endif //READLINE_H
