/*	SCCS Id: @(#)extern.h	3.4	2003/03/10	*/
/* Copyright (c) Steve Creps, 1988.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

/* ### alloc.c ### */

extern void *alloc(usize);
extern void nhfree(const void*);
#define free nhfree
extern char *fmt_ptr(const void*,char *);

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### allmain.c ### */

extern void moveloop(void);
extern void stop_occupation(void);
extern void display_gamewindows(void);
extern void newgame(void);
extern void welcome(boolean);
extern time_t get_realtime(void);

/* ### apply.c ### */

extern int doapply(void);
extern int dorub(void);
extern int dojump(void);
extern int jump(int);
extern int jump(int);
extern int number_leashed(void);
extern void o_unleash(struct obj *);
extern void m_unleash(struct monst *,boolean);
extern void unleash_all(void);
extern boolean next_to_u(void);
extern struct obj *get_mleash(struct monst *);
extern void check_leash(struct monst *,xchar,xchar,boolean);
extern boolean um_dist(xchar,xchar,xchar);
extern boolean snuff_candle(struct obj *);
extern boolean snuff_lit(struct obj *);
extern boolean catch_lit(struct obj *);
extern void use_unicorn_horn(struct obj *);
extern boolean tinnable(struct obj *);
extern void reset_trapset(void);
extern void fig_transform(void *, long);
extern int unfixable_trouble_count(boolean);
extern int wand_explode(struct obj *,boolean);

/* ### artifact.c ### */

extern void init_artifacts(void);
extern void init_artifacts1(void);
extern void save_artifacts(int);
extern void restore_artifacts(int);
extern const char *artiname(int);
extern struct obj *mk_artifact(struct obj *,aligntyp);
extern const char *artifact_name(const char *,short *);
extern boolean exist_artifact(int,const char *);
extern void artifact_exists(struct obj *,const char *,boolean);
extern int nartifact_exist(void);
extern boolean spec_ability(struct obj *,unsigned long);
extern boolean confers_luck(struct obj *);
extern boolean arti_reflects(struct obj *);
extern boolean restrict_name(struct obj *,const char *);
extern boolean defends(int,struct obj *);
extern boolean protects(int,struct obj *);
extern void set_artifact_intrinsic(struct obj *,boolean,long);
extern int touch_artifact(struct obj *,struct monst *);
extern int spec_abon(struct obj *,struct monst *);
extern int spec_dbon(struct obj *,struct monst *,int);
extern void discover_artifact(xchar);
extern boolean undiscovered_artifact(xchar);
extern int disp_artifact_discoveries(winid);
extern boolean artifact_hit(struct monst *,struct monst *,
				struct obj *,int *,int);
extern int doinvoke(void);
extern void arti_speak(struct obj *);
extern boolean artifact_light(struct obj *);
extern int artifact_wet(struct obj *, boolean);
extern void arti_speak(struct obj *);
extern boolean artifact_light(struct obj *);
extern long spec_m2(struct obj *);
extern boolean artifact_has_invprop(struct obj *,uchar);
extern long arti_cost(struct obj *);

/* ### attrib.c ### */

extern boolean adjattrib(int,int,int);
extern void change_luck(schar);
extern int stone_luck(boolean);
extern void set_moreluck(void);
extern void gainstr(struct obj *,int);
extern void losestr(int);
extern void restore_attrib(void);
extern void exercise(int,boolean);
extern void exerchk(void);
extern void reset_attribute_clock(void);
extern void init_attr(int);
extern void redist_attr(void);
extern void adjabil(int,int);
extern int newhp(void);
extern schar acurr(int);
extern schar acurrstr(void);
extern void adjalign(int);
/* KMH, balance patch -- new function */
extern void recalc_health(void);
extern int uhp(void);
extern int uhpmax(void);

/* ### ball.c ### */

extern void ballfall(void);
extern void placebc(void);
extern void unplacebc(void);
extern void set_bc(boolean);
extern void move_bc(int,int,xchar,xchar,xchar,xchar);
extern boolean drag_ball(xchar,xchar,
		int *,xchar *,xchar *,xchar *,xchar *, boolean *,boolean);
extern void drop_ball(xchar,xchar);
extern void drag_down(void);

/* ### bones.c ### */

extern boolean can_make_bones(void);
extern void savebones(struct obj *);
extern int getbones(void);

/* ### borg.c ### */

extern bool borg_on;
extern char borg_line[80];
extern char borg_input(void);

/* ### botl.c ### */

extern int xlev_to_rank(int);
extern int title_to_mon(const char *,int *,int *);
extern void max_rank_sz(void);
extern long botl_score(void);
extern int describe_level(char *, int);
extern const char *rank_of(int,short,boolean);
extern void bot_set_handler(void (*)());
extern void bot_reconfig(void);
extern void bot(void);
#ifdef DUMP_LOG
extern void bot1str(char *);
extern void bot2str(char *);
#endif

#if 0
extern const char * shorten_bot1(const char *, int);
#endif
#ifdef TTY_GRAPHICS
extern const char * shorten_bot2(const char *, uint);
#endif

/* ### cmd.c ### */

extern char randomkey(void);
extern void reset_occupations(void);
extern void set_occupation(int (*)(void),const char *,int);
extern char pgetchar(void);
extern void pushch(char);
extern void savech(char);
extern char nhgetch(void);
extern void rhack(char *);
extern void dokeylist(void);
extern int doextlist(void);
extern int extcmd_via_menu(void);
extern void enlightenment(int);
extern void show_conduct(int);
#ifdef DUMP_LOG
extern void dump_enlightenment(int);
extern void dump_conduct(int);
#endif
extern int xytod(schar,schar);
extern void dtoxy(coord *,int);
extern int movecmd(char);
extern int getdir(const char *);
extern void confdir(void);
extern int isok(int,int);
extern int get_adjacent_loc(const char *, const char *, xchar, xchar, coord *);
extern const char *click_to_cmd(int,int,int);
extern char readchar(void);
extern void sanity_check(void);
extern void commands_init(void);
extern char *stripspace(char*);
extern void parsebindings(char*);
extern void parseautocomplete(char*,boolean);
extern void parsemappings(char*);
extern char txt2key(char*);
extern char *key2txt(char, char*);
extern char *str2txt(char*, char*);
extern char yn_function(const char *, const char *, char);

/* ### dbridge.c ### */

extern boolean is_pool(int,int);
extern boolean is_lava(int,int);
extern boolean is_ice(int,int);
extern int is_drawbridge_wall(int,int);
extern boolean is_db_wall(int,int);
extern boolean find_drawbridge(int *,int*);
extern boolean create_drawbridge(int,int,int,boolean);
extern void open_drawbridge(int,int);
extern void close_drawbridge(int,int);
extern void destroy_drawbridge(int,int);

/* ### decl.c ### */

extern void decl_init(void);
extern bool curses_stupid_hack;

/* ### detect.c ### */

extern struct obj *o_in(struct obj*,char);
extern struct obj *o_material(struct obj*,unsigned);
extern int gold_detect(struct obj *);
extern int food_detect(struct obj *);
extern int object_detect(struct obj *,int);
extern int monster_detect(struct obj *,int);
extern int trap_detect(struct obj *);
extern const char *level_distance(d_level *);
extern void use_crystal_ball(struct obj *);
extern void do_mapping(void);
extern void do_vicinity_map(void);
extern void cvt_sdoor_to_door(struct rm *);
extern int findit(void);
extern int openit(void);
extern void find_trap(struct trap *);
extern int dosearch0(int);
extern int dosearch(void);
extern void sokoban_detect(void);
/* KMH -- Sokoban levels */
extern void sokoban_detect(void);

/* ### dig.c ### */

extern boolean is_digging(void);
extern int holetime(void);
extern boolean dig_check(struct monst *, boolean, int, int);
extern void digactualhole(int,int,struct monst *,int);
extern boolean dighole(boolean);
extern int use_pick_axe(struct obj *);
extern int use_pick_axe2(struct obj *);
extern boolean mdig_tunnel(struct monst *);
extern void watch_dig(struct monst *,xchar,xchar,boolean);
extern void zap_dig(void);
extern struct obj *bury_an_obj(struct obj *);
extern void bury_objs(int,int);
extern void unearth_objs(int,int);
extern void rot_organic(void *, long);
extern void rot_corpse(void *, long);
#if 0
extern void bury_monst(struct monst *);
extern void bury_you(void);
extern void unearth_you(void);
extern void escape_tomb(void);
extern void bury_obj(struct obj *);
#endif

/* ### display.c ### */

extern struct obj *vobj_at(xchar,xchar);
extern void magic_map_background(xchar,xchar,int);
extern void map_background(xchar,xchar,int);
extern void map_trap(struct trap *,int);
extern void map_object(struct obj *,int);
extern void map_invisible(xchar,xchar);
extern void unmap_object(int,int);
extern void map_location(int,int,int);
extern int memory_glyph(int, int);
extern void clear_memory_glyph(int, int, int);
extern void feel_location(xchar,xchar);
extern void newsym(int,int);
extern void shieldeff(xchar,xchar);
extern void tmp_at(int,int);
#ifdef DISPLAY_LAYERS
extern int glyph_is_floating(int);
#endif
extern void swallowed(int);
extern void under_ground(int);
extern void under_water(int);
extern void see_monsters(void);
extern void set_mimic_blocking(void);
extern void see_objects(void);
extern void see_traps(void);
extern void curs_on_u(void);
extern int doredraw(void);
extern void docrt(void);
extern void show_glyph(int,int,int);
extern void clear_glyph_buffer(void);
extern void row_refresh(int,int,int);
extern void cls(void);
extern void flush_screen(int);
#ifdef DUMP_LOG
extern void dump_screen(void);
#endif
extern int back_to_glyph(xchar,xchar);
extern int zapdir_to_glyph(int,int,int);
extern int glyph_at(xchar,xchar);
extern void set_wall_state(void);

/* ### do.c ### */

