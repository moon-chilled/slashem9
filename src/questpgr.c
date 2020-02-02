/*	SCCS Id: @(#)questpgr.c	3.4	2000/05/05	*/
/*	Copyright 1991, M. Stephenson		  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"

/*  quest-specific pager routines. */

#include "qtext.h"

#define QTEXT_FILE "quest.txt"

static const char *intermed(void);
static const char *neminame(void);
static const char *guardname(void);
static const char *homebase(void);
static struct qtmsg *msg_in(struct qtmsg *qtm_list, int msgnum);
static void convert_arg(char c);
static void convert_line(void);
static void deliver_by_pline(struct qtmsg *qt_msg);
static void deliver_by_window(struct qtmsg *qt_msg, int how);
static bool should_skip_pager(bool common);


static char in_line[80], cvt_buf[64], out_line[128];
static struct qtlists qt_list;
static dlb *msg_file;
/* used by ldrname() and neminame(), then copied into cvt_buf */
static char nambuf[sizeof cvt_buf];

bool qt_comment(char *line) {
	return line[0] == '#' || strlen(line) == 1;
}

void load_qtlist(void) {
	if (!(msg_file = dlb_fopen(QTEXT_FILE, "r"))) {
		panic("CANNOT OPEN QUEST TEXT FILE %s.", QTEXT_FILE);
	}

	enum {
		in_msg,
		in_body,
	} state = in_body;

	char line[BUFSZ];
	char role_name[4];
	bool are_common = false;  // true => this is a message for everyone

	usize num_role_msgs[NUM_ROLES] = {0};
	usize num_common_msgs = 0;
	int id;
	usize role_no;

	usize line_no = 0;

	usize curr_pos, prev_pos;
#define get_qtlist (are_common ? qt_list.common : qt_list.chrole[role_no])
#define get_qtlen  (are_common ? num_common_msgs : num_role_msgs[role_no])
#define set_qtlen(val)                                 \
	do {                                           \
		usize _tmp = val;                      \
		if (are_common)                        \
			num_common_msgs = _tmp;        \
		else                                   \
			num_role_msgs[role_no] = _tmp; \
	} while (0)

	while (dlb_fgets(line, BUFSZ, msg_file)) {
		//pline("Fgetting (%zu) '%s'", line_no, line);
		line_no++;
		prev_pos = curr_pos;
		curr_pos = dlb_ftell(msg_file);
		if (qt_comment(line)) {
			continue;

		} else if (line[0] == '%' && line[1] == 'C') {
			if (state != in_body) {
				impossible("Bad quest file: control record encountered during message - line %zu\n", line_no);
				continue;
			}
			state = in_msg;
			if (sscanf(&line[4], "%3s %5d", role_name, &id) != 2) {
				impossible("Bad quest file: unrecognized control record - line %zu\n", line_no);
				continue;
			}
			if (!strcmp(role_name, "-")) {
				are_common = true;
			} else {
				role_no = str2role(role_name);
				if (role_no == ROLE_NONE) {
					impossible("Bad quest file: nonexistent role '%s' - line %zu", role_name, line_no);
					continue;
				}
			}

			set_qtlen(get_qtlen + 1);
			get_qtlist[get_qtlen - 1].msgnum = id;
			get_qtlist[get_qtlen - 1].delivery = line[2];
			get_qtlist[get_qtlen - 1].offset = curr_pos;
		} else if (line[0] == '%' && line[1] == 'E') {
			if (state != in_msg) {
				impossible("Bad quest file: end record encountered before message - line %zu\n", line_no);
				continue;
			}
			get_qtlist[get_qtlen - 1].size = prev_pos - get_qtlist[get_qtlen - 1].offset;
			state = in_body;
		} else {
			// pass: normal line of text
		}
	}

#undef get_qtlist
#undef get_qtlen
#undef set_qtlen
}

/* called at program exit */
void unload_qtlist(void) {
	if (msg_file)
		dlb_fclose(msg_file), msg_file = NULL;
}

