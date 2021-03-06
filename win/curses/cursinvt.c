#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursinvt.h"

/* Permanent inventory for curses interface */

/* Runs when the game indicates that the inventory has been updated */
void curses_update_inv(void) {
	WINDOW *win = curses_get_nhwin(INV_WIN);

	/* Check if the inventory window is enabled in first place */
	if (!win) {
		/* It's not. Re-initialize the main windows if the
           option was enabled. */
		if (flags.perm_invent) {
			curses_create_main_windows();
			curses_last_messages();
			doredraw();
		}
		return;
	}

	bool border = curses_window_has_border(INV_WIN);

	/* Figure out drawing area */
	int x = 0;
	int y = 0;
	if (border) {
		x++;
		y++;
	}

	/* Clear the window as it is at the moment. */
	werase(win);

	wmove(win, y, x);
	attr_t attr = A_UNDERLINE;
	wattron(win, attr);
	wprintw(win, "Inventory:");
	wattroff(win, attr);

	/* The actual inventory will override this if we do carry stuff */
	wmove(win, y + 1, x);
	wprintw(win, "Not carrying anything");

	display_inventory(NULL, false);

	if (border)
		box(win, 0, 0);

	wnoutrefresh(win);
}

/* Adds an inventory item. */
void curses_add_inv(int y, int glyph, char accelerator, attr_t attr,
		    const char *str) {
	WINDOW *win = curses_get_nhwin(INV_WIN);
	int height, width;
	(void)height;
	getmaxyx(win, height, width);
	width -= 2;  // border
	height -= 2;

	/* Figure out where to draw the line */
	int x = 0;
	if (curses_window_has_border(INV_WIN)) {
		x++;
		y++;
	}

	wmove(win, y, x);
	if (accelerator) {
		attr_t bold = A_BOLD;
		wattron(win, bold);
		waddch(win, accelerator);
		wattroff(win, bold);
		wprintw(win, ") ");
		width -= 3;
	}

	if (accelerator && glyph != NO_GLYPH && iflags.use_menu_glyphs) {
		unsigned dummy; /* Not used */
		int color;
		glyph_t symbol;
		mapglyph(glyph, &symbol, &color, &dummy, u.ux, u.uy);
		attr_t glyphclr = curses_color_attr(color, 0);
		wattron(win, glyphclr);
		wprintw(win, "%s ", utf8_tmpstr(symbol));
		wattroff(win, glyphclr);
		width -= 2;
	}

	if (accelerator && /* Don't colorize categories */
	    iflags.use_menu_color) {
		int color = NO_COLOR;
		char str_mutable[BUFSZ];
		strcpy(str_mutable, str);
		attr = 0;
		get_menu_coloring(str_mutable, &color, &attr);
		if (color != NO_COLOR)
			attr |= curses_color_attr(color, 0);
	}

	wattron(win, attr);

	if ((strlen(str) > width) && ((iflags.graphics == UTF8_GRAPHICS) || (iflags.graphics == UTF8COMPAT_GRAPHICS)))
		wprintw(win, "%.*s…", width - 1, str);
	else
		wprintw(win, "%.*s", width, str);

	wattroff(win, attr);
	wclrtoeol(win);
}