extern int dodrop(void);
extern boolean boulder_hits_pool(struct obj *,int,int,boolean);
extern boolean flooreffects(struct obj *,int,int,const char *);
extern void doaltarobj(struct obj *);
extern boolean canletgo(struct obj *,const char *);
extern void dropx(struct obj *);
extern void dropy(struct obj *);
extern void obj_no_longer_held(struct obj *);
extern int doddrop(void);
extern int dodown(void);
extern int doup(void);
#ifdef INSURANCE
extern void save_currentstate(void);
#endif
extern void goto_level(d_level *,boolean,boolean,boolean);
extern void schedule_goto(d_level *,boolean,boolean,int,
			     const char *,const char *);
extern void deferred_goto(void);
extern boolean revive_corpse(struct obj *, boolean);
extern void revive_mon(void *, long);
extern void moldy_corpse(void *, long);
extern int donull(void);
extern int dowipe(void);
extern void set_wounded_legs(long,int);
extern void heal_legs(void);

/* ### do_name.c ### */

extern int getpos(coord *,boolean,const char *);
extern struct monst *christen_monst(struct monst *,const char *);
extern int do_mname(void);
extern struct obj *oname(struct obj *,const char *);
extern int ddocall(void);
extern void docall(struct obj *);
extern const char *rndghostname(void);
extern char *x_monnam(struct monst *,int,const char *,int,boolean);
extern char *l_monnam(struct monst *);
extern char *mon_nam(struct monst *);
extern char *noit_mon_nam(struct monst *);
extern char *Monnam(struct monst *);
extern char *noit_Monnam(struct monst *);
extern char *m_monnam(struct monst *);
extern char *y_monnam(struct monst *);
extern char *Adjmonnam(struct monst *,const char *);
extern char *Amonnam(struct monst *);
extern char *a_monnam(struct monst *);
extern char *distant_monnam(struct monst *,int,char *);
extern const char *rndmonnam(void);
extern const char *hcolor(const char *);
extern const char *rndcolor(void);
extern const char *roguename(void);
extern struct obj *realloc_obj(struct obj *, int, void *, int, const char *);
extern char *coyotename(struct monst *,char *);


/* ### do_wear.c ### */

extern int Armor_on(void);
extern int Boots_on(void);
extern int Cloak_on(void);
extern int Helmet_on(void);
extern int Gloves_on(void);
extern int Shield_on(void);
extern int Shirt_on(void);
extern void Amulet_on(void);
extern void off_msg(struct obj *);
extern void set_wear(void);
extern boolean donning(struct obj *);
extern void cancel_don(void);
extern int Armor_off(void);
extern int Armor_gone(void);
extern int Helmet_off(void);
extern int Gloves_off(void);
extern int Boots_off(void);
extern int Cloak_off(void);
extern int Shield_off(void);
extern int Shirt_off(void);
extern void Amulet_off(void);
extern void Ring_on(struct obj *);
extern void Ring_off(struct obj *);
extern void Ring_gone(struct obj *);
extern void Blindf_on(struct obj *);
extern void Blindf_off(struct obj *);
extern int dotakeoff(void);
extern int doremring(void);
extern int cursed(struct obj *);
extern int armoroff(struct obj *);
extern int canwearobj(struct obj *, long *, boolean);
extern int dowear(void);
extern int doputon(void);
extern void find_ac(void);
extern void glibr(void);
extern struct obj *some_armor(struct monst *);
extern void erode_armor(struct monst *,boolean);
extern struct obj *stuck_ring(struct obj *,int);
extern struct obj *unchanger(void);
extern void reset_remarm(void);
extern int doddoremarm(void);
extern int destroy_arm(struct obj *);
extern void adj_abon(struct obj *,schar);
extern int dowear2(const char *, const char *);

/* ### dog.c ### */

extern void initedog(struct monst *);
extern struct monst *make_familiar(struct obj *,xchar,xchar,boolean);
extern struct monst *make_helper(int,xchar,xchar);
extern struct monst *makedog(void);
extern void update_mlstmv(void);
extern void losedogs(void);
extern void mon_arrive(struct monst *,boolean);
extern void mon_catchup_elapsed_time(struct monst *,long);
extern void keepdogs(boolean);
extern void migrate_to_level(struct monst *,xchar,xchar,coord *);
extern int dogfood(struct monst *,struct obj *);
extern struct monst *tamedog(struct monst *,struct obj *);
extern int make_pet_minion(int,aligntyp);
extern void abuse_dog(struct monst *);
extern void wary_dog(struct monst *, boolean);

/* ### dogmove.c ### */

extern int dog_nutrition(struct monst *,struct obj *);
extern int dog_eat(struct monst *,struct obj *,int,int,boolean);
extern int dog_move(struct monst *,int);
extern boolean betrayed(struct monst *);

/* ### dokick.c ### */

extern boolean ghitm(struct monst *,struct obj *);
extern void container_impact_dmg(struct obj *);
extern int dokick(void);
extern boolean ship_object(struct obj *,xchar,xchar,boolean);
extern void obj_delivery(void);
extern schar down_gate(xchar,xchar);
extern void impact_drop(struct obj *,xchar,xchar,xchar);

/* ### dothrow.c ### */

extern struct obj *splitoneoff(struct obj **);
extern int dothrow(void);
extern int dofire(void);
extern void hitfloor(struct obj *);
extern void hurtle(int,int,int,boolean);
extern void mhurtle(struct monst *,int,int,int);
extern void throwit(struct obj *,long,boolean,int);
extern int omon_adj(struct monst *,struct obj *,boolean);
extern int thitmonst(struct monst *,struct obj *, int);
extern int hero_breaks(struct obj *,xchar,xchar,boolean);
extern int breaks(struct obj *,xchar,xchar);
extern boolean breaktest(struct obj *);
extern boolean walk_path(coord *, coord *, boolean (*)(void *,int,int), void *);
extern boolean hurtle_step(void *, int, int);

/* ### drawing.c ### */
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern int def_char_to_objclass(char);
extern int def_char_to_monclass(char);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void assign_graphics(const glyph_t *,int,int,int);
extern void switch_graphics(int);
extern void assign_rogue_graphics(boolean);
extern void assign_utf8graphics_symbol(int, glyph_t);
#ifdef USER_DUNGEONCOLOR
extern void assign_colors(uchar *,int,int,int);
#endif

/* ### dungeon.c ### */

extern void save_dungeon(int,boolean,boolean);
extern void restore_dungeon(int);
extern void insert_branch(branch *,boolean);
extern void init_dungeons(void);
extern s_level *find_level(const char *);
extern s_level *Is_special(d_level *);
extern branch *Is_branchlev(d_level *);
extern xchar ledger_no(d_level *);
extern xchar maxledgerno(void);
extern schar depth(d_level *);
extern xchar dunlev(d_level *);
extern xchar dunlevs_in_dungeon(d_level *);
extern xchar real_dunlevs_in_dungeon(d_level *);
extern xchar ledger_to_dnum(xchar);
extern xchar ledger_to_dlev(xchar);
extern xchar deepest_lev_reached(boolean);
extern boolean on_level(d_level *,d_level *);
extern void next_level(boolean);
extern void prev_level(boolean);
extern void u_on_newpos(int,int);
extern void u_on_sstairs(void);
extern void u_on_upstairs(void);
extern void u_on_dnstairs(void);
extern boolean On_stairs(xchar,xchar);
extern void get_level(d_level *,int);
extern boolean Is_botlevel(d_level *);
extern boolean Can_fall_thru(d_level *);
extern boolean Can_dig_down(d_level *);
extern boolean Can_rise_up(int,int,d_level *);
extern boolean In_quest(d_level *);
extern boolean In_mines(d_level *);
extern boolean In_spiders(d_level *);
extern branch *dungeon_branch(const char *);
extern boolean at_dgn_entrance(const char *);
extern boolean In_hell(d_level *);
extern boolean In_V_tower(d_level *);
extern boolean On_W_tower_level(d_level *);
extern boolean In_W_tower(int,int,d_level *);
extern void find_hell(d_level *);
extern void goto_hell(boolean,boolean);
extern void assign_level(d_level *,d_level *);
extern void assign_rnd_level(d_level *,d_level *,int);
extern int induced_align(int);
extern boolean Invocation_lev(d_level *);
extern xchar level_difficulty(void);
extern schar lev_by_name(const char *);
extern schar print_dungeon(boolean,schar *,xchar *);

/* ### eat.c ### */

extern boolean is_edible(struct obj *);
extern void init_uhunger(void);
extern int Hear_again(void);
extern void reset_eat(void);
extern int doeat(void);
extern void gethungry(void);
extern void morehungry(int);
extern void lesshungry(int);
extern boolean is_fainted(void);
extern void reset_faint(void);
extern void violated_vegetarian(void);
#if 0
extern void sync_hunger(void);
#endif
extern void newuhs(boolean);
extern boolean can_reach_floorobj(void);
extern void vomit(void);
extern int eaten_stat(int,struct obj *);
extern void food_disappears(struct obj *);
extern void food_substitution(struct obj *,struct obj *);
extern boolean bite_monster(struct monst *mon);
extern void fix_petrification(void);
extern void consume_oeaten(struct obj *,int);
extern boolean maybe_finished_meal(boolean);

/* ### end.c ### */

extern void done1(int);
extern int done2(void);
extern void done_in_by(struct monst *);
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern void panic(const char *,...) PRINTF_F(1,2);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void done(int);
extern void container_contents(struct obj *,boolean,boolean);
#ifdef DUMP_LOG
extern void dump(char *, char *);
extern void do_containerconts(struct obj *,boolean,boolean,boolean);
#endif
extern void terminate(int);
extern int dolistvanq(void);
extern int num_genocides(void);
/* KMH, ethics */
extern int doethics(void);


/* ### engrave.c ### */

extern char *random_engraving(char *);
extern void wipeout_text(char *,int,unsigned);
extern boolean can_reach_floor(void);
extern const char *surface(int,int);
extern const char *ceiling(int,int);
extern struct engr *engr_at(xchar,xchar);
extern int sengr_at(const char *,xchar,xchar);
extern void u_wipe_engr(int);
extern void wipe_engr_at(xchar,xchar,xchar);
extern boolean sense_engr_at(int,int,boolean);
extern void make_engr_at(int,int,const char *,long,xchar);
extern void del_engr_at(int,int);
extern boolean freehand(void);
extern int   doengrave(void);
extern void save_engravings(int,int);
extern void rest_engravings(int);
extern void del_engr(struct engr *);
extern void rloc_engr(struct engr *);
extern void make_grave(int,int,const char *);