short quest_info(int typ) {
	switch (typ) {
		case 0:
			return urole.questarti;
		case MS_LEADER:
			return urole.ldrnum;
		case MS_NEMESIS:
			return urole.neminum;
		case MS_GUARDIAN:
			return urole.guardnum;
		default:
			impossible("quest_info(%d)", typ);
	}
	return 0;
}

// return your role leader's name
const char *ldrname(void) {
	int i = urole.ldrnum;

	sprintf(nambuf, "%s%s",
		type_is_pname(&mons[i]) ? "" : "the ",
		mons[i].mname);
	return nambuf;
}

// return your intermediate target string
static const char *intermed(void) {
	return urole.intermed;
}

bool is_quest_artifact(struct obj *otmp) {
	return otmp->oartifact == urole.questarti;
}

// return your role nemesis' name
static const char *neminame(void) {
	int i = urole.neminum;

	sprintf(nambuf, "%s%s",
		type_is_pname(&mons[i]) ? "" : "the ",
		mons[i].mname);
	return nambuf;
}

// return your role leader's guard monster name
static const char *guardname(void) {
	int i = urole.guardnum;

	return mons[i].mname;
}

// return your role leader's location
static const char *homebase(void) {
	return urole.homebase;
}

static struct qtmsg *msg_in(struct qtmsg *qtm_list, int msgnum) {
	struct qtmsg *qt_msg;

	for (qt_msg = qtm_list; qt_msg->msgnum > 0; qt_msg++)
		if (qt_msg->msgnum == msgnum) return qt_msg;

	return NULL;
}

static void convert_arg(char c) {
	const char *str;

	switch (c) {
		case 'p':
			str = plname;
			break;
		case 'c':
			str = (flags.female && urole.name.f) ?
				      urole.name.f :
				      urole.name.m;
			break;
		case 'r':
			str = rank_of(u.ulevel, Role_switch, flags.female);
			break;
		case 'R':
			str = rank_of(MIN_QUEST_LEVEL, Role_switch,
				      flags.female);
			break;
		case 's':
			str = (flags.female) ? "sister" : "brother";
			break;
		case 'S':
			str = (flags.female) ? "daughter" : "son";
			break;
		case 'l':
			str = ldrname();
			break;
		case 'i':
			str = intermed();
			break;
		case 'o':
			str = the(artiname(urole.questarti));
			break;
		case 'n':
			str = neminame();
			break;
		case 'g':
			str = guardname();
			break;
		case 'G':
			str = align_gtitle(u.ualignbase[A_ORIGINAL]);
			break;
		case 'H':
			str = homebase();
			break;
		case 'a':
			str = align_str(u.ualignbase[A_ORIGINAL]);
			break;
		case 'A':
			str = align_str(u.ualign.type);
			break;
		case 'd':
			str = align_gname(u.ualignbase[A_ORIGINAL]);
			break;
		case 'D':
			str = align_gname(A_LAWFUL);
			break;
		case 'C':
			str = "chaotic";
			break;
		case 'N':
			str = "neutral";
			break;
		case 'L':
			str = "lawful";
			break;
		case 'x':
			str = Blind ? "sense" : "see";
			break;
		case 'Z':
			str = dungeons[0].dname;
			break;
		case '%':
			str = "%";
			break;
		default:
			str = "";
			break;
	}
	strcpy(cvt_buf, str);
}