/* ### exper.c ### */

extern long newuexp(int);
extern int experience(struct monst *,int);
extern void more_experienced(int,int);
extern void losexp(const char *, boolean);
extern void newexplevel(void);
extern void pluslvl(boolean);
extern long rndexp(boolean);

/* ### explode.c ### */

extern void explode(xchar,xchar,int,int,char,int);
extern long scatter(int, int, int, uint, struct obj *);
extern void splatter_burning_oil(int, int);
extern void grenade_explode(struct obj *, int, int, boolean, int);
extern void arm_bomb(struct obj *, boolean);

/* ### extralev.c ### */

extern void makeroguerooms(void);
extern void corr(int,int);
extern void makerogueghost(void);

/* ### files.c ### */

extern char *fname_encode(const char *, char, char *, char *, int);
extern char *fname_decode(char, char *, char *, int);
extern const char *fqname(const char *, int, int);
#ifndef FILE_AREAS
extern FILE *fopen_datafile(const char *,const char *,int);
#endif
extern boolean uptodate(int,const char *);
extern void store_version(int);
extern void set_levelfile_name(char *,int);
extern int create_levelfile(int,char *);
extern int open_levelfile(int,char *);
extern void delete_levelfile(int);
extern void clearlocks(void);
extern int create_bonesfile(d_level*,char **, char *);
extern void commit_bonesfile(d_level *);
extern int open_bonesfile(d_level*,char **);
extern boolean delete_bonesfile(d_level*);
extern void set_savefile_name(void);
#ifdef INSURANCE
extern void save_savefile_name(int);
#endif
extern void set_error_savefile(void);
extern int create_savefile(void);
extern int open_savefile(void);
extern int delete_savefile(void);
extern int restore_saved_game(void);
#ifndef FILE_AREAS
extern boolean lock_file(const char *,int,int);
extern void unlock_file(const char *);
#endif
#ifdef USER_SOUNDS
extern boolean can_read_file(const char *);
#endif
extern void read_config_file(const char *);
extern void check_recordfile(const char *);
extern int get_uchar_list(char*, uchar*, int);
extern void read_wizkit(void);
extern void paniclog(const char *, const char *);
extern int validate_prefix_locations(char *);
extern void free_saved_games(char**);
#ifdef SELF_RECOVER
extern boolean recover_savefile(void);
#endif
#ifdef HOLD_LOCKFILE_OPEN
extern void really_close(void);
#endif

/* ### fountain.c ### */

extern void floating_above(const char *);
extern void dogushforth(int);
extern void dryup(xchar,xchar, boolean);
extern void drinkfountain(void);
extern void dipfountain(struct obj *);
extern void whetstone_fountain_effects(struct obj *);
extern void diptoilet(struct obj *);
extern void breaksink(int,int);
extern void breaktoilet(int,int);
extern void drinksink(void);
extern void drinktoilet(void);
extern void whetstone_sink_effects(struct obj *);
extern void whetstone_toilet_effects(struct obj *);

/* ### gypsy.c ### */

extern void gypsy_init(struct monst *);
extern void gypsy_chat(struct monst *);


/* ### hack.c ### */

extern anything int_to_any(int);
extern anything uint_to_any(uint);
extern anything monst_to_any(struct monst*);
extern anything obj_to_any(struct obj*);
#ifdef DUNGEON_GROWTH
extern void catchup_dgn_growths(int);
extern void dgn_growths(boolean,boolean);
#endif
extern boolean revive_nasty(int,int,const char*);
extern void movobj(struct obj *,xchar,xchar);
extern boolean may_dig(xchar,xchar);
extern boolean may_passwall(xchar,xchar);
extern boolean bad_rock(struct monst *,xchar,xchar);
extern boolean invocation_pos(xchar,xchar);
extern boolean test_move(int, int, int, int, int);
extern void domove(void);
extern void invocation_message(void);
extern void spoteffects(boolean);
extern char *in_rooms(xchar,xchar,int);
extern boolean in_town(int,int);
extern void check_special_room(boolean);
extern int dopickup(void);
extern void lookaround(void);
extern int monster_nearby(void);
extern void nomul(int);
extern void unmul(const char *);
extern void showdmg(int);
extern void losehp(int,const char *, int);
extern int weight_cap(void);
extern int inv_weight(void);
extern int near_capacity(void);
extern int calc_capacity(int);
extern int max_capacity(void);
extern boolean check_capacity(const char *);
extern int inv_cnt(void);
#ifdef GOLDOBJ
extern long money_cnt(struct obj *);
#endif

/* ### hacklib.c ### */

extern boolean digit(char);
extern boolean letter(char);
extern char highc(char);
extern char lowc(char);
extern char *lcase(char *);
extern char *upstart(char *);
extern char *mungspaces(char *);
extern char *eos(char *);
extern char *strkitten(char *,char);
extern char *s_suffix(const char *);
extern boolean onlyspace(const char *);
extern char *tabexpand(char *);
extern char *visctrl(char);
extern const char *ordin(uint);
extern char *sitoa(int);
extern int sgn(int);
extern int rounddiv(long,int);
extern int dist2(int,int,int,int);
extern int distmin(int,int,int,int);
extern boolean online2(int,int,int,int);
extern boolean pmatch(const char *,const char *);
#ifndef STRNCMPI
extern int strncmpi(const char *,const char *, usize);
#endif
#ifndef STRSTRI
extern char *strstri(const char *,const char *);
#endif
extern boolean fuzzymatch(const char *,const char *,const char *,boolean);
extern void setrandom(void);
extern int getyear(void);
extern int getmonth(void);	/* KMH -- Used by gypsies */
#if 0
extern char *yymmdd(time_t);
#endif
extern long yyyymmdd(time_t);
extern int phase_of_the_moon(void);
extern boolean friday_13th(void);
extern boolean groundhog_day(void);	/* KMH -- February 2 */
extern int night(void);
extern int midnight(void);

/* ### invent.c ### */

extern void assigninvlet(struct obj *);
extern struct obj *merge_choice(struct obj *,struct obj *);
extern boolean merged(struct obj **,struct obj **);
extern void addinv_core1(struct obj *);
extern void addinv_core2(struct obj *);
extern struct obj *addinv(struct obj *);
extern struct obj *hold_another_object(struct obj *,const char *,const char *,const char *);
extern void useupall(struct obj *);
extern void useup(struct obj *);
extern void consume_obj_charge(struct obj *,boolean);
extern void freeinv_core(struct obj *);
extern void freeinv(struct obj *);
extern void delallobj(int,int);
extern void delobj(struct obj *);
extern struct obj *sobj_at(int,int,int);
extern struct obj *carrying(int);
extern boolean have_lizard(void);
extern struct obj *o_on(uint,struct obj *);
extern boolean obj_here(struct obj *,int,int);
extern boolean wearing_armor(void);
extern bool is_worn(struct obj *);
extern struct obj *g_at(int,int);
extern struct obj *mkgoldobj(long);
extern struct obj *getobj(const char *,const char *);
extern int ggetobj(const char *,int (*)(struct obj*),int,boolean,unsigned *);
extern void fully_identify_obj(struct obj *);
extern int identify(struct obj *);
extern void identify_pack(int);
extern int askchain(struct obj **,const char *,int,int (*)(struct obj*),
			int (*)(struct obj*),int,const char *);
extern void prinv(const char *,struct obj *,long);
extern char *xprname(struct obj *,const char *,char,boolean,long,long);
extern int ddoinv(void);
extern char display_inventory(const char *,boolean);
#ifdef DUMP_LOG
extern char dump_inventory(const char *,boolean);
#endif
extern int display_binventory(int,int,boolean);
extern struct obj *display_cinventory(struct obj *);
extern struct obj *display_minventory(struct monst *,int,char *);
extern int dotypeinv(void);
extern const char *dfeature_at(int,int,char *);
extern int look_here(int,boolean);
extern int dolook(void);
extern boolean will_feel_cockatrice(struct obj *,boolean);
extern void feel_cockatrice(struct obj *,boolean);
extern void stackobj(struct obj *);
extern int doprgold(void);
extern int doprwep(void);
extern int doprarm(void);
extern int doprring(void);
extern int dopramulet(void);
extern int doprtool(void);
extern int doprinuse(void);
extern void useupf(struct obj *,long);
extern char *let_to_name(char,boolean);
extern void free_invbuf(void);
extern void reassign(void);
extern int doorganize(void);
extern int count_unpaid(struct obj *);
extern int count_buc(struct obj *,int);
extern void carry_obj_effects(struct monst *, struct obj *);
extern const char *currency(long);
extern void silly_thing(const char *,struct obj *);
extern int doinvinuse(void);
/* KMH, balance patch -- new function */
extern int jumble_pack(void);

/* ### ioctl.c ### */

#ifdef UNIX
extern void getwindowsz(void);
extern void getioctls(void);
extern void setioctls(void);
# ifdef SUSPEND
extern int dosuspend(void);
# endif /* SUSPEND */
#endif /* UNIX */

/* ### light.c ### */

extern void new_light_source(xchar, xchar, int, int, anything);
extern void del_light_source(int, anything);
extern void do_light_sources(char **);
extern struct monst *find_mid(unsigned, unsigned);
extern void save_light_sources(int, int, int);
extern void restore_light_sources(int);
extern void relink_light_sources(boolean);
extern void obj_move_light_source(struct obj *, struct obj *);
extern boolean any_light_source(void);
extern void snuff_light_source(int, int);
extern boolean obj_sheds_light(struct obj *);
extern boolean obj_is_burning(struct obj *);
extern boolean obj_permanent_light(struct obj *);
extern void obj_split_light_source(struct obj *, struct obj *);
extern void obj_merge_light_sources(struct obj *,struct obj *);
extern int candle_light_range(struct obj *);
extern int wiz_light_sources(void);

/* ### lock.c ### */

extern boolean picking_lock(int *,int *);
extern boolean picking_at(int,int);
extern void reset_pick(void);
extern int pick_lock(struct obj **);
extern int doforce(void);
extern boolean boxlock(struct obj *,struct obj *);
extern boolean doorlock(struct obj *,int,int);
extern int doopen(void);
extern int doclose(void);
extern int artifact_door(int,int);

#ifdef MAC
/* These declarations are here because the main code calls them. */

/* ### macfile.c ### */

extern int maccreat(const char *,long);
extern int macopen(const char *,int,long);
extern int macclose(int);
extern int macread(int,void *,unsigned);
extern int macwrite(int,void *,unsigned);
extern long macseek(int,long,short);
extern int macunlink(const char *);

/* ### macsnd.c ### */

extern void mac_speaker(struct obj *,char *);

/* ### macunix.c ### */

extern void regularize(char *);
extern void getlock(void);

/* ### macwin.c ### */

extern void lock_mouse_cursor(Boolean);
extern int SanePositions(void);

/* ### mttymain.c ### */

extern void getreturn(const char *);
extern void msmsg(const char *,...);
extern void gettty(void);
extern void setftty(void);
extern void settty(const char *);
extern int tgetch(void);
extern void cmov(int x, int y);
extern void nocmov(int x, int y);

#endif /* MAC */

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
extern void getmailstatus(void);
# endif
extern void ckmailstatus(void);
extern void readmail(struct obj *);
#endif /* MAIL */

/* ### makemon.c ### */

extern boolean is_home_elemental(struct permonst *);
extern struct monst *clone_mon(struct monst *,xchar,xchar);
extern struct monst *makemon(struct permonst *,int,int,int);
extern boolean create_critters(int,struct permonst *);
extern struct permonst *rndmonst(void);
extern void reset_rndmonst(int);
extern struct permonst *mkclass(char,int);
extern int pm_mkclass(char,int);
extern int adj_lev(struct permonst *);
extern struct permonst *grow_up(struct monst *,struct monst *);
extern int mongets(struct monst *,int);
extern int golemhp(int);
extern boolean peace_minded(struct permonst *);
extern void set_malign(struct monst *);
extern void set_mimic_sym(struct monst *);
extern int mbirth_limit(int);
extern void mimic_hit_msg(struct monst *, short);
#ifdef GOLDOBJ
extern void mkmonmoney(struct monst *, long);
#endif
extern void bagotricks(struct obj *);
extern boolean propagate(int, boolean,boolean);

/* ### mapglyph.c ### */

extern void mapglyph(int, glyph_t *, int *, unsigned *, int, int);

/* ### mcastu.c ### */

extern int castmu(struct monst *,struct attack *,boolean,boolean);
extern bool buzzmu(struct monst *,struct attack *);

/* ### mhitm.c ### */

extern int  fightm(struct monst *);
extern int  mattackm(struct monst *,struct monst *);
extern bool noattacks(struct permonst *);
extern int  sleep_monst(struct monst *,int,int);
extern void slept_monst(struct monst *);
extern long attk_protection(int);

/* ### mhitu.c ### */

extern const char *mpoisons_subj(struct monst *,struct attack *);
extern void u_slow_down(void);
extern struct monst *cloneu(void);
extern void expels(struct monst *,struct permonst *,boolean);
extern struct attack *getmattk(struct permonst *,int,int *,struct attack *);
extern int mattacku(struct monst *);
extern int magic_negation(struct monst *);
extern int gazemu(struct monst *,struct attack *);
extern void mdamageu(struct monst *,int);
extern int could_seduce(struct monst *,struct monst *,struct attack *);
extern int doseduce(struct monst *);

/* ### minion.c ### */

extern void msummon(struct monst *);
extern void summon_minion(aligntyp,boolean);
extern bool demon_talk(struct monst *);
extern int lawful_minion(int);
extern int neutral_minion(int);
extern int chaotic_minion(int);
extern long bribe(struct monst *);
extern int dprince(aligntyp);
extern int dlord(aligntyp);
extern int llord(void);
extern int ndemon(aligntyp);
extern int lminion(void);

/* ### mklev.c ### */

extern void sort_rooms(void);
extern void add_room(int,int,int,int,boolean,schar,boolean);
extern void add_subroom(struct mkroom *,int,int,int,int,
			   boolean,schar,boolean);
extern void makecorridors(void);
extern int add_door(int,int,struct mkroom *);
extern void mklev(void);
#ifdef SPECIALIZATION
extern void topologize(struct mkroom *,boolean);
#else
extern void topologize(struct mkroom *);
#endif
extern void place_branch(branch *,xchar,xchar);
extern boolean occupied(xchar,xchar);
extern int okdoor(xchar,xchar);
extern void dodoor(int,int,struct mkroom *);
extern void mktrap(int,int,struct mkroom *,coord*);
extern void mkstairs(xchar,xchar,char,struct mkroom *);
extern void mkinvokearea(void);

/* ### mkmap.c ### */

void flood_fill_rm(int,int,int,boolean,boolean);
void remove_rooms(int,int,int,int);

/* ### mkmaze.c ### */

extern void wallification(int,int,int,int, boolean);
extern void walkfrom(int,int);
extern void makemaz(const char *);
extern void mazexy(coord *);
extern void bound_digging(void);
extern void mkportal(xchar,xchar,xchar,xchar);
extern boolean bad_location(xchar,xchar,xchar,xchar,xchar,xchar);
extern void place_lregion(xchar,xchar,xchar,xchar,
			     xchar,xchar,xchar,xchar,
			     xchar,d_level *);
extern void movebubbles(void);
extern void water_friction(void);
extern void save_waterlevel(int,int);
extern void restore_waterlevel(int);
extern const char *waterbody_name(xchar,xchar);

/* ### mkobj.c ### */

extern struct obj *mkobj_at(char,int,int,boolean);
extern struct obj *mksobj_at(int,int,int,boolean,boolean);
extern struct obj *mkobj(char,boolean);
extern int rndmonnum(void);
extern struct obj *splitobj(struct obj *,long);
extern void replace_object(struct obj *,struct obj *);
extern void bill_dummy_object(struct obj *);
extern struct obj *mksobj(int,boolean,boolean);
extern int bcsign(struct obj *);
extern int weight(struct obj *);
extern struct obj *mkgold(long,int,int);
extern struct obj *mkcorpstat(int,struct monst *,struct permonst *,int,int,boolean);
extern struct obj *obj_attach_mid(struct obj *, unsigned);
extern struct monst *get_mtraits(struct obj *, boolean);
extern struct obj *mk_tt_object(int,int,int);
extern struct obj *mk_named_object(int,struct permonst *,int,int,const char *);
extern struct obj *rnd_treefruit_at(int, int);
extern void start_corpse_timeout(struct obj *);
extern void bless(struct obj *);
extern void unbless(struct obj *);
extern void curse(struct obj *);
extern void uncurse(struct obj *);
extern void blessorcurse(struct obj *,int);
extern boolean is_flammable(struct obj *);
extern boolean is_rottable(struct obj *);
extern void place_object(struct obj *,int,int);
extern void remove_object(struct obj *);
extern void discard_minvent(struct monst *);
extern void obj_extract_self(struct obj *);
extern struct obj *container_extract_indestructable(struct obj *obj);
extern void extract_nobj(struct obj *, struct obj **);
extern void extract_nexthere(struct obj *, struct obj **);
extern int add_to_minv(struct monst *, struct obj *);
extern struct obj *add_to_container(struct obj *, struct obj *);
extern void add_to_migration(struct obj *);
extern void add_to_buried(struct obj *);
extern void dealloc_obj(struct obj *);
extern void obj_ice_effects(int, int, boolean);
extern long peek_at_iced_corpse_age(struct obj *);
extern void obj_sanity_check(void);

/* ### mkroom.c ### */

extern void mkroom(int);
extern void fill_zoo(struct mkroom *);
extern boolean nexttodoor(int,int);
extern boolean has_dnstairs(struct mkroom *);
extern boolean has_upstairs(struct mkroom *);
extern int somex(struct mkroom *);
extern int somey(struct mkroom *);
extern boolean inside_room(struct mkroom *,xchar,xchar);
extern boolean somexy(struct mkroom *,coord *);
extern void mkundead(coord *,boolean,int);
extern struct permonst *courtmon(void);
extern struct permonst *antholemon(void);
extern struct permonst *realzoomon(void);
extern void save_rooms(int);
extern void rest_rooms(int);
extern struct mkroom *search_special(schar);

/* ### mon.c ### */

extern int undead_to_corpse(int);
extern int genus(int,int);
extern int pm_to_cham(int);
extern int minliquid(struct monst *);
extern int movemon(void);
extern int meatmetal(struct monst *);
extern void meatcorpse(struct monst *);
extern int meatobj(struct monst *);
extern void mpickgold(struct monst *);
extern boolean mpickstuff(struct monst *,const char *);
extern int curr_mon_load(struct monst *);
extern int max_mon_load(struct monst *);
extern boolean can_carry(struct monst *,struct obj *);
extern int mfndpos(struct monst *,coord *,long *,long);
extern boolean monnear(struct monst *,int,int);
extern void dmonsfree(void);
extern int mcalcmove(struct monst*);
extern void mcalcdistress(void);
extern void replmon(struct monst *,struct monst *);
extern void relmon(struct monst *);
extern struct obj *mlifesaver(struct monst *);
extern boolean corpse_chance(struct monst *,struct monst *,boolean);
extern void mondead(struct monst *);
extern void mondied(struct monst *);
extern void mongone(struct monst *);
extern void monstone(struct monst *);
extern void monkilled(struct monst *,const char *,int);
extern void mon_xkilled(struct monst *,const char *,int);
extern void unstuck(struct monst *);
extern void killed(struct monst *);
extern void xkilled(struct monst *,int);
extern void mon_to_stone(struct monst*);
extern void mnexto(struct monst *);
extern boolean mnearto(struct monst *,xchar,xchar,boolean);
extern void poisontell(int);
extern void poisoned(const char *,int,const char *,int);
extern void m_respond(struct monst *);
extern void setmangry(struct monst *);
extern void wakeup(struct monst *);
extern void wake_nearby(void);
extern void wake_nearto(int,int,int);
extern void seemimic(struct monst *);
extern void rescham(void);
extern void restartcham(void);
extern void restore_cham(struct monst *);
extern void mon_animal_list(boolean);
extern int newcham(struct monst *,struct permonst *,boolean,boolean);
extern int can_be_hatched(int);
extern int egg_type_from_parent(int,boolean);
extern boolean dead_species(int,boolean);
extern void kill_genocided_monsters(void);
extern void golemeffects(struct monst *,int,int);
extern boolean angry_guards(boolean);
extern void pacify_guards(void);