static void convert_line(void) {
	char *c, *cc;

	cc = out_line;
	for (c = in_line; *c; c++) {
		*cc = 0;
		switch (*c) {
			case '\r':
			case '\n':
				*(++cc) = 0;
				return;

			case '%':
				if (*(c + 1)) {
					convert_arg(*(++c));
					switch (*(++c)) {
						/* insert "a"/"an" prefix */
						case 'A':
							strcat(cc, An(cvt_buf));
							cc += strlen(cc);
							continue; /* for */
						case 'a':
							strcat(cc, an(cvt_buf));
							cc += strlen(cc);
							continue; /* for */

						/* capitalize */
						case 'C':
							cvt_buf[0] = highc(cvt_buf[0]);
							break;

						/* pluralize */
						case 'P':
							cvt_buf[0] = highc(cvt_buf[0]);
						//fallthru
						case 'p':
							strcpy(cvt_buf, makeplural(cvt_buf));
							break;

						/* append possessive suffix */
						case 'S':
							cvt_buf[0] = highc(cvt_buf[0]);
						//fallthru
						case 's':
							strcpy(cvt_buf, s_suffix(cvt_buf));
							break;

						/* strip any "the" prefix */
						case 't':
							if (!strncmpi(cvt_buf, "the ", 4)) {
								strcat(cc, &cvt_buf[4]);
								cc += strlen(cc);
								continue; /* for */
							}
							break;

						default:
							--c; /* undo switch increment */
							break;
					}
					strcat(cc, cvt_buf);
					cc += strlen(cvt_buf);
					break;
				} /* else fall through */

			default:
				*cc++ = *c;
				break;
		}
	}
	if (cc >= out_line + sizeof out_line)
		panic("convert_line: overflow");
	*cc = 0;
	return;
}

static void deliver_by_pline(struct qtmsg *qt_msg) {
	long size;

	for (size = 0; size < qt_msg->size; size += (long)strlen(in_line)) {
		dlb_fgets(in_line, 80, msg_file);
		convert_line();
		plines(out_line);
	}
}

static void deliver_by_window(struct qtmsg *qt_msg, int how) {
	long size;
	winid datawin = create_nhwindow(how);

	for (size = 0; size < qt_msg->size; size += (long)strlen(in_line)) {
		dlb_fgets(in_line, 80, msg_file);
		convert_line();
		putstr(datawin, 0, out_line);
	}
	display_nhwindow(datawin, true);
	destroy_nhwindow(datawin);
}

bool should_skip_pager(bool common) {
	// WIZKIT: suppress plot feedback if starting with quest artifact
	if (program_state.wizkit_wishing) return true;

	if (!(common ? qt_list.common : qt_list.chrole[flags.initrole])) {
		panic("%s: no %s quest text data available",
		      common ? "com_pager" : "qt_pager",
		      common ? "common" : "role-specific");
	}

	return false;
}

void com_pager(int msgnum) {
	if (should_skip_pager(true)) return;

	struct qtmsg *qt_msg;

	if (!(qt_msg = msg_in(qt_list.common, msgnum))) {
		impossible("com_pager: message %d not found.", msgnum);
		return;
	}

	dlb_fseek(msg_file, qt_msg->offset, SEEK_SET);
	if (qt_msg->delivery == 'p')
		deliver_by_pline(qt_msg);
	else if (msgnum == 1)
		deliver_by_window(qt_msg, NHW_MENU);
	else
		deliver_by_window(qt_msg, NHW_TEXT);
	return;
}

void qt_pager(int msgnum) {
	if (should_skip_pager(false)) return;

	struct qtmsg *qt_msg;

	if (!(qt_msg = msg_in(qt_list.chrole[flags.initrole], msgnum))) {
		impossible("qt_pager: message %d not found.", msgnum);
		return;
	}

	dlb_fseek(msg_file, qt_msg->offset, SEEK_SET);
	if (qt_msg->delivery == 'p' && strcmp(windowprocs.name, "X11"))
		deliver_by_pline(qt_msg);
	else
		deliver_by_window(qt_msg, NHW_TEXT);
	return;
}

struct permonst *qt_montype(void) {
	int qpm;

	if (rn2(5)) {
		qpm = urole.enemy1num;
		if (qpm != NON_PM && rn2(5) && !(mvitals[qpm].mvflags & G_GENOD))
			return &mons[qpm];
		return mkclass(urole.enemy1sym, 0);
	}
	qpm = urole.enemy2num;
	if (qpm != NON_PM && rn2(5) && !(mvitals[qpm].mvflags & G_GENOD))
		return &mons[qpm];
	return mkclass(urole.enemy2sym, 0);
}

/*questpgr.c*/