/* ### mondata.c ### */

extern void set_mon_data(struct monst *,struct permonst *,int);
extern struct attack *attacktype_fordmg(struct permonst *,int,int);
extern boolean attacktype(struct permonst *,int);
extern boolean poly_when_stoned(struct permonst *);
extern boolean resists_drli(struct monst *);
extern boolean resists_magm(struct monst *);
extern boolean resists_blnd(struct monst *);
extern boolean can_blnd(struct monst *,struct monst *,uchar,struct obj *);
extern boolean ranged_attk(struct permonst *);
extern boolean passes_bars(struct permonst *);
extern boolean can_track(struct permonst *);
extern boolean breakarm(struct permonst *);
extern boolean sliparm(struct permonst *);
extern boolean sticks(struct permonst *);
extern int num_horns(struct permonst *);
/* extern boolean canseemon(struct monst *); */
extern struct attack *dmgtype_fromattack(struct permonst *,int,int);
extern boolean dmgtype(struct permonst *,int);
extern int max_passive_dmg(struct monst *,struct monst *);
extern int monsndx(struct permonst *);
extern int name_to_mon(const char *);
extern int gender(struct monst *);
extern int pronoun_gender(struct monst *);
extern boolean levl_follower(struct monst *);
extern int little_to_big(int);
extern int big_to_little(int);
extern const char *locomotion(const struct permonst *,const char *);
extern const char *stagger(const struct permonst *,const char *);
extern const char *on_fire(struct permonst *,struct attack *);
extern const struct permonst *raceptr(struct monst *);

/* ### monmove.c ### */

extern boolean itsstuck(struct monst *);
extern boolean mb_trapped(struct monst *);
extern void mon_regen(struct monst *,boolean);
extern int dochugw(struct monst *);
extern boolean onscary(int,int,struct monst *);
extern void monflee(struct monst *, int, boolean, boolean);
extern int dochug(struct monst *);
extern int m_move(struct monst *,int);
extern boolean closed_door(int,int);
extern boolean accessible(int,int);
extern void set_apparxy(struct monst *);
extern boolean can_ooze(struct monst *);

/* ### monst.c ### */

extern void monst_init(void);

/* ### monstr.c ### */
extern void monstr_init(void);
extern int mstrength(struct permonst mon);

/* ### mplayer.c ### */

extern struct monst *mk_mplayer(struct permonst *,xchar, xchar,boolean);
extern void create_mplayers(int,boolean);
extern void mplayer_talk(struct monst *);

/* ### unicode.c ### */
extern void unicode_to_utf8(glyph_t, char[5]);
extern char *utf8_tmpstr(glyph_t);
extern void pututf8char(glyph_t);


#ifdef WIN32

/* ### winnt.c ### */

#  ifndef WIN32
extern int tgetch(void);
#  endif
extern char switchar(void);
# ifndef __GO32__
extern long freediskspace(char *);
extern int findfirst(char *);
extern int findnext(void);
extern long filesize(char *);
extern char *foundfile_buffer(void);
# endif /* __GO32__ */
# ifndef __CYGWIN__
extern void chdrive(char *);
# endif
extern void disable_ctrlP(void);
extern void enable_ctrlP(void);
# ifdef WIN32
extern char *get_username(int *);
extern int set_binary_mode(int, int);
extern void nt_regularize(char *);
extern int (*t_kbhit(void));
extern void Delay(int);
# endif /* WIN32 */
#endif /* WIN32 */

/* ### mthrowu.c ### */

extern int thitu(int,int,struct obj *,const char *);
extern int ohitmon(struct monst *,struct monst *,struct obj *,int,boolean);
extern void thrwmu(struct monst *);
extern int spitmu(struct monst *,struct attack *);
extern int breamu(struct monst *,struct attack *);
extern boolean breamspot(struct monst *, struct attack *, xchar, xchar);
extern boolean linedup(xchar,xchar,xchar,xchar);
extern boolean lined_up(struct monst *);
extern struct obj *m_carrying(struct monst *,int);
extern void m_useup(struct monst *,struct obj *);
extern void m_throw(struct monst *,int,int,int,int,int,struct obj *);
extern boolean hits_bars(struct obj **,int,int,int,int);

/* ### muse.c ### */

extern boolean find_defensive(struct monst *);
extern int use_defensive(struct monst *);
extern int rnd_defensive_item(struct monst *);
extern boolean find_offensive(struct monst *);
extern int use_offensive(struct monst *);
extern int rnd_offensive_item(struct monst *);
extern boolean find_misc(struct monst *);
extern int use_misc(struct monst *);
extern int rnd_misc_item(struct monst *);
extern boolean searches_for_item(struct monst *,struct obj *);
extern boolean mon_reflects(struct monst *,const char *);
extern boolean ureflects(const char *,const char *);
extern boolean munstone(struct monst *,boolean);

/* ### music.c ### */

extern void awaken_soldiers(void);
extern int do_play_instrument(struct obj *);

/* ### nhlan.c ### */
#ifdef LAN_FEATURES
extern void init_lan_features(void);
extern char *lan_username(void);
# ifdef LAN_MAIL
extern bool lan_mail_check(void);
extern void lan_mail_read(struct obj *);
extern void lan_mail_init(void);
extern void lan_mail_finish(void);
extern void lan_mail_terminate(void);
# endif
#endif

/* ### nttty.c ### */

#ifdef WIN32CON
extern void get_scr_size(void);
extern int nttty_kbhit(void);
extern void nttty_check_stdio(void);
extern void nttty_open(void);
extern void nttty_rubout(void);
extern int tgetch(void);
extern int ntposkey(int *, int *, int *);
extern void set_output_mode(int);
extern void synch_cursor(void);
#endif

/* ### o_init.c ### */

extern void init_objects(void);
extern int find_skates(void);
extern void oinit(void);
extern void savenames(int,int);
extern void restnames(int);
extern void discover_object(int,boolean,boolean);
extern void undiscover_object(int);
extern int dodiscovered(void);

/* ### objects.c ### */

extern void objects_init(void);

/* ### objnam.c ### */

extern char *obj_typename(int);
extern char *simple_typename(int);
extern boolean obj_is_pname(struct obj *);
extern char *distant_name(struct obj *,char *(*)(struct obj*));
extern char *fruitname(boolean);
extern char *xname(struct obj *);
extern char *mshot_xname(struct obj *);
extern boolean the_unique_obj(struct obj *obj);
extern char *doname(struct obj *);
extern bool not_fully_identified(struct obj *);
extern char *corpse_xname(struct obj *,boolean);
extern char *cxname(struct obj *);
extern char *killer_xname(struct obj *);
extern char *killer_cxname(struct obj *,boolean);
extern const char *singular(struct obj *,char *(*)(struct obj*));
extern char *an(const char *);
extern char *An(const char *);
extern char *The(const char *);
extern char *the(const char *);
extern char *aobjnam(struct obj *,const char *);
extern char *Tobjnam(struct obj *,const char *);
extern char *otense(struct obj *,const char *);
extern char *vtense(const char *,const char *);
extern char *Doname2(struct obj *);
extern char *yname(struct obj *);
extern char *Yname2(struct obj *);
extern char *ysimple_name(struct obj *);
extern char *Ysimple_name2(struct obj *);
extern char *makeplural(const char *);
extern char *makesingular(const char *);
extern struct obj *readobjnam(char *,struct obj *,boolean);
extern int rnd_class(int,int);
extern const char *cloak_simple_name(struct obj *);
extern const char *mimic_obj_name(struct monst *);

/* ### options.c ### */

extern boolean match_optname(const char *,const char *,int,boolean);
extern void initoptions(void);
extern bool parse_monster_symbol(const char*);
extern bool parse_object_symbol(const char*);
extern bool parse_symbol(const char*);
extern void parseoptions(char *,boolean,boolean);
extern void parsetileset(char *);
extern int doset(void);
extern int dotogglepickup(void);
extern void option_help(void);
extern void next_opt(winid,const char *);
extern int fruitadd(char *);
extern int choose_classes_menu(const char *,int,boolean,char *,char *);
extern void add_menu_cmd_alias(char, char);
extern char map_menu_cmd(char);
extern void assign_warnings(uchar *);
extern char *nh_getenv(const char *);
extern void set_duplicate_opt_detection(int);
extern void set_wc_option_mod_status(unsigned long, int);
extern void set_wc2_option_mod_status(unsigned long, int);
extern void set_option_mod_status(const char *, int);
extern boolean add_menu_coloring(char *);
extern int add_autopickup_exception(const char *);
extern void free_autopickup_exceptions(void);

/* ### pager.c ### */

extern int dowhatis(void);
extern int doquickwhatis(void);
extern int doidtrap(void);
extern int dowhatdoes(void);
extern char *dowhatdoes_core(char, char *);
extern int dohelp(void);
extern int dohistory(void);

/* ### pcmain.c ### */

#ifdef WIN32
# ifdef CHDIR
extern void chdirx(char *, bool);
# endif /* CHDIR */
#endif /* WIN32 */

/* ### pcsys.c ### */

#if defined(WIN32)
extern void flushout(void);
extern int dosh(void);
extern void append_slash(char *);
extern void getreturn(const char *);
extern void msmsg(const char *,...);
extern FILE *fopenp(const char *,const char *);
#endif /* WIN32 */

/* ### pctty.c ### */

#ifdef WIN32
extern void gettty(void);
extern void settty(const char *);
extern void setftty(void);
extern void error(const char *,...);
#if defined(TIMED_DELAY) && defined(_MSC_VER)
extern void msleep(unsigned);
#endif
#endif /* WIN32 */

/* ### pcunix.c ### */

#if defined(PC_LOCKING)
extern void getlock(void);
#endif

/* ### pickup.c ### */

#ifdef GOLDOBJ
extern int collect_obj_classes(char *,struct obj *,boolean,bool (*)(struct obj*), int *);
#else
extern int collect_obj_classes(char *,struct obj *,boolean,boolean,bool (*)(struct obj*), int *);
#endif
extern void add_valid_menu_class(int);
extern bool allow_all(struct obj *);
extern bool allow_category(struct obj *);
extern bool is_worn_by_type(struct obj *);
extern boolean mbag_explodes(struct obj *, int);
extern void destroy_mbag(struct obj *, boolean);
extern int pickup(int);
extern int pickup_object(struct obj *, long, boolean);
extern int query_category(const char *, struct obj *, int,
				menu_item **, int);
extern int query_objlist(const char *, struct obj *, int, menu_item **, int, bool (*)(struct obj*));
extern struct obj *pick_obj(struct obj *);
extern int encumber_msg(void);
extern int doloot(void);
extern boolean container_gone(int (*)(struct obj*));
extern int use_container(struct obj **,int);
extern int loot_mon(struct monst *,int *,boolean *);
extern const char *safe_qbuf(const char *,unsigned,
				const char *,const char *,const char *);
extern boolean is_autopickup_exception(struct obj *, boolean);

/* ### pline.c ### */

// for safety; if you say pline(x), where x is untrusted user data, then the
// controller of that data has arbitrary code execution.  Maybe.
#define plines(str) pline("%s", (str))
#define Noreps(str) Norep("%s", (str))
#define verbalizes(str) verbalize("%s", (str))
#define You_hear(str) You_hearf("%s", (str))
extern void msgpline_add(int, char *);
extern void msgpline_free(void);
extern void pline(const char *,...) PRINTF_F(1,2);
extern void Norep(const char *,...) PRINTF_F(1,2);
extern void free_youbuf(void);
extern void You_hearf(const char *,...) PRINTF_F(1,2);
extern void verbalize(const char *,...) PRINTF_F(1,2);
extern void raw_printf(const char *,...) PRINTF_F(1,2);
extern void impossible(const char *,...) PRINTF_F(1,2);
extern const char *align_str(aligntyp);
extern void mstatusline(struct monst *);
extern void ustatusline(void);
extern void self_invis_message(void);

/* ### polyself.c ### */

extern void init_uasmon(void);
extern void set_uasmon(void);
extern void change_sex(void);
extern void polyself(boolean);
extern int polymon(int);
extern void rehumanize(void);
extern int dobreathe(void);
extern int dospit(void);
extern int doremove(void);
extern int dospinweb(void);
extern int dosummon(void);
extern int dogaze(void);
extern int dohide(void);
extern int domindblast(void);
extern void skinback(boolean);
extern const char *mbodypart(struct monst *,int);
extern const char *body_part(int);
extern int poly_gender(void);
extern void ugolemeffects(int,int);
extern int polyatwill(void);

/* ### potion.c ### */

extern void set_itimeout(long *,long);
extern void incr_itimeout(long *,int);
extern void make_confused(long,boolean);
extern void make_stunned(long,boolean);
extern void make_blinded(long,boolean);
extern void make_sick(long, const char *, boolean,int);
extern void make_vomiting(long,boolean);
extern boolean make_hallucinated(long,boolean,long);
extern int dodrink(void);
extern int dopotion(struct obj *);
extern int peffects(struct obj *);
extern void healup(int,int,boolean,boolean);
extern void strange_feeling(struct obj *,const char *);
extern void potionhit(struct monst *,struct obj *,boolean);
extern void potionbreathe(struct obj *);
extern boolean get_wet(struct obj *, boolean);
extern int dodip(void);
extern void djinni_from_bottle(struct obj *);
/* KMH, balance patch -- new function */
extern int upgrade_obj(struct obj *);
extern struct monst *split_mon(struct monst *,struct monst *);
extern const char *bottlename(void);

/* ### pray.c ### */

extern int dosacrifice(void);
extern boolean can_pray(boolean);
extern int dopray(void);
extern const char *u_gname(void);
extern int doturn(void);
extern int turn_undead(void);
extern const char *a_gname(void);
extern const char *a_gname_at(xchar x,xchar y);
extern const char *align_gname(aligntyp);
extern const char *halu_gname(aligntyp);
extern const char *align_gtitle(aligntyp);
extern void altar_wrath(int,int);


/* ### priest.c ### */

extern int move_special(struct monst *,boolean,schar,boolean,boolean,
			   xchar,xchar,xchar,xchar);
extern char temple_occupied(char *);
extern int pri_move(struct monst *);
extern void priestini(d_level *,struct mkroom *,int,int,boolean);
extern char *priestname(struct monst *,char *);
extern boolean p_coaligned(struct monst *);
extern struct monst *findpriest(char);
extern void intemple(int);
extern void priest_talk(struct monst *);
extern struct monst *mk_roamer(struct permonst *,aligntyp,
				  xchar,xchar,boolean);
extern void reset_hostility(struct monst *);
extern boolean in_your_sanctuary(struct monst *,xchar,xchar);
extern void ghod_hitsu(struct monst *);
extern void angry_priest(void);
extern void clearpriests(void);
extern void restpriest(struct monst *,boolean);

/* ### quest.c ### */

extern void onquest(void);
extern void nemdead(void);
extern void artitouch(void);
extern boolean ok_to_quest(void);
extern void leader_speaks(struct monst *);
extern void nemesis_speaks(void);
extern void quest_chat(struct monst *);
extern void quest_talk(struct monst *);
extern void quest_stat_check(struct monst *);
extern void finish_quest(struct obj *);

/* ### questpgr.c ### */

extern void load_qtlist(void);
extern void unload_qtlist(void);
extern short quest_info(int);
extern const char *ldrname(void);
extern bool is_quest_artifact(struct obj*);
extern void com_pager(int);
extern void qt_pager(int);
extern struct permonst *qt_montype(void);

/* ### random.c ### */

#if defined(RANDOM) && !defined(__GO32__) /* djgpp has its own random */
extern void srandom(unsigned);
extern char *initstate(unsigned,char *,int);
extern char *setstate(char *);
extern long random(void);
#endif /* RANDOM */

/* ### read.c ### */

extern int doread(void);
extern boolean is_chargeable(struct obj *);
extern void recharge(struct obj *,int);
extern void forget(int);
extern void forget_objects(int);
extern void forget_levels(int);
extern void forget_traps(void);
extern void forget_map(int);
extern int seffects(struct obj *);
extern void litroom(boolean,struct obj *);
extern void do_genocide(int);
extern void punish(struct obj *);
extern void unpunish(void);
extern boolean cant_create(int *, boolean);
extern struct monst *create_particular(void);

/* ### rect.c ### */

extern void init_rect(void);
extern NhRect *get_rect(NhRect *);
extern NhRect *rnd_rect(void);
extern void remove_rect(NhRect *);
extern void add_rect(NhRect *);
extern void split_rects(NhRect *,NhRect *);

/* ## region.c ### */
extern void clear_regions(void);
extern void run_regions(void);
extern boolean in_out_region(xchar,xchar);
extern boolean m_in_out_region(struct monst *,xchar,xchar);
extern void update_player_regions(void);
extern void update_monster_region(struct monst *);
extern NhRegion *visible_region_at(xchar,xchar);
extern void show_region(NhRegion*, xchar, xchar);
extern void save_regions(int,int);
extern void rest_regions(int,boolean);
extern NhRegion* create_gas_cloud(xchar, xchar, int, int);
extern NhRegion* create_cthulhu_death_cloud(xchar, xchar, int, int);

/* ### restore.c ### */

extern void inven_inuse(boolean);
extern int dorecover(int);
extern void trickery(char *);
extern void getlev(int,int,xchar,boolean);
extern void minit(void);
extern boolean lookup_id_mapping(unsigned, unsigned *);
extern void mread(int,void *,uint);

/* ### rip.c ### */

extern void genl_outrip(winid,int);

/* ### rnd.c ### */

extern uint rn2(uint x);
extern uint rnl(uint x);
extern uint rnd(uint x);
extern uint d(uint n, uint x);
extern uint rne(uint x);
extern uint rnz(uint i);

/* ### role.c ### */

extern boolean validrole(int);
extern boolean validrace(int, int);
extern boolean validgend(int, int, int);
extern boolean validalign(int, int, int);
extern int randrole(void);
extern int randrace(int);
extern int randgend(int, int);
extern int randalign(int, int);
extern int str2role(char *);
extern int str2race(char *);
extern int str2gend(char *);
extern int str2align(char *);
extern int mrace2race(int);
extern boolean ok_role(int, int, int, int);
extern int pick_role(int, int, int, int);
extern boolean ok_race(int, int, int, int);
extern int pick_race(int, int, int, int);
extern boolean ok_gend(int, int, int, int);
extern int pick_gend(int, int, int, int);
extern boolean ok_align(int, int, int, int);
extern int pick_align(int, int, int, int);
extern void role_init(void);
extern void rigid_role_checks(void);
extern void plnamesuffix(void);
extern const char *Hello(struct monst *);
extern const char *Goodbye(void);
extern char *build_plselection_prompt(char *, int, int, int, int, int);
extern char *root_plselection_prompt(char *, int, int, int, int, int);

/* ### rumors.c ### */

extern char *getrumor(int, char *, bool);
extern void outrumor(int,int);
extern void outoracle(boolean, boolean);
extern int doconsult(struct monst *);

/* ### save.c ### */

extern int dosave(void);
#if defined(UNIX) || defined(__EMX__) || defined(WIN32)
extern void hangup(int);
#endif
extern int dosave0(void);
#ifdef INSURANCE
extern void savestateinlock(void);
#endif
extern void savelev(int,xchar,int);
extern void bufon(int);
extern void bufoff(int);
extern void bflush(int);
extern void bwrite(int,void *,uint);
extern void bclose(int);
extern void savefruitchn(int,int);
extern void free_dungeons(void);
extern void freedynamicdata(void);

/* ### shk.c ### */

#ifdef GOLDOBJ
extern long money2mon(struct monst *, long);
extern void money2u(struct monst *, long);
#endif
extern char *shkname(struct monst *);
extern void shkgone(struct monst *);
extern void set_residency(struct monst *,boolean);
extern void replshk(struct monst *,struct monst *);
extern void save_shk_bill(int, struct monst *);
extern void restore_shk_bill(int, struct monst *);
extern void restshk(struct monst *,boolean);
extern char inside_shop(xchar,xchar);
extern void u_left_shop(char *,boolean);
extern void remote_burglary(xchar,xchar);
extern void u_entered_shop(char *);
extern boolean same_price(struct obj *,struct obj *);
extern void shopper_financial_report(void);
extern int inhishop(struct monst *);
extern struct monst *shop_keeper(char);
extern boolean tended_shop(struct mkroom *);
extern void delete_contents(struct obj *);
extern void obfree(struct obj *,struct obj *);
extern void shk_free(struct monst *);
extern void home_shk(struct monst *,boolean);
extern void make_happy_shk(struct monst *,boolean);
extern void hot_pursuit(struct monst *);
extern void make_angry_shk(struct monst *,xchar,xchar);
extern int dopay(void);
extern boolean paybill(int);
extern void finish_paybill(void);
extern struct obj *find_oid(unsigned);
extern long contained_cost(struct obj *,struct monst *,long,boolean, boolean);
extern long contained_gold(struct obj *);
extern void picked_container(struct obj *);
extern long unpaid_cost(struct obj *);
extern void addtobill(struct obj *,boolean,boolean,boolean);
extern void splitbill(struct obj *,struct obj *);
extern void subfrombill(struct obj *,struct monst *);
extern long stolen_value(struct obj *,xchar,xchar,boolean,boolean,
			    boolean);
extern void sellobj_state(int);
extern void sellobj(struct obj *,xchar,xchar);
extern int doinvbill(int);
extern struct monst *shkcatch(struct obj *,xchar,xchar);
extern void add_damage(xchar,xchar,long);
extern int repair_damage(struct monst *,struct damage *,boolean);
extern int shk_move(struct monst *);
extern void after_shk_move(struct monst *);
extern boolean is_fshk(struct monst *);
extern void shopdig(int);
extern void pay_for_damage(const char *,boolean);
extern boolean costly_spot(xchar,xchar);
extern struct obj *shop_object(xchar,xchar);
extern void price_quote(struct obj *);
extern void shk_chat(struct monst *);
extern void check_unpaid_usage(struct obj *,boolean);
extern void check_unpaid(struct obj *);
extern void costly_gold(xchar,xchar,long);
extern boolean block_door(xchar,xchar);
extern boolean block_entry(xchar,xchar);
extern boolean block_entry(xchar,xchar);
extern void blkmar_guards(struct monst *);
extern char *shk_your(char *,struct obj *);
extern char *Shk_Your(char *,struct obj *);

/* ### shknam.c ### */

extern void stock_room(int,struct mkroom *);
extern boolean saleable(struct monst *,struct obj *);
extern int get_shop_item(int);

/* ### sit.c ### */

extern void take_gold(void);
extern int dosit(void);
extern void rndcurse(void);
extern void attrcurse(void);

/* ### sounds.c ### */

extern void dosounds(void);
extern void pet_distress(struct monst *, int);
extern const char *growl_sound(struct monst *);
/* JRN: converted growl,yelp,whimper to macros based on pet_distress.
  Putting them here since I don't know where else (TOFIX) */
#define growl(mon) pet_distress((mon),3)
#define yelp(mon) pet_distress((mon),2)
#define whimper(mon) pet_distress((mon),1)
extern void beg(struct monst *);
extern int dotalk(void);
#ifdef USER_SOUNDS
extern int add_sound_mapping(const char *);
extern void play_sound_for_message(const char *);
#endif

/* ### sp_lev.c ### */

extern boolean check_room(xchar *,xchar *,xchar *,xchar *,boolean);
extern boolean create_room(xchar,xchar,xchar,xchar,
			      xchar,xchar,xchar,xchar);
extern void create_secret_door(struct mkroom *,xchar);
extern boolean dig_corridor(coord *,coord *,boolean,schar,schar);
extern void fill_room(struct mkroom *,boolean);
extern boolean load_special(const char *);

/* ### spell.c ### */

extern int study_book(struct obj *);
extern void book_disappears(struct obj *);
extern void book_substitution(struct obj *,struct obj *);
extern void age_spells(void);
extern int docast(void);
extern int spell_skilltype(int);
extern int spelleffects(int,boolean);
extern void losespells(void);
extern int dovspell(void);
extern void learnspell(struct obj *);
extern boolean studyspell(void);
extern void initialspell(struct obj *);

/* ### steal.c ### */

#ifdef GOLDOBJ
extern long somegold(long);
#else
extern long somegold(void);
#endif
extern void stealgold(struct monst *);
extern void remove_worn_item(struct obj *,boolean);
extern int steal(struct monst *, char *);
extern int mpickobj(struct monst *,struct obj *);
extern void stealamulet(struct monst *);
extern void mdrop_special_objs(struct monst *);
extern void relobj(struct monst *,int,boolean);
#ifdef GOLDOBJ
extern struct obj *findgold(struct obj *);
#endif

/* ### steed.c ### */

extern void rider_cant_reach(void);
extern boolean can_saddle(struct monst *);
extern int use_saddle(struct obj *);
extern boolean can_ride(struct monst *);
extern int doride(void);
extern boolean mount_steed(struct monst *, boolean);
extern void exercise_steed(void);
extern void kick_steed(void);
extern void dismount_steed(int);
extern void place_monster(struct monst *,int,int);

/* ### tech.c ### */

extern void adjtech(int,int);
extern int dotech(void);
extern void docalm(void);
extern int tech_inuse(int);
extern void tech_timeout(void);
extern boolean tech_known(short);
extern void learntech(short,long,int);

/* ### teleport.c ### */

extern boolean goodpos(int,int,struct monst *,unsigned);
extern boolean enexto(coord *,xchar,xchar,struct permonst *);
extern boolean enexto_core(coord *,xchar,xchar,struct permonst *,unsigned);
extern int epathto(coord *,int,xchar,xchar,struct permonst *);
extern boolean wpathto(coord *, coord *, boolean (*)(void *, int, int),
			  void *, struct permonst *, int);
extern void xpathto(int,xchar,xchar,int (*)(void *,int,int),void *);
extern void teleds(int,int,boolean);
extern boolean safe_teleds(boolean);
extern boolean teleport_pet(struct monst *,boolean);
extern void tele(void);
extern int dotele(void);
extern void level_tele(void);
extern void domagicportal(struct trap *);
extern void tele_trap(struct trap *);
extern void level_tele_trap(struct trap *);
extern void rloc_to(struct monst *,int,int);
extern boolean rloc(struct monst *, boolean);
extern boolean tele_restrict(struct monst *);
extern void mtele_trap(struct monst *, struct trap *,int);
extern int mlevel_tele_trap(struct monst *, struct trap *,boolean,int);
extern void rloco(struct obj *);
extern int random_teleport_level(void);
extern boolean u_teleport_mon(struct monst *,boolean);

/* ### tile.c ### */
#ifdef USE_TILES
extern void substitute_tiles(d_level *);
#endif

/* ### timeout.c ### */

extern void burn_away_slime(void);
extern void nh_timeout(void);
extern void fall_asleep(int, boolean);
extern void set_obj_poly(struct obj *, struct obj *);
extern void unpoly_obj(void *, long);
extern int mon_poly(struct monst *, boolean, const char *);
extern int mon_spec_poly(struct monst *, struct permonst *, long,
			    boolean, boolean, boolean, boolean);
extern void unpoly_mon(void *, long);
extern void attach_bomb_blow_timeout(struct obj *, int, boolean);
extern void attach_egg_hatch_timeout(struct obj *);
extern void attach_fig_transform_timeout(struct obj *);
extern void kill_egg(struct obj *);
extern void hatch_egg(void *, long);
extern void learn_egg_type(int);
extern void burn_object(void *, long);
extern void begin_burn(struct obj *, boolean);
extern void end_burn(struct obj *, boolean);
extern void burn_faster(struct obj *, long);
extern void lightsaber_deactivate(struct obj *, boolean);
extern void do_storms(void);
extern boolean start_timer(long, short, short, anything);
extern long stop_timer(short, anything);
extern void run_timers(void);
extern void obj_move_timers(struct obj *, struct obj *);
extern void obj_split_timers(struct obj *, struct obj *);
extern void obj_stop_timers(struct obj *);
extern void mon_stop_timers(struct monst *);
extern boolean obj_is_local(struct obj *);
extern void save_timers(int,int,int);
extern void restore_timers(int,int,boolean,long);
extern void relink_timers(boolean);
extern int wiz_timeout_queue(void);
extern void timer_sanity_check(void);

/* ### topten.c ### */

extern void topten(int);
extern void prscore(int,char **);
extern struct obj *tt_oname(struct obj *);
#ifdef PROXY_GRAPHICS
extern winid create_toptenwin(void);
extern void destroy_toptenwin(void);
#endif

/* ### track.c ### */

extern void initrack(void);
extern void settrack(void);
extern coord *gettrack(int,int);

/* ### trap.c ### */

extern boolean burnarmor(struct monst *);
extern boolean rust_dmg(struct obj *,const char *,int,boolean,struct monst *);
extern void grease_protect(struct obj *,const char *,struct monst *);
extern struct trap *maketrap(int,int,int);
extern void fall_through(boolean);
extern struct monst *animate_statue(struct obj *,xchar,xchar,int,int *);
extern struct monst *activate_statue_trap(struct trap *,xchar,xchar,boolean);
extern void dotrap(struct trap *, unsigned);
extern void seetrap(struct trap *);
extern int mintrap(struct monst *);
extern void instapetrify(const char *);
extern void minstapetrify(struct monst *,boolean);
extern void selftouch(const char *);
extern void mselftouch(struct monst *,const char *,boolean);
extern void float_up(void);
extern void fill_pit(int,int);
extern int float_down(long, long);
extern int fire_damage(struct obj *,boolean,boolean,xchar,xchar);
extern void water_damage(struct obj *,boolean,boolean);
extern boolean drown(void);
extern void mon_drain_en(struct monst *, int);
extern void drain_en(int);
extern int dountrap(void);
extern int untrap(boolean);
extern boolean chest_trap(struct obj *,int,boolean);
extern void deltrap(struct trap *);
extern boolean delfloortrap(struct trap *);
extern struct trap *t_at(int,int);
extern void b_trapped(const char *,int);
extern boolean unconscious(void);
extern boolean lava_effects(void);
extern void blow_up_landmine(struct trap *);
extern int launch_obj(short,int,int,int,int,int);
/* KMH, balance patch -- new function */
extern int uunstone(void);

/* ### u_init.c ### */

extern void u_init(void);

/* ### uhitm.c ### */

extern void hurtmarmor(struct monst *,int);
extern int attack_checks(struct monst *,boolean);
extern void check_caitiff(struct monst *);
extern schar find_roll_to_hit(struct monst *);
extern boolean attack(struct monst *);
extern boolean hmon(struct monst *,struct obj *,int);
extern int damageum(struct monst *,struct attack *);
extern void missum(struct monst *,int, int, struct attack *);
extern int passive(struct monst *,int,int,uchar);
extern void passive_obj(struct monst *,struct obj *,struct attack *);
extern void stumble_onto_mimic(struct monst *);
extern int flash_hits_mon(struct monst *,struct obj *);

/* ### unixmain.c ### */

#ifdef UNIX
# ifdef PORT_HELP
extern void port_help(void);
# endif
#endif /* UNIX */

/* ### unixtty.c ### */

#ifdef UNIX
extern void gettty(void);
extern void settty(const char *);
extern void setftty(void);
extern void intron(void);
extern void introff(void);
extern void error(const char *,...) PRINTF_F(1,2);
#endif /* UNIX */

/* ### unixunix.c ### */

#ifdef UNIX
extern void getlock(void);
extern void regularize(char *);
# if defined(TIMED_DELAY) && !defined(msleep) && defined(SYSV)
extern void msleep(unsigned);
# endif
# ifdef SHELL
extern int dosh(void);
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
extern int child(int);
# endif
#ifdef FILE_AREAS
extern char *make_file_name(const char *, const char *);
extern FILextern *fopen_datafile_area(const char *,const char *,const char *,
				boolean);
extern FILextern *freopen_area(const char *,const char *,const char *, FILextern *);
extern int chmod_area(const char *, const char *, int);
extern int open_area(const char *, const char *, int, int);
extern int creat_area(const char *, const char *, int);
extern boolean lock_file_area(const char *, const char *,int);
extern void unlock_file_area(const char *, const char *);
#endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
extern int hide_privileges(boolean);
#endif /* UNIX */

/* ### vault.c ### */

extern boolean grddead(struct monst *);
extern char vault_occupied(char *);
extern void invault(void);
extern int gd_move(struct monst *);
extern void paygd(void);
extern long hidden_gold(void);
extern boolean gd_sound(void);

/* ### version.c ### */

extern char *version_string_tmp(void);
extern char *full_version_string_tmp();
extern int doversion(void);
extern boolean check_version(struct version_info *, const char *,boolean);
extern unsigned long get_feature_notice_ver(char *);
extern unsigned long get_current_feature_ver(void);
#ifdef RUNTIME_PORT_ID
extern void append_port_id(char *);
#endif

/* ### video.c ### */

#ifdef VIDEOSHADES
extern int assign_videoshades(char *);
extern int assign_videocolors(char *);
#endif

/* ### vision.c ### */

extern void vision_init(void);
extern int does_block(int,int,struct rm*);
extern void vision_reset(void);
extern void vision_recalc(int);
extern void block_point(int,int);
extern void unblock_point(int,int);
extern bool clear_path(int,int,int,int);
extern void do_clear_area(int,int,int,
			     void (*)(int,int,void *),void *);

/* ### weapon.c ### */

extern int hitval(struct obj *,struct monst *);
extern int dmgval(struct obj *,struct monst *);
extern struct obj *select_rwep(struct monst *);
extern struct obj *select_hwep(struct monst *);
extern void possibly_unwield(struct monst *,boolean);
extern int mon_wield_item(struct monst *);
extern int abon(void);
extern int dbon(void);
extern int enhance_weapon_skill(void);
#ifdef DUMP_LOG
extern void dump_weapon_skill(void);
#endif
extern void unrestrict_weapon_skill(int);
extern void use_skill(int,int);
extern void add_weapon_skill(int);
extern void lose_weapon_skill(int);
extern int weapon_type(struct obj *);
extern int uwep_skill_type(void);
extern int weapon_hit_bonus(struct obj *);
extern int weapon_dam_bonus(struct obj *);
extern int skill_bonus(int);
extern void skill_init(const struct def_skill *);
extern void practice_weapon(void);

/* ### were.c ### */

extern int counter_were(int);
extern void were_change(struct monst *);
extern void new_were(struct monst *);
extern int were_summon(struct permonst *,boolean,int *,char *);
extern void you_were(void);
extern void you_unwere(boolean);

/* ### wield.c ### */

extern void setuwep(struct obj *,boolean);
extern void setuqwep(struct obj *);
extern void setuswapwep(struct obj *,boolean);
extern int dowield(void);
extern int doswapweapon(void);
extern int dowieldquiver(void);
extern boolean wield_tool(struct obj *,const char *);
extern int can_twoweapon(void);
extern void drop_uswapwep(void);
extern int dotwoweapon(void);
extern void uwepgone(void);
extern void uswapwepgone(void);
extern void uqwepgone(void);
extern void untwoweapon(void);
extern void erode_obj(struct obj *,boolean,boolean);
extern int chwepon(struct obj *,int);
extern int welded(struct obj *);
extern void weldmsg(struct obj *);
extern void setmnotwielded(struct monst *,struct obj *);
extern void unwield(struct obj *,boolean);

/* ### windows.c ### */

extern void choose_windows(const char *);
extern char genl_message_menu(char,int,const char *);
extern void genl_preference_update(const char *);

/* ### wizard.c ### */

extern void amulet(void);
extern int mon_has_amulet(struct monst *);
extern int mon_has_special(struct monst *);
extern int tactics(struct monst *);
extern void aggravate(void);
extern void clonewiz(void);
extern int pick_nasty(void);
extern int nasty(struct monst*);
extern void resurrect(void);
extern void intervene(void);
extern void wizdead(void);
extern void cuss(struct monst *);

/* ### worm.c ### */

extern int get_wormno(void);
extern void initworm(struct monst *,int);
extern void worm_move(struct monst *);
extern void worm_nomove(struct monst *);
extern void wormgone(struct monst *);
extern void wormhitu(struct monst *);
extern int cutworm(struct monst *,xchar,xchar,struct obj *);
extern void see_wsegs(struct monst *);
extern void detect_wsegs(struct monst *,boolean);
extern void save_worm(int,int);
extern void rest_worm(int);
extern void place_wsegs(struct monst *);
extern void remove_worm(struct monst *);
extern void place_worm_tail_randomly(struct monst *,xchar,xchar);
extern int count_wsegs(struct monst *);
extern boolean worm_known(struct monst *);

/* ### worn.c ### */

extern void setworn(struct obj *,long);
extern void setnotworn(struct obj *);
extern void mon_set_minvis(struct monst *);
extern void mon_adjust_speed(struct monst *,int,struct obj *);
extern void update_mon_intrinsics(struct monst *,struct obj *,boolean,boolean);
extern int find_mac(struct monst *);
extern void m_dowear(struct monst *,boolean);
extern struct obj *which_armor(struct monst *,long);
extern void mon_break_armor(struct monst *,boolean);
extern void bypass_obj(struct obj *);
extern void clear_bypasses(void);
extern int racial_exception(struct monst *, struct obj *);

/* ### write.c ### */

extern int dowrite(struct obj *);

/* ### zap.c ### */

extern int bhitm(struct monst *,struct obj *);
extern void probe_monster(struct monst *);
extern boolean get_obj_location(struct obj *,xchar *,xchar *,int);
extern boolean get_mon_location(struct monst *,xchar *,xchar *,int);
extern struct monst *get_container_location(struct obj *obj, int *, int *);
extern struct monst *montraits(struct obj *,coord *);
extern struct monst *revive(struct obj *);
extern int unturn_dead(struct monst *);
extern void cancel_item(struct obj *);
extern boolean drain_item(struct obj *);	/* KMH */
extern boolean obj_resists(struct obj *,int,int);
extern boolean obj_shudders(struct obj *);
extern void do_osshock(struct obj *);
extern void puton_worn_item(struct obj *);
extern void obj_set_oinvis(struct obj *, boolean, boolean);
extern struct obj *poly_obj(struct obj *, int);
extern int bhito(struct obj *,struct obj *);
extern int bhitpile(struct obj *,int (*)(struct obj*,struct obj*),int,int);
extern int zappable(struct obj *);
extern void zapnodir(struct obj *);
extern int dozap(void);
extern int zapyourself(struct obj *,boolean);
extern boolean cancel_monst(struct monst *,struct obj *,
			       boolean,boolean,boolean);
extern void weffects(struct obj *);
extern int spell_damage_bonus(int);
extern const char *exclam(int force);
extern void hit(const char *,struct monst *,const char *);
extern void miss(const char *,struct monst *);
extern struct monst *bhit(int,int,int,int,int (*)(struct monst*,struct obj*),
			     int (*)(struct obj*,struct obj*),struct obj **);
extern struct monst *boomhit(int,int);
extern int burn_floor_paper(int,int,boolean,boolean);
extern void buzz(int,int,xchar,xchar,int,int);
extern void melt_ice(xchar,xchar);
extern int zap_over_floor(xchar,xchar,int,boolean *);
extern void fracture_rock(struct obj *);
extern boolean break_statue(struct obj *);
extern void destroy_item(int,int);
extern int destroy_mitem(struct monst *,int,int);
extern int resist(struct monst *,char,int,int);
extern void makewish(void);
/* KMH -- xchar to xchar */
extern void zap_strike_fx(xchar, xchar, int);
extern void throwspell(void);

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

#endif /* EXTERN_H */
