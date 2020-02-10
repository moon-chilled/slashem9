/*	SCCS Id: @(#)extern.h	3.4	2003/03/10	*/
/* Copyright (c) Steve Creps, 1988.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

#include "nhstr.h"

/* ### alloc.c ### */

void *alloc(usize);
void nhfree(const void *);
#define free nhfree
char *fmt_ptr(const void *, char *);

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#ifndef LEV_LEX_C

/* ### allmain.c ### */

void moveloop(void);
void stop_occupation(void);
void create_gamewindows(void);
void show_gamewindows(void);
void newgame(void);
void welcome(boolean);
time_t get_realtime(void);

/* ### apply.c ### */

int doapply(void);
int dorub(void);
int dojump(void);
int jump(int);
int jump(int);
int number_leashed(void);
void o_unleash(struct obj *);
void m_unleash(struct monst *, boolean);
void unleash_all(void);
boolean next_to_u(void);
struct obj *get_mleash(struct monst *);
void check_leash(struct monst *, xchar, xchar, boolean);
boolean um_dist(xchar, xchar, xchar);
boolean snuff_candle(struct obj *);
boolean snuff_lit(struct obj *);
boolean catch_lit(struct obj *);
void use_unicorn_horn(struct obj *);
boolean tinnable(struct obj *);
void reset_trapset(void);
void fig_transform(void *, long);
int unfixable_trouble_count(boolean);
int wand_explode(struct obj *, boolean);

/* ### artifact.c ### */

void init_artifacts(void);
void init_artifacts1(void);
void save_artifacts(int fd);
void restore_artifacts(int fd);
const char *artiname(int artinum);
struct obj *mk_artifact(struct obj *otmp, aligntyp alignment);
const char *artifact_name(const char *name, short *otyp);
bool exist_artifact(int otyp, const char *name);
void artifact_exists(struct obj *otmp, const char *name, bool mod);
int nartifact_exist(void);
bool spec_ability(struct obj *otmp, unsigned long abil);
bool confers_luck(struct obj *obj);
bool arti_reflects(struct obj *obj);
bool restrict_name(struct obj *otmp, const char *name);
bool defends(int adtype, struct obj *otmp);
bool protects(int adtype, struct obj *otmp);
void set_artifact_intrinsic(struct obj *otmp, bool on, long wp_mask);
int touch_artifact(struct obj *obj, struct monst *mon);
long spec_m2(struct obj *otmp);
int spec_abon(struct obj *otmp, struct monst *mon);
int spec_dbon(struct obj *otmp, struct monst *mon, int tmp);
void discover_artifact(int m);
bool undiscovered_artifact(xchar m);
int disp_artifact_discoveries(winid tmpwin);
bool artifact_hit(struct monst *magr, struct monst *mdef, struct obj *otmp, int *dmgptr, int dieroll);
int doinvoke(void);
int artifact_wet(struct obj *obj, bool silent);
bool artifact_light(struct obj *obj);
void arti_speak(struct obj *obj);
bool artifact_has_invprop(struct obj *otmp, uchar inv_prop);
long arti_cost(struct obj *otmp);

/* ### attrib.c ### */

boolean adjattrib(int, int, int);
void change_luck(schar);
int stone_luck(boolean);
void set_moreluck(void);
void gainstr(struct obj *, int);
void losestr(int);
void restore_attrib(void);
void exercise(int, boolean);
void exerchk(void);
void init_attr(int);
void redist_attr(void);
void adjabil(int, int);
int newhp(void);
schar acurr(int);
schar acurrstr(void);
void adjalign(int);
/* KMH, balance patch -- new function */
void recalc_health(void);
int uhp(void);
int uhpmax(void);

/* ### ball.c ### */

void ballfall(void);
void placebc(void);
void unplacebc(void);
void set_bc(boolean);
void move_bc(int, int, xchar, xchar, xchar, xchar);
boolean drag_ball(xchar, xchar,
		  int *, xchar *, xchar *, xchar *, xchar *, boolean *, boolean);
void drop_ball(xchar, xchar);
void drag_down(void);

/* ### bones.c ### */

boolean can_make_bones(void);
void savebones(struct obj *);
int getbones(void);

/* ### borg.c ### */

bool borg_on;
char borg_line[80];
char borg_input(void);

/* ### botl.c ### */

int xlev_to_rank(int);
int title_to_mon(const char *, int *, int *);
void max_rank_sz(void);
long botl_score(void);
int describe_level(char *, int);
const char *rank_of(int, short, boolean);
void bot_set_handler(void (*)());
void bot_reconfig(void);
void bot(void);
nhstr bot1str(void);
nhstr bot2str(void);

/* ### cmd.c ### */

char randomkey(void);
void reset_occupations(void);
void set_occupation(int (*)(void), const char *, int);
char pgetchar(void);
void pushch(char);
void savech(char);
char nhgetch(void);
void rhack(char *);
void dokeylist(void);
int doextlist(void);
int extcmd_via_menu(void);
void enlightenment(int);
void show_conduct(int);
int xytod(schar, schar);
void dtoxy(coord *, int);
int movecmd(char);
int getdir(const char *);
void confdir(void);
int isok(int, int);
int get_adjacent_loc(const char *, const char *, xchar, xchar, coord *);
const char *click_to_cmd(int, int, int);
char readchar(void);
void sanity_check(void);
void commands_init(void);
char *stripspace(char *);
void parsebindings(char *);
void parseautocomplete(char *, boolean);
void parsemappings(char *);
char txt2key(char *);
char *key2txt(char, char *);
char *str2txt(char *, char *);
char yn_function(const char *, const char *, char);

/* ### dbridge.c ### */

bool is_pool(int x, int y);
bool is_lava(int x, int y);
bool is_ice(int x, int y);
int is_drawbridge_wall(int x, int y);
bool is_db_wall(int x, int y);
bool find_drawbridge(int *x, int *y);
bool create_drawbridge(int x, int y, int dir, bool flag);
void open_drawbridge(int x, int y);
void close_drawbridge(int x, int y);
void destroy_drawbridge(int x, int y);

/* ### decl.c ### */

void decl_init(void);
bool curses_stupid_hack;

/* ### detect.c ### */

struct obj *o_in(struct obj *, char);
struct obj *o_material(struct obj *, unsigned);
int gold_detect(struct obj *);
int food_detect(struct obj *);
int object_detect(struct obj *, int);
int monster_detect(struct obj *, int);
int trap_detect(struct obj *);
const char *level_distance(d_level *);
void use_crystal_ball(struct obj *);
void do_mapping(void);
void do_vicinity_map(void);
void cvt_sdoor_to_door(struct rm *);
int findit(void);
int openit(void);
void find_trap(struct trap *);
void dosearch0(bool);
int dosearch(void);
void sokoban_detect(void);
/* KMH -- Sokoban levels */
void sokoban_detect(void);

/* ### dig.c ### */

boolean is_digging(void);
int holetime(void);
boolean dig_check(struct monst *, boolean, int, int);
void digactualhole(int, int, struct monst *, int);
boolean dighole(boolean);
int use_pick_axe(struct obj *);
int use_pick_axe2(struct obj *);
boolean mdig_tunnel(struct monst *);
void watch_dig(struct monst *, xchar, xchar, boolean);
void zap_dig(void);
struct obj *bury_an_obj(struct obj *);
void bury_objs(int, int);
void unearth_objs(int, int);
void rot_organic(void *, long);
void rot_corpse(void *, long);
schar fillholetyp(int x, int y);
void liquid_flow(xchar x, xchar y, schar typ, struct trap *ttmp, const char *fillmsg);

/* ### display.c ### */

struct obj *vobj_at(xchar, xchar);
void magic_map_background(xchar, xchar, int);
void map_background(xchar, xchar, int);
void map_trap(struct trap *, int);
void map_object(struct obj *, int);
void map_invisible(xchar, xchar);
void unmap_object(int, int);
void map_location(int, int, int);
int memory_glyph(int, int);
void clear_memory_glyph(int, int, int);
void feel_location(xchar, xchar);
void newsym(int, int);
void shieldeff(xchar, xchar);
void tmp_at(int, int);
int glyph_is_floating(int);
void swallowed(int);
void under_ground(int);
void under_water(int);
void see_monsters(void);
void set_mimic_blocking(void);
void see_objects(void);
void see_traps(void);
void curs_on_u(void);
int doredraw(void);
void docrt(void);
void show_glyph(int, int, int);
void clear_glyph_buffer(void);
void row_refresh(int, int, int);
void cls(void);
void flush_screen(int);
int back_to_glyph(xchar, xchar);
int zapdir_to_glyph(int, int, int);
int glyph_at(xchar, xchar);
void set_wall_state(void);
void dump_map(void);

/* ### do.c ### */

int dodrop(void);
boolean boulder_hits_pool(struct obj *, int, int, boolean);
boolean flooreffects(struct obj *, int, int, const char *);
void doaltarobj(struct obj *);
boolean canletgo(struct obj *, const char *);
void dropx(struct obj *);
void dropy(struct obj *);
void obj_no_longer_held(struct obj *);
int doddrop(void);
int dodown(void);
int doup(void);
#ifdef INSURANCE
void save_currentstate(void);
#endif
void goto_level(d_level *, boolean, boolean, boolean);
void schedule_goto(d_level *, boolean, boolean, int,
		   const char *, const char *);
void deferred_goto(void);
boolean revive_corpse(struct obj *, boolean);
void revive_mon(void *, long);
void moldy_corpse(void *, long);
int donull(void);
int dowipe(void);
void set_wounded_legs(long, int);
void heal_legs(void);

/* ### do_name.c ### */

int getpos(coord *, boolean, const char *);
struct monst *christen_monst(struct monst *, const char *);
int do_mname(void);
struct obj *oname(struct obj *, const char *);
int ddocall(void);
void docall(struct obj *);
const char *rndghostname(void);
char *x_monnam(struct monst *, int, const char *, int, boolean);
char *l_monnam(struct monst *);
char *mon_nam(struct monst *);
char *noit_mon_nam(struct monst *);
char *Monnam(struct monst *);
char *noit_Monnam(struct monst *);
char *m_monnam(struct monst *);
char *y_monnam(struct monst *);
char *Adjmonnam(struct monst *, const char *);
char *Amonnam(struct monst *);
char *a_monnam(struct monst *);
char *distant_monnam(struct monst *, int, char *);
const char *rndmonnam(void);
const char *hcolor(const char *);
const char *rndcolor(void);
const char *roguename(void);
struct obj *realloc_obj(struct obj *, int, void *, int, const char *);
char *coyotename(struct monst *, char *);

/* ### do_wear.c ### */

int Armor_on(void);
int Boots_on(void);
int Cloak_on(void);
int Helmet_on(void);
int Gloves_on(void);
int Shield_on(void);
int Shirt_on(void);
void Amulet_on(void);
void off_msg(struct obj *);
void set_wear(void);
boolean donning(struct obj *);
void cancel_don(void);
int Armor_off(void);
int Armor_gone(void);
int Helmet_off(void);
int Gloves_off(void);
int Boots_off(void);
int Cloak_off(void);
int Shield_off(void);
int Shirt_off(void);
void Amulet_off(void);
void Ring_on(struct obj *);
void Ring_off(struct obj *);
void Ring_gone(struct obj *);
void Blindf_on(struct obj *);
void Blindf_off(struct obj *);
int dotakeoff(void);
int doremring(void);
int cursed(struct obj *);
int armoroff(struct obj *);
int canwearobj(struct obj *, long *, boolean);
int dowear(void);
int doputon(void);
void find_ac(void);
void glibr(void);
struct obj *some_armor(struct monst *);
void erode_armor(struct monst *, boolean);
struct obj *stuck_ring(struct obj *, int);
struct obj *unchanger(void);
void reset_remarm(void);
int doddoremarm(void);
int destroy_arm(struct obj *);
void adj_abon(struct obj *, schar);
int dowear2(const char *, const char *);

/* ### dog.c ### */

void initedog(struct monst *);
struct monst *make_familiar(struct obj *, xchar, xchar, boolean);
struct monst *make_helper(int, xchar, xchar);
struct monst *makedog(void);
void update_mlstmv(void);
void losedogs(void);
void mon_arrive(struct monst *, boolean);
void mon_catchup_elapsed_time(struct monst *, long);
void keepdogs(boolean);
void migrate_to_level(struct monst *, xchar, xchar, coord *);
int dogfood(struct monst *, struct obj *);
struct monst *tamedog(struct monst *, struct obj *);
int make_pet_minion(int, aligntyp);
void abuse_dog(struct monst *);
void wary_dog(struct monst *, boolean);

/* ### dogmove.c ### */

bool cursed_object_at(struct monst *mtmp, int x, int y);
int dog_nutrition(struct monst *mtmp, struct obj *obj);
int dog_eat(struct monst *mtmp, struct obj *obj, int x, int y, bool devour);
int dog_move(struct monst *mtmp, int after);
bool betrayed(struct monst *mtmp);
void finish_meating(struct monst *mtmp);

/* ### dokick.c ### */

boolean ghitm(struct monst *, struct obj *);
void container_impact_dmg(struct obj *);
int dokick(void);
boolean ship_object(struct obj *, xchar, xchar, boolean);
void obj_delivery(bool near_hero);
schar down_gate(xchar, xchar);
void impact_drop(struct obj *, xchar, xchar, xchar);

/* ### dothrow.c ### */

struct obj *splitoneoff(struct obj **);
int dothrow(void);
int dofire(void);
void hitfloor(struct obj *);
void hurtle(int, int, int, boolean);
void mhurtle(struct monst *, int, int, int);
void throwit(struct obj *, long, boolean, int);
int omon_adj(struct monst *, struct obj *, boolean);
int thitmonst(struct monst *, struct obj *, int);
int hero_breaks(struct obj *, xchar, xchar, bool);
int breaks(struct obj *, xchar, xchar);
boolean breaktest(struct obj *);
boolean walk_path(coord *, coord *, boolean (*)(void *, int, int), void *);
boolean hurtle_step(void *, int, int);

/* ### drawing.c ### */
#endif /* !LEV_LEX_C */
int def_char_to_objclass(char ch);
int def_char_to_monclass(char ch);
#ifndef LEV_LEX_C
void assign_graphics(const glyph_t *graph_chars, int glth, int maxlen, int offset);
void assign_colors(uchar *graph_colors, int glth, int maxlen, int offset);
void switch_graphics(int graphics);
void assign_rogue_graphics(bool is_rlevel);

/* ### dungeon.c ### */

void save_dungeon(int, boolean, boolean);
void restore_dungeon(int);
void insert_branch(branch *, boolean);
void init_dungeons(void);
s_level *find_level(const char *);
s_level *Is_special(d_level *);
branch *Is_branchlev(d_level *);
xchar ledger_no(d_level *);
xchar maxledgerno(void);
schar depth(d_level *);
xchar dunlev(d_level *);
xchar dunlevs_in_dungeon(d_level *);
xchar real_dunlevs_in_dungeon(d_level *);
xchar ledger_to_dnum(xchar);
xchar ledger_to_dlev(xchar);
xchar deepest_lev_reached(boolean);
boolean on_level(d_level *, d_level *);
void next_level(boolean);
void prev_level(boolean);
void u_on_newpos(int, int);
void u_on_sstairs(void);
void u_on_upstairs(void);
void u_on_dnstairs(void);
boolean On_stairs(xchar, xchar);
void get_level(d_level *, int);
boolean Is_botlevel(d_level *);
boolean Can_fall_thru(d_level *);
boolean Can_dig_down(d_level *);
boolean Can_rise_up(int, int, d_level *);
boolean In_quest(d_level *);
boolean In_mines(d_level *);
boolean In_spiders(d_level *);
branch *dungeon_branch(const char *);
boolean at_dgn_entrance(const char *);
boolean In_hell(d_level *);
boolean In_V_tower(d_level *);
boolean On_W_tower_level(d_level *);
boolean In_W_tower(int, int, d_level *);
void find_hell(d_level *);
void goto_hell(boolean, boolean);
void assign_level(d_level *, d_level *);
void assign_rnd_level(d_level *, d_level *, int);
int induced_align(int);
boolean Invocation_lev(d_level *);
xchar level_difficulty(void);
schar lev_by_name(const char *);
schar print_dungeon(boolean, schar *, xchar *);
int donamelevel(void);
int dooverview(void);
void forget_mapseen(int);
void init_mapseen(d_level *);
void recalc_mapseen(void);
void recbranch_mapseen(d_level *, d_level *);
void remdun_mapseen(int);

/* ### eat.c ### */

boolean is_edible(struct obj *);
void init_uhunger(void);
int Hear_again(void);
void reset_eat(void);
int doeat(void);
void gethungry(void);
void morehungry(int);
void lesshungry(int);
boolean is_fainted(void);
void reset_faint(void);
void violated_vegetarian(void);
void newuhs(boolean);
bool can_reach_floorobj(void);
void vomit(void);
int eaten_stat(int, struct obj *);
void food_disappears(struct obj *);
void food_substitution(struct obj *, struct obj *);
boolean bite_monster(struct monst *mon);
void fix_petrification(void);
void consume_oeaten(struct obj *, int);
boolean maybe_finished_meal(boolean);
void set_tin_variety(struct obj *obj, int forcetype);
int tin_variety_txt(char *x, int *tin_variety);
bool Popeye(int threat);

/* ### end.c ### */

void done1(int);
int done2(void);
void done_in_by(struct monst *);
#endif /* !LEV_LEX_C */
noreturn void panic(const char *, ...) PRINTF_F(1, 2);
#ifndef LEV_LEX_C
void done(int);
void container_contents(struct obj *, boolean, boolean);
void terminate(int);
int dolistvanq(void);
int num_genocides(void);
/* KMH, ethics */
int doethics(void);
void delayed_killer(int id, int format, nhstr killername);
struct kinfo *find_delayed_killer(int id);
void dealloc_killer(struct kinfo *kptr);
void save_killers(int fd, int mode);
void restore_killers(int fd);

/* ### engrave.c ### */

char *random_engraving(char *);
void wipeout_text(char *, int, unsigned);
boolean can_reach_floor(void);
const char *surface(int, int);
const char *ceiling(int, int);
struct engr *engr_at(xchar, xchar);
int sengr_at(const char *, xchar, xchar);
void u_wipe_engr(int);
void wipe_engr_at(xchar, xchar, xchar);
boolean sense_engr_at(int, int, boolean);
void make_engr_at(int, int, const char *, long, xchar);
void del_engr_at(int, int);
boolean freehand(void);
int doengrave(void);
void save_engravings(int, int);
void rest_engravings(int);
void del_engr(struct engr *);
void rloc_engr(struct engr *);
void make_grave(int, int, const char *);

/* ### exper.c ### */

long newuexp(int);
int experience(struct monst *, int);
void more_experienced(int, int);
void losexp(const char *, boolean);
void newexplevel(void);
void pluslvl(boolean);
long rndexp(boolean);

/* ### explode.c ### */

void explode(xchar, xchar, int, int, char, int);
long scatter(int, int, int, uint, struct obj *);
void splatter_burning_oil(int, int);
void grenade_explode(struct obj *, int, int, boolean, int);
void arm_bomb(struct obj *, boolean);

/* ### extralev.c ### */

void makeroguerooms(void);
void corr(int, int);
void makerogueghost(void);

/* ### files.c ### */

char *fname_encode(const char *, char, char *, char *, int);
char *fname_decode(char, char *, char *, int);
boolean uptodate(int, const char *);
void store_version(int);
void set_levelfile_name(char *, int);
int create_levelfile(int, char *);
int open_levelfile(int, char *);
void delete_levelfile(int);
void clearlocks(void);
int create_bonesfile(d_level *, char **, char *);
void commit_bonesfile(d_level *);
int open_bonesfile(d_level *, char **);
boolean delete_bonesfile(d_level *);
void set_savefile_name(void);
#ifdef INSURANCE
void save_savefile_name(int);
#endif
void set_error_savefile(void);
int create_savefile(void);
int open_savefile(void);
int delete_savefile(void);
int restore_saved_game(void);
boolean lock_file(const char *, int, int);
void unlock_file(const char *);
#ifdef USER_SOUNDS
boolean can_read_file(const char *);
#endif
void read_config_file(const char *);
void check_recordfile(const char *);
int get_uchar_list(char *, uchar *, int);
void read_wizkit(void);
void paniclog(const char *, const char *);
void free_saved_games(char **);
#ifdef SELF_RECOVER
boolean recover_savefile(void);
#endif
#ifdef HOLD_LOCKFILE_OPEN
void really_close(void);
#endif

/* ### fountain.c ### */

void floating_above(const char *);
void dogushforth(int);
void dryup(xchar, xchar, boolean);
void drinkfountain(void);
void dipfountain(struct obj *);
void whetstone_fountain_effects(struct obj *);
void diptoilet(struct obj *);
void breaksink(int, int);
void breaktoilet(int, int);
void drinksink(void);
void drinktoilet(void);
void whetstone_sink_effects(struct obj *);
void whetstone_toilet_effects(struct obj *);

/* ### gypsy.c ### */

void gypsy_init(struct monst *);
void gypsy_chat(struct monst *);

/* ### hack.c ### */

anything int_to_any(int);
anything uint_to_any(uint);
anything monst_to_any(struct monst *);
anything obj_to_any(struct obj *);
anything long_to_any(long l);
void catchup_dgn_growths(int);
void dgn_growths(boolean, boolean);
boolean revive_nasty(int, int, const char *);
void movobj(struct obj *, xchar, xchar);
boolean may_dig(xchar, xchar);
boolean may_passwall(xchar, xchar);
boolean bad_rock(struct monst *, xchar, xchar);
boolean invocation_pos(xchar, xchar);
boolean test_move(int, int, int, int, int);
void domove(void);
void invocation_message(void);
void spoteffects(boolean);
char *in_rooms(xchar, xchar, int);
boolean in_town(int, int);
void check_special_room(boolean);
int dopickup(void);
void lookaround(void);
int monster_nearby(void);
void nomul(int);
void unmul(const char *);
void showdmg(int);
void losehp(int, const char *, int);
int weight_cap(void);
int inv_weight(void);
int near_capacity(void);
int calc_capacity(int);
int max_capacity(void);
boolean check_capacity(const char *);
int inv_cnt(void);
long money_cnt(struct obj *);

/* ### hacklib.c ### */

bool digit(char c);
bool letter(char c);
char highc(char c);
char lowc(char c);
char *lcase(char *s);
char *upstart(char *s);
char *mungspaces(char *bp);
char *eos(char *s);
char *strkitten(char *s, char c);
char *s_suffix(const char *s);
bool onlyspace(const char *s);
char *tabexpand(char *sbuf);
char *visctrl(char c);
char *strsubst(char *bp, const char *orig, const char *replacement);
const char *ordin(uint n);
char *sitoa(int n);
int sgn(int n);
int rounddiv(long x, int y);
int distmin(int x0, int y0, int x1, int y1);
int dist2(int x0, int y0, int x1, int y1);
bool online2(int x0, int y0, int x1, int y1);
bool regmatch(const char *pattern, const char *string, bool caseblind);
#ifndef STRNCMPI
int strncmpi(const char *s1, const char *s2, usize n);
#endif
#ifndef STRSTRI
char *strstri(const char *str, const char *sub);
#endif
bool fuzzymatch(const char *s1, const char *s2, const char *ignore_chars, bool caseblind);
void setrandom(void);
int getyear(void);
int getmonth(void); /* KMH -- Used by gypsies */
long yyyymmdd(time_t date);
long hhmmss(time_t date);
int phase_of_the_moon(void);
bool friday_13th(void);
bool groundhog_day(void); /* KMH -- February 2 */
bool night(void);
bool midnight(void);
void msleep(uint ms);

/* ### invent.c ### */

void assigninvlet(struct obj *);
struct obj *merge_choice(struct obj *, struct obj *);
boolean merged(struct obj **, struct obj **);
void addinv_core1(struct obj *);
void addinv_core2(struct obj *);
struct obj *addinv(struct obj *);
struct obj *hold_another_object(struct obj *, const char *, const char *, const char *);
void useupall(struct obj *);
void useup(struct obj *);
void consume_obj_charge(struct obj *, boolean);
void freeinv_core(struct obj *);
void freeinv(struct obj *);
void delallobj(int, int);
void delobj(struct obj *);
struct obj *sobj_at(int, int, int);
struct obj *nxtobj(struct obj *obj, int type, bool by_nexthere);
struct obj *carrying(int);
boolean have_lizard(void);
struct obj *o_on(uint, struct obj *);
boolean obj_here(struct obj *, int, int);
boolean wearing_armor(void);
bool is_worn(struct obj *);
struct obj *g_at(int, int);
struct obj *mkgoldobj(long);
struct obj *getobj(const char *, const char *);
int ggetobj(const char *, int (*)(struct obj *), int, boolean, unsigned *);
void fully_identify_obj(struct obj *);
int identify(struct obj *);
void identify_pack(int);
int askchain(struct obj **, const char *, int, int (*)(struct obj *),
	     int (*)(struct obj *), int, const char *);
void prinv(const char *, struct obj *, long);
char *xprname(struct obj *, const char *, char, boolean, long, long);
int ddoinv(void);
char display_inventory(const char *, boolean);
int display_binventory(int, int, boolean);
struct obj *display_cinventory(struct obj *);
struct obj *display_minventory(struct monst *, int, char *);
int dotypeinv(void);
const char *dfeature_at(int, int, char *);
int look_here(int, boolean);
int dolook(void);
boolean will_feel_cockatrice(struct obj *, boolean);
void feel_cockatrice(struct obj *, boolean);
void stackobj(struct obj *);
int doprgold(void);
int doprwep(void);
int doprarm(void);
int doprring(void);
int dopramulet(void);
int doprtool(void);
int doprinuse(void);
void useupf(struct obj *, long);
char *let_to_name(char, boolean);
void free_invbuf(void);
void reassign(void);
int doorganize(void);
int count_unpaid(struct obj *);
int count_buc(struct obj *, int);
void carry_obj_effects(struct monst *, struct obj *);
const char *currency(long);
void silly_thing(const char *, struct obj *);
int doinvinuse(void);
/* KMH, balance patch -- new function */
int jumble_pack(void);

/* ### ioctl.c ### */

#ifdef UNIX
void getwindowsz(void);
void getioctls(void);
void setioctls(void);
#ifdef SUSPEND
int dosuspend(void);
#endif /* SUSPEND */
#endif /* UNIX */

/* ### light.c ### */

void new_light_source(xchar, xchar, int, int, anything);
void del_light_source(int, anything);
void do_light_sources(char **);
struct monst *find_mid(unsigned, unsigned);
void save_light_sources(int, int, int);
void restore_light_sources(int);
void relink_light_sources(boolean);
void obj_move_light_source(struct obj *, struct obj *);
boolean any_light_source(void);
void snuff_light_source(int, int);
boolean obj_sheds_light(struct obj *);
boolean obj_is_burning(struct obj *);
boolean obj_permanent_light(struct obj *);
void obj_split_light_source(struct obj *, struct obj *);
void obj_merge_light_sources(struct obj *, struct obj *);
int candle_light_range(struct obj *);
int wiz_light_sources(void);

/* ### lock.c ### */

boolean picking_lock(int *, int *);
boolean picking_at(int, int);
void reset_pick(void);
int pick_lock(struct obj **);
int doforce(void);
boolean boxlock(struct obj *, struct obj *);
boolean doorlock(struct obj *, int, int);
int doopen(void);
int doclose(void);
int artifact_door(int, int);

#ifdef MAC
/* These declarations are here because the main code calls them. */

/* ### macfile.c ### */

int maccreat(const char *, long);
int macopen(const char *, int, long);
int macclose(int);
int macread(int, void *, unsigned);
int macwrite(int, void *, unsigned);
long macseek(int, long, short);
int macunlink(const char *);

/* ### macsnd.c ### */

void mac_speaker(struct obj *, char *);

/* ### macunix.c ### */

void regularize(char *);
void getlock(void);

/* ### macwin.c ### */

void lock_mouse_cursor(Boolean);
int SanePositions(void);

/* ### mttymain.c ### */

void getreturn(const char *);
void msmsg(const char *, ...);
void gettty(void);
void setftty(void);
void settty(const char *);
int tgetch(void);
void cmov(int x, int y);
void nocmov(int x, int y);

#endif /* MAC */

/* ### mail.c ### */

#ifdef MAIL
#ifdef UNIX
void getmailstatus(void);
#endif
void ckmailstatus(void);
void readmail(struct obj *);
#endif /* MAIL */

/* ### makemon.c ### */

boolean is_home_elemental(struct permonst *);
struct monst *clone_mon(struct monst *, xchar, xchar);
void newmonhp(struct monst *mon, int mndx);
struct monst *makemon(struct permonst *, int, int, int);
boolean create_critters(int, struct permonst *);
struct permonst *rndmonst(void);
void reset_rndmonst(int);
struct permonst *mkclass(char, int);
int pm_mkclass(char, int);
int adj_lev(struct permonst *);
struct permonst *grow_up(struct monst *, struct monst *);
int mongets(struct monst *, int);
int golemhp(int);
boolean peace_minded(struct permonst *);
void set_malign(struct monst *);
void set_mimic_sym(struct monst *);
int mbirth_limit(int);
void mimic_hit_msg(struct monst *, short);
void mkmonmoney(struct monst *, long);
void bagotricks(struct obj *);
boolean propagate(int, boolean, boolean);
bool usmellmon(struct permonst *mdat);

/* ### mapglyph.c ### */

void mapglyph(int, glyph_t *, int *, unsigned *, int, int);

/* ### mcastu.c ### */

int castmu(struct monst *, struct attack *, boolean, boolean);
bool buzzmu(struct monst *, struct attack *);

/* ### mhitm.c ### */

int fightm(struct monst *mtmp);
int mattackm(struct monst *magr, struct monst *mdef);
int mdisplacem(struct monst *magr, struct monst *mdef, bool);
bool noattacks(struct permonst *ptr);
int sleep_monst(struct monst *mon, int amt, int how);
void slept_monst(struct monst *mon);
long attk_protection(int aatyp);

/* ### mhitu.c ### */

const char *mpoisons_subj(struct monst *, struct attack *);
void u_slow_down(void);
struct monst *cloneu(void);
void expels(struct monst *, struct permonst *, boolean);
struct attack *getmattk(struct permonst *, int, int *, struct attack *);
int mattacku(struct monst *);
int magic_negation(struct monst *);
bool gulp_blnd_check();
int gazemu(struct monst *, struct attack *);
void mdamageu(struct monst *, int);
int could_seduce(struct monst *, struct monst *, struct attack *);
int doseduce(struct monst *);

/* ### minion.c ### */

int msummon(struct monst *);
void summon_minion(aligntyp, boolean);
bool demon_talk(struct monst *);
int lawful_minion(int);
int neutral_minion(int);
int chaotic_minion(int);
long bribe(struct monst *);
int dprince(aligntyp);
int dlord(aligntyp);
int llord(void);
int ndemon(aligntyp);
int lminion(void);

/* ### mklev.c ### */

void sort_rooms(void);
void add_room(int, int, int, int, boolean, schar, boolean);
void add_subroom(struct mkroom *, int, int, int, int,
		 boolean, schar, boolean);
void makecorridors(void);
int add_door(int, int, struct mkroom *);
void mklev(void);
#ifdef SPECIALIZATION
void topologize(struct mkroom *, boolean);
#else
void topologize(struct mkroom *);
#endif
void place_branch(branch *, xchar, xchar);
boolean occupied(xchar, xchar);
int okdoor(xchar, xchar);
void dodoor(int, int, struct mkroom *);
void mktrap(int, int, struct mkroom *, coord *);
void mkstairs(xchar, xchar, char, struct mkroom *);
void mkinvokearea(void);

/* ### mkmap.c ### */

void flood_fill_rm(int, int, int, boolean, boolean);
void remove_rooms(int, int, int, int);

/* ### mkmaze.c ### */

void wallification(int, int, int, int, boolean);
void walkfrom(int, int);
void makemaz(const char *);
void mazexy(coord *);
void bound_digging(void);
void mkportal(xchar, xchar, xchar, xchar);
boolean bad_location(xchar, xchar, xchar, xchar, xchar, xchar);
void place_lregion(xchar, xchar, xchar, xchar,
		   xchar, xchar, xchar, xchar,
		   xchar, d_level *);
void movebubbles(void);
void water_friction(void);
void save_waterlevel(int, int);
void restore_waterlevel(int);
const char *waterbody_name(xchar, xchar);

/* ### mkobj.c ### */

struct obj *mkobj_at(char, int, int, boolean);
struct obj *mksobj_at(int, int, int, boolean, boolean);
struct obj *mkobj(char, boolean);
int rndmonnum(void);
struct obj *splitobj(struct obj *, long);
void replace_object(struct obj *, struct obj *);
void bill_dummy_object(struct obj *);
struct obj *mksobj(int, boolean, boolean);
int bcsign(struct obj *);
int weight(struct obj *);
struct obj *mkgold(long, int, int);
struct obj *mkcorpstat(int, struct monst *, struct permonst *, int, int, unsigned);
int corpse_revive_type(struct obj *obj);
struct obj *obj_attach_mid(struct obj *, unsigned);
struct monst *get_mtraits(struct obj *, boolean);
struct obj *mk_tt_object(int, int, int);
struct obj *mk_named_object(int, struct permonst *, int, int, const char *);
struct obj *rnd_treefruit_at(int, int);
void start_corpse_timeout(struct obj *);
void bless(struct obj *);
void unbless(struct obj *);
void curse(struct obj *);
void uncurse(struct obj *);
void blessorcurse(struct obj *, int);
boolean is_flammable(struct obj *);
boolean is_rottable(struct obj *);
void place_object(struct obj *, int, int);
void remove_object(struct obj *);
void discard_minvent(struct monst *);
void obj_extract_self(struct obj *);
struct obj *container_extract_indestructable(struct obj *obj);
void extract_nobj(struct obj *, struct obj **);
void extract_nexthere(struct obj *, struct obj **);
int add_to_minv(struct monst *, struct obj *);
struct obj *add_to_container(struct obj *, struct obj *);
void add_to_migration(struct obj *);
void add_to_buried(struct obj *);
void dealloc_obj(struct obj *);
void obj_ice_effects(int, int, boolean);
long peek_at_iced_corpse_age(struct obj *);
void obj_sanity_check(void);

/* ### mkroom.c ### */

void mkroom(int);
void fill_zoo(struct mkroom *);
boolean nexttodoor(int, int);
boolean has_dnstairs(struct mkroom *);
boolean has_upstairs(struct mkroom *);
int somex(struct mkroom *);
int somey(struct mkroom *);
boolean inside_room(struct mkroom *, xchar, xchar);
boolean somexy(struct mkroom *, coord *);
void mkundead(coord *, boolean, int);
struct permonst *courtmon(void);
struct permonst *antholemon(void);
struct permonst *realzoomon(void);
void save_rooms(int);
void rest_rooms(int);
struct mkroom *search_special(schar);

/* ### mon.c ### */

int undead_to_corpse(int);
int genus(int, int);
int pm_to_cham(int);
int minliquid(struct monst *);
int movemon(void);
int meatmetal(struct monst *);
void meatcorpse(struct monst *);
int meatobj(struct monst *);
void mpickgold(struct monst *);
boolean mpickstuff(struct monst *, const char *);
int curr_mon_load(struct monst *);
int max_mon_load(struct monst *);
boolean can_carry(struct monst *, struct obj *);
int mfndpos(struct monst *, coord *, long *, long);
boolean monnear(struct monst *, int, int);
void dmonsfree(void);
int mcalcmove(struct monst *);
void mcalcdistress(void);
void replmon(struct monst *, struct monst *);
void relmon(struct monst *);
struct obj *mlifesaver(struct monst *);
boolean corpse_chance(struct monst *, struct monst *, boolean);
void mondead(struct monst *);
void mondied(struct monst *);
void mongone(struct monst *);
void monstone(struct monst *);
void monkilled(struct monst *, const char *, int);
void mon_xkilled(struct monst *, const char *, int);
void unstuck(struct monst *);
void killed(struct monst *);
void xkilled(struct monst *, int);
void mon_to_stone(struct monst *);
void mnexto(struct monst *);
boolean mnearto(struct monst *, xchar, xchar, boolean);
void poisontell(int);
void poisoned(const char *, int, const char *, int);
void m_respond(struct monst *);
void setmangry(struct monst *);
void wakeup(struct monst *);
void wake_nearby(void);
void wake_nearto(int, int, int);
void seemimic(struct monst *);
void rescham(void);
void restartcham(void);
void restore_cham(struct monst *);
void mon_animal_list(boolean);
int select_newcham_form(struct monst *mon);
int newcham(struct monst *, struct permonst *, boolean, boolean);
int can_be_hatched(int);
int egg_type_from_parent(int, boolean);
boolean dead_species(int, boolean);
void kill_genocided_monsters(void);
void golemeffects(struct monst *, int, int);
boolean angry_guards(boolean);
void pacify_guards(void);
void decide_to_shapeshift(struct monst *mon, int shiftflags);

/* ### mondata.c ### */

void set_mon_data(struct monst *, struct permonst *, int);
struct attack *attacktype_fordmg(struct permonst *, int, int);
boolean attacktype(struct permonst *, int);
boolean poly_when_stoned(struct permonst *);
boolean resists_drli(struct monst *);
boolean resists_magm(struct monst *);
boolean resists_blnd(struct monst *);
boolean can_blnd(struct monst *, struct monst *, uchar, struct obj *);
boolean ranged_attk(struct permonst *);
boolean passes_bars(struct permonst *);
bool can_blow(struct monst *mtmp);
boolean can_track(struct permonst *);
boolean breakarm(struct permonst *);
boolean sliparm(struct permonst *);
boolean sticks(struct permonst *);
int num_horns(struct permonst *);
/* extern boolean canseemon(struct monst *); */
struct attack *dmgtype_fromattack(struct permonst *, int, int);
boolean dmgtype(struct permonst *, int);
int max_passive_dmg(struct monst *, struct monst *);
bool same_race(struct permonst *pm1, struct permonst *pm2);
int monsndx(struct permonst *);
int name_to_mon(const char *);
int gender(struct monst *);
int pronoun_gender(struct monst *);
boolean levl_follower(struct monst *);
int little_to_big(int);
int big_to_little(int);
const char *locomotion(const struct permonst *, const char *);
const char *stagger(const struct permonst *, const char *);
const char *on_fire(struct permonst *, struct attack *);
const struct permonst *raceptr(struct monst *);
bool olfaction(struct permonst *mdat);
bool is_vampshifter(struct monst *mon);

/* ### monmove.c ### */

boolean itsstuck(struct monst *);
boolean mb_trapped(struct monst *);
void mon_regen(struct monst *, boolean);
int dochugw(struct monst *);
boolean onscary(int, int, struct monst *);
void monflee(struct monst *, int, boolean, boolean);
int dochug(struct monst *);
int m_move(struct monst *, int);
boolean closed_door(int, int);
boolean accessible(int, int);
void set_apparxy(struct monst *);
bool can_ooze(struct monst *mtmp);
bool can_fog(struct monst *mtmp);
bool should_displace(struct monst *mtmp, coord *poss, long *info, int cnt, xchar gx, xchar gy);
bool undesirable_disp(struct monst *mtmp, xchar x, xchar y);

/* ### monst.c ### */

void monst_init(void);

/* ### monstr.c ### */
int mstrength(struct permonst *mon);

/* ### mplayer.c ### */

struct monst *mk_mplayer(struct permonst *, xchar, xchar, boolean);
void create_mplayers(int, boolean);
void mplayer_talk(struct monst *);

/* ### unicode.c ### */
void unicode_to_utf8(glyph_t, char[5]);
char *utf8_tmpstr(glyph_t);
void pututf8char(glyph_t);

#ifdef WIN32

/* ### winnt.c ### */

#ifndef WIN32
int tgetch(void);
#endif
char switchar(void);
long freediskspace(char *);
int findfirst(char *);
int findnext(void);
long filesize(char *);
char *foundfile_buffer(void);
#ifndef __CYGWIN__
void chdrive(char *);
#endif
void disable_ctrlP(void);
void enable_ctrlP(void);
#ifdef WIN32
char *get_username(int *);
int set_binary_mode(int, int);
void nt_regularize(char *);
int(*t_kbhit(void));
void Delay(int);
#endif /* WIN32 */
#endif /* WIN32 */

/* ### mthrowu.c ### */

int thitu(int, int, struct obj *, const char *);
int ohitmon(struct monst *, struct monst *, struct obj *, int, boolean);
void thrwmu(struct monst *);
int spitmu(struct monst *, struct attack *);
int breamu(struct monst *, struct attack *);
boolean breamspot(struct monst *, struct attack *, xchar, xchar);
boolean linedup(xchar, xchar, xchar, xchar);
boolean lined_up(struct monst *);
struct obj *m_carrying(struct monst *, int);
void m_useup(struct monst *, struct obj *);
void m_throw(struct monst *, int, int, int, int, int, struct obj *);
boolean hits_bars(struct obj **, int, int, int, int);

/* ### muse.c ### */

boolean find_defensive(struct monst *);
int use_defensive(struct monst *);
int rnd_defensive_item(struct monst *);
boolean find_offensive(struct monst *);
int use_offensive(struct monst *);
int rnd_offensive_item(struct monst *);
boolean find_misc(struct monst *);
int use_misc(struct monst *);
int rnd_misc_item(struct monst *);
boolean searches_for_item(struct monst *, struct obj *);
boolean mon_reflects(struct monst *, const char *);
boolean ureflects(const char *, const char *);
boolean munstone(struct monst *, boolean);

/* ### music.c ### */

void awaken_soldiers(void);
int do_play_instrument(struct obj *);

/* ### nhlan.c ### */
#ifdef LAN_FEATURES
void init_lan_features(void);
char *lan_username(void);
#ifdef LAN_MAIL
bool lan_mail_check(void);
void lan_mail_read(struct obj *);
void lan_mail_init(void);
void lan_mail_finish(void);
void lan_mail_terminate(void);
#endif
#endif

/* ### o_init.c ### */

void init_objects(void);
void obj_shuffle_range(int otyp, int *lo_p, int *hi_p);
int find_skates(void);
void oinit(void);
void savenames(int, int);
void restnames(int);
void discover_object(int, boolean, boolean);
void undiscover_object(int);
int dodiscovered(void);

/* ### objects.c ### */

void objects_init(void);

/* ### objnam.c ### */

char *obj_typename(int);
char *simple_typename(int);
boolean obj_is_pname(struct obj *);
char *distant_name(struct obj *, char *(*)(struct obj *));
char *fruitname(boolean);
char *xname(struct obj *);
char *mshot_xname(struct obj *);
boolean the_unique_obj(struct obj *obj);
char *doname(struct obj *);
bool not_fully_identified(struct obj *);
char *corpse_xname(struct obj *, boolean);
char *cxname(struct obj *);
char *killer_xname(struct obj *);
char *killer_cxname(struct obj *, boolean);
const char *singular(struct obj *, char *(*)(struct obj *));
char *an(const char *);
char *An(const char *);
char *The(const char *);
char *the(const char *);
char *aobjnam(struct obj *, const char *);
char *yobjnam(struct obj *obj, const char *verb);
char *Yobjnam2(struct obj *obj, const char *verb);
char *Tobjnam(struct obj *, const char *);
char *otense(struct obj *, const char *);
char *vtense(const char *, const char *);
char *Doname2(struct obj *);
char *yname(struct obj *);
char *Yname2(struct obj *);
char *ysimple_name(struct obj *);
char *Ysimple_name2(struct obj *);
char *makeplural(const char *);
char *makesingular(const char *);
struct obj *readobjnam(char *bp, struct obj *no_wish);
int rnd_class(int, int);
const char *cloak_simple_name(const struct obj *cloack);
const char *helm_simple_name(const struct obj *helmet);
const char *mimic_obj_name(const struct monst *mtmp);

/* ### options.c ### */

boolean match_optname(const char *, const char *, int, boolean);
void initoptions(void);
bool parse_monster_symbol(const char *);
bool parse_object_symbol(const char *);
bool parse_symbol(const char *);
void parseoptions(char *, boolean, boolean);
void parsetileset(char *);
int doset(void);
int dotogglepickup(void);
void option_help(void);
void next_opt(winid, const char *);
int fruitadd(char *);
int choose_classes_menu(const char *, int, boolean, char *, char *);
void add_menu_cmd_alias(char, char);
char map_menu_cmd(char);
void assign_warnings(uchar *);
char *nh_getenv(const char *);
void set_duplicate_opt_detection(int);
void set_wc_option_mod_status(unsigned long, int);
void set_wc2_option_mod_status(unsigned long, int);
void set_option_mod_status(const char *, int);
boolean add_menu_coloring(char *);
int add_autopickup_exception(const char *);
void free_autopickup_exceptions(void);

/* ### pager.c ### */

int dowhatis(void);
int doquickwhatis(void);
int doidtrap(void);
int dowhatdoes(void);
char *dowhatdoes_core(char, char *);
int dohelp(void);
int dohistory(void);
int do_look(int mode, coord *click_cc);

/* ### pcmain.c ### */

#ifdef WIN32
#ifdef CHDIR
void chdirx(char *, bool);
#endif /* CHDIR */
#endif /* WIN32 */

/* ### pcsys.c ### */

#if defined(WIN32)
void flushout(void);
int dosh(void);
void append_slash(char *);
void getreturn(const char *);
void msmsg(const char *, ...);
FILE *fopenp(const char *, const char *);
#endif /* WIN32 */

/* ### pctty.c ### */

#ifdef WIN32
void gettty(void);
void settty(const char *);
void setftty(void);
void error(const char *, ...);
#ifdef _MSC_VER
void msleep(unsigned);
#endif
#endif /* WIN32 */

/* ### pcunix.c ### */

#if defined(PC_LOCKING)
void getlock(void);
#endif

/* ### pickup.c ### */

int collect_obj_classes(char *, struct obj *, boolean, bool (*)(struct obj *), int *);
void add_valid_menu_class(int);
bool allow_all(struct obj *);
bool allow_category(struct obj *);
bool is_worn_by_type(struct obj *);
boolean mbag_explodes(struct obj *, int);
void destroy_mbag(struct obj *, boolean);
int pickup(int);
int pickup_object(struct obj *, long, boolean);
int query_category(const char *, struct obj *, int,
		   menu_item **, int);
int query_objlist(const char *, struct obj *, int, menu_item **, int, bool (*)(struct obj *));
struct obj *pick_obj(struct obj *);
int encumber_msg(void);
int doloot(void);
boolean container_gone(int (*)(struct obj *));
int use_container(struct obj **, int);
int loot_mon(struct monst *, int *, boolean *);
const char *safe_qbuf(const char *, unsigned,
		      const char *, const char *, const char *);
boolean is_autopickup_exception(struct obj *, boolean);
int dotip(void);

/* ### pline.c ### */

// for safety; if you say pline(x), where x is untrusted user data, then the
// controller of that data has arbitrary code execution.  Maybe.
#define plines(str)	pline("%s", (str))
#define Noreps(str)	Norep("%s", (str))
#define verbalizes(str) verbalize("%s", (str))
#define You_hear(str)	You_hearf("%s", (str))
void msgpline_add(int typ, char *pattern);
void msgpline_free(void);
void pline(const char *line, ...) PRINTF_F(1, 2);
void Norep(const char *line, ...) PRINTF_F(1, 2);
void hear(const char *fmt, ...) PRINTF_F(1, 2);
void You_hearf(const char *line, ...) PRINTF_F(1, 2);
void verbalize(const char *line, ...) PRINTF_F(1, 2);
void raw_printf(const char *line, ...) PRINTF_F(1, 2);
void impossible(const char *s, ...) PRINTF_F(1, 2);
const char *align_str(aligntyp alignment);
void mstatusline(struct monst *mtmp);
void ustatusline(void);
void self_invis_message(void);

/* ### polyself.c ### */

void init_uasmon(void);
void change_sex(void);
void polyself(int psflags);
int polymon(int);
void rehumanize(void);
int dobreathe(void);
int dospit(void);
int doremove(void);
int dospinweb(void);
int dosummon(void);
int dogaze(void);
int dohide(void);
int dopoly(void);
int domindblast(void);
void skinback(boolean);
const char *mbodypart(struct monst *, int);
const char *body_part(int);
int poly_gender(void);
void ugolemeffects(int, int);
int polyatwill(void);

/* ### potion.c ### */

void set_itimeout(long *, long);
void incr_itimeout(long *, int);
void make_confused(long, boolean);
void make_stunned(long, boolean);
void make_blinded(long, boolean);
void make_sick(long, const char *, boolean, int);
void make_slimed(long xtime, const char *msg);
void make_vomiting(long, boolean);
boolean make_hallucinated(long, boolean, long);
int dodrink(void);
int dopotion(struct obj *);
int peffects(struct obj *);
void healup(int, int, boolean, boolean);
void strange_feeling(struct obj *, const char *);
void potionhit(struct monst *, struct obj *, boolean);
void potionbreathe(struct obj *);
boolean get_wet(struct obj *, boolean);
int dodip(void);
void djinni_from_bottle(struct obj *);
/* KMH, balance patch -- new function */
int upgrade_obj(struct obj *);
struct monst *split_mon(struct monst *, struct monst *);
const char *bottlename(void);

/* ### pray.c ### */

int dosacrifice(void);
boolean can_pray(boolean);
int dopray(void);
const char *u_gname(void);
int doturn(void);
int turn_undead(void);
const char *a_gname(void);
const char *a_gname_at(xchar x, xchar y);
const char *align_gname(aligntyp);
const char *halu_gname(aligntyp);
const char *align_gtitle(aligntyp);
void altar_wrath(int, int);

/* ### priest.c ### */

int move_special(struct monst *, boolean, schar, boolean, boolean,
		 xchar, xchar, xchar, xchar);
char temple_occupied(char *);
int pri_move(struct monst *);
void priestini(d_level *, struct mkroom *, int, int, boolean);
char *priestname(struct monst *, char *);
boolean p_coaligned(struct monst *);
struct monst *findpriest(char);
void intemple(int);
void priest_talk(struct monst *);
struct monst *mk_roamer(struct permonst *, aligntyp,
			xchar, xchar, boolean);
void reset_hostility(struct monst *);
boolean in_your_sanctuary(struct monst *, xchar, xchar);
void ghod_hitsu(struct monst *);
void angry_priest(void);
void clearpriests(void);
void restpriest(struct monst *, boolean);

/* ### quest.c ### */

void onquest(void);
void nemdead(void);
void artitouch(void);
boolean ok_to_quest(void);
void leader_speaks(struct monst *);
void nemesis_speaks(void);
void quest_chat(struct monst *);
void quest_talk(struct monst *);
void quest_stat_check(struct monst *);
void finish_quest(struct obj *);

/* ### questpgr.c ### */

void load_qtlist(void);
void unload_qtlist(void);
short quest_info(int);
const char *ldrname(void);
bool is_quest_artifact(struct obj *);
void com_pager(int);
void qt_pager(int);
struct permonst *qt_montype(void);

/* ### random.c ### */

#ifdef RANDOM
void srandom(unsigned);
char *initstate(unsigned, char *, int);
char *setstate(char *);
long random(void);
#endif /* RANDOM */

/* ### read.c ### */

int doread(void);
boolean is_chargeable(struct obj *);
void recharge(struct obj *, int);
void forget(int);
void forget_objects(int);
void forget_levels(int);
void forget_traps(void);
void forget_map(int);
int seffects(struct obj *);
void litroom(boolean, struct obj *);
void do_genocide(int);
void punish(struct obj *);
void unpunish(void);
boolean cant_create(int *, boolean);
struct monst *create_particular(void);

/* ### rect.c ### */

void init_rect(void);
NhRect *get_rect(NhRect *);
NhRect *rnd_rect(void);
void remove_rect(NhRect *);
void add_rect(NhRect *);
void split_rects(NhRect *, NhRect *);

/* ## region.c ### */
void clear_regions(void);
void run_regions(void);
boolean in_out_region(xchar, xchar);
boolean m_in_out_region(struct monst *, xchar, xchar);
void update_player_regions(void);
void update_monster_region(struct monst *);
NhRegion *visible_region_at(xchar, xchar);
void show_region(NhRegion *, xchar, xchar);
void save_regions(int, int);
void rest_regions(int, boolean);
NhRegion *create_gas_cloud(xchar, xchar, int, int);
NhRegion *create_cthulhu_death_cloud(xchar, xchar, int, int);

/* ### restore.c ### */

void inven_inuse(boolean);
int dorecover(int);
void trickery(char *);
void getlev(int, int, xchar, boolean);
boolean lookup_id_mapping(unsigned, unsigned *);
void mread(int, void *, uint);

/* ### rip.c ### */

void genl_outrip(winid tmpwin, int how);

/* ### rnd.c ### */

uint rn1(uint x, uint y);
uint rn2(uint x);
uint rnl(uint x);
uint rnd(uint x);
uint d(uint n, uint x);
uint rne(uint x);
uint rnz(uint i);
void seed_good_random(char data[64]);
uint good_random(void);

/* ### role.c ### */

boolean validrole(int);
boolean validrace(int, int);
boolean validgend(int, int, int);
boolean validalign(int, int, int);
int randrole(void);
int randrace(int);
int randgend(int, int);
int randalign(int, int);
int str2role(char *);
int str2race(char *);
int str2gend(char *);
int str2align(char *);
int mrace2race(int);
boolean ok_role(int, int, int, int);
int pick_role(int, int, int, int);
boolean ok_race(int, int, int, int);
int pick_race(int, int, int, int);
boolean ok_gend(int, int, int, int);
int pick_gend(int, int, int, int);
boolean ok_align(int, int, int, int);
int pick_align(int, int, int, int);
void role_init(void);
void rigid_role_checks(void);
void plnamesuffix(void);
const char *Hello(struct monst *);
const char *Goodbye(void);
char *build_plselection_prompt(char *, int, int, int, int, int);
char *root_plselection_prompt(char *, int, int, int, int, int);

/* ### rumors.c ### */

char *getrumor(int, char *, bool);
void outrumor(int, int);
void outoracle(boolean, boolean);
int doconsult(struct monst *);

/* ### save.c ### */

int dosave(void);
#if defined(UNIX) || defined(__EMX__) || defined(WIN32)
void hangup(int);
#endif
int dosave0(void);
#ifdef INSURANCE
void savestateinlock(void);
#endif
void savelev(int, xchar, int);
void bufon(int);
void bufoff(int);
void bflush(int);
void bwrite(int, const void *, uint);
void bclose(int);
void savefruitchn(int, int);
void free_dungeons(void);
void freedynamicdata(void);

/* ### shk.c ### */

long money2mon(struct monst *, long);
void money2u(struct monst *, long);
void shkgone(struct monst *);
void set_residency(struct monst *, boolean);
void replshk(struct monst *, struct monst *);
void save_shk_bill(int, struct monst *);
void restore_shk_bill(int, struct monst *);
void restshk(struct monst *, boolean);
char inside_shop(xchar, xchar);
void u_left_shop(char *, boolean);
void remote_burglary(xchar, xchar);
void u_entered_shop(char *);
boolean same_price(struct obj *, struct obj *);
void shopper_financial_report(void);
int inhishop(struct monst *);
struct monst *shop_keeper(char);
boolean tended_shop(struct mkroom *);
void delete_contents(struct obj *);
void obfree(struct obj *, struct obj *);
void shk_free(struct monst *);
void home_shk(struct monst *, boolean);
void make_happy_shk(struct monst *, boolean);
void hot_pursuit(struct monst *);
void make_angry_shk(struct monst *, xchar, xchar);
int dopay(void);
boolean paybill(int);
void finish_paybill(void);
struct obj *find_oid(unsigned);
long contained_cost(struct obj *, struct monst *, long, boolean, boolean);
long contained_gold(struct obj *);
void picked_container(struct obj *);
long unpaid_cost(struct obj *);
void addtobill(struct obj *, boolean, boolean, boolean);
void splitbill(struct obj *, struct obj *);
void subfrombill(struct obj *, struct monst *);
long stolen_value(struct obj *, xchar, xchar, boolean, boolean,
		  boolean);
void sellobj_state(int);
void sellobj(struct obj *, xchar, xchar);
int doinvbill(int);
struct monst *shkcatch(struct obj *, xchar, xchar);
void add_damage(xchar, xchar, long);
int repair_damage(struct monst *, struct damage *, boolean);
int shk_move(struct monst *);
void after_shk_move(struct monst *);
boolean is_fshk(struct monst *);
void shopdig(int);
void pay_for_damage(const char *, boolean);
boolean costly_spot(xchar, xchar);
struct obj *shop_object(xchar, xchar);
void price_quote(struct obj *);
void shk_chat(struct monst *);
void check_unpaid_usage(struct obj *, boolean);
void check_unpaid(struct obj *);
void costly_gold(xchar, xchar, long);
boolean block_door(xchar, xchar);
boolean block_entry(xchar, xchar);
boolean block_entry(xchar, xchar);
void blkmar_guards(struct monst *);
char *shk_your(char *, struct obj *);
char *Shk_Your(char *, struct obj *);

/* ### shknam.c ### */

void stock_room(int shp_indx, struct mkroom *sroom);
bool saleable(struct monst *shkp, struct obj *obj);
int get_shop_item(int type);
const char *shkname(struct monst *mtmp);
bool shkname_is_pname(struct monst *mtmp);

/* ### sit.c ### */

void take_gold(void);
int dosit(void);
void rndcurse(void);
void attrcurse(void);

/* ### sounds.c ### */

void dosounds(void);
void pet_distress(struct monst *, int);
const char *growl_sound(struct monst *);
/* JRN: converted growl,yelp,whimper to macros based on pet_distress.
  Putting them here since I don't know where else (TOFIX) */
#define growl(mon)   pet_distress((mon), 3)
#define yelp(mon)    pet_distress((mon), 2)
#define whimper(mon) pet_distress((mon), 1)
void beg(struct monst *);
int dotalk(void);
#ifdef USER_SOUNDS
int add_sound_mapping(const char *);
void play_sound_for_message(const char *);
#endif

/* ### sp_lev.c ### */

boolean check_room(xchar *, xchar *, xchar *, xchar *, boolean);
boolean create_room(xchar, xchar, xchar, xchar,
		    xchar, xchar, xchar, xchar);
void create_secret_door(struct mkroom *, xchar);
boolean dig_corridor(coord *, coord *, boolean, schar, schar);
void fill_room(struct mkroom *, boolean);
boolean load_special(const char *);

/* ### spell.c ### */

int study_book(struct obj *);
void book_disappears(struct obj *);
void book_substitution(struct obj *, struct obj *);
void age_spells(void);
int docast(void);
int spell_skilltype(int);
int spelleffects(int, boolean);
void losespells(void);
int dovspell(void);
void learnspell(struct obj *);
boolean studyspell(void);
void initialspell(struct obj *);

/* ### steal.c ### */

long somegold(long);
void stealgold(struct monst *);
void remove_worn_item(struct obj *, boolean);
int steal(struct monst *, char *);
int mpickobj(struct monst *, struct obj *);
void stealamulet(struct monst *);
void mdrop_special_objs(struct monst *);
void relobj(struct monst *, int, boolean);
struct obj *findgold(struct obj *);

/* ### steed.c ### */

void rider_cant_reach(void);
boolean can_saddle(struct monst *);
int use_saddle(struct obj *);
boolean can_ride(struct monst *);
int doride(void);
boolean mount_steed(struct monst *, boolean);
void exercise_steed(void);
void kick_steed(void);
void dismount_steed(int);
void place_monster(struct monst *, int, int);

/* ### tech.c ### */

void adjtech(int, int);
int dotech(void);
void docalm(void);
int tech_inuse(int);
void tech_timeout(void);
boolean tech_known(short);
void learntech(short, long, int);

/* ### teleport.c ### */

boolean goodpos(int, int, struct monst *, unsigned);
boolean enexto(coord *, xchar, xchar, struct permonst *);
boolean enexto_core(coord *, xchar, xchar, struct permonst *, unsigned);
int epathto(coord *, int, xchar, xchar, struct permonst *);
boolean wpathto(coord *, coord *, boolean (*)(void *, int, int),
		void *, struct permonst *, int);
void xpathto(int, xchar, xchar, int (*)(void *, int, int), void *);
void teleds(int, int, boolean);
boolean safe_teleds(boolean);
boolean teleport_pet(struct monst *, boolean);
void tele(void);
int dotele(void);
void level_tele(void);
void domagicportal(struct trap *);
void tele_trap(struct trap *);
void level_tele_trap(struct trap *);
void rloc_to(struct monst *, int, int);
boolean rloc(struct monst *, boolean);
boolean tele_restrict(struct monst *);
void mtele_trap(struct monst *, struct trap *, int);
int mlevel_tele_trap(struct monst *, struct trap *, boolean, int);
bool rloco(struct obj *obj);
int random_teleport_level(void);
boolean u_teleport_mon(struct monst *, boolean);

/* ### tile.c ### */
#ifdef USE_TILES
void substitute_tiles(d_level *);
#endif

/* ### timeout.c ### */

void burn_away_slime(void);
void nh_timeout(void);
void fall_asleep(int how_long, bool wakeup_msg);
void set_obj_poly(struct obj *, struct obj *);
void unpoly_obj(void *, long);
int mon_poly(struct monst *, boolean, const char *);
int mon_spec_poly(struct monst *, struct permonst *, long,
		  boolean, boolean, boolean, boolean);
void unpoly_mon(void *, long);
void attach_bomb_blow_timeout(struct obj *, int, boolean);
void attach_egg_hatch_timeout(struct obj *);
void attach_fig_transform_timeout(struct obj *);
void kill_egg(struct obj *);
void hatch_egg(void *, long);
void learn_egg_type(int);
void burn_object(void *, long);
void begin_burn(struct obj *, boolean);
void end_burn(struct obj *, boolean);
void burn_faster(struct obj *, long);
void lightsaber_deactivate(struct obj *, boolean);
void do_storms(void);
boolean start_timer(long, short, short, anything);
long stop_timer(short, anything);
long peek_timer(short, anything);
void run_timers(void);
void obj_move_timers(struct obj *, struct obj *);
void obj_split_timers(struct obj *, struct obj *);
void obj_stop_timers(struct obj *);
bool obj_has_timer(struct obj*, short);
void spot_stop_timers(xchar x, xchar y, short func_index);
long spot_time_expires(xchar x, xchar y, short func_index);
long spot_time_left(xchar x, xchar y, short func_index);
void mon_stop_timers(struct monst *);
boolean obj_is_local(struct obj *);
void save_timers(int, int, int);
void restore_timers(int, int, boolean, long);
void relink_timers(boolean);
int wiz_timeout_queue(void);
void timer_sanity_check(void);

/* ### topten.c ### */

void topten(int);
void prscore(int, char **);
struct obj *tt_oname(struct obj *);
#ifdef PROXY_GRAPHICS
winid create_toptenwin(void);
void destroy_toptenwin(void);
#endif

/* ### track.c ### */

void initrack(void);
void settrack(void);
coord *gettrack(int, int);

/* ### trap.c ### */

boolean burnarmor(struct monst *);
boolean rust_dmg(struct obj *, const char *, int, boolean, struct monst *);
void grease_protect(struct obj *, const char *, struct monst *);
struct trap *maketrap(int, int, int);
void fall_through(boolean);
struct monst *animate_statue(struct obj *, xchar, xchar, int, int *);
struct monst *activate_statue_trap(struct trap *, xchar, xchar, boolean);
void dotrap(struct trap *, unsigned);
void seetrap(struct trap *);
int mintrap(struct monst *);
void instapetrify(const char *);
void minstapetrify(struct monst *, boolean);
void selftouch(const char *);
void mselftouch(struct monst *, const char *, boolean);
void float_up(void);
void fill_pit(int, int);
int float_down(long, long);
int fire_damage(struct obj *, boolean, boolean, xchar, xchar);
bool water_damage(struct obj *, boolean, boolean);
boolean drown(void);
void mon_drain_en(struct monst *, int);
void drain_en(int);
int dountrap(void);
int untrap(boolean);
boolean chest_trap(struct obj *, int, boolean);
void deltrap(struct trap *);
boolean delfloortrap(struct trap *);
struct trap *t_at(int, int);
void b_trapped(const char *, int);
boolean unconscious(void);
boolean lava_effects(void);
void blow_up_landmine(struct trap *);
int launch_obj(short, int, int, int, int, int);
bool launch_in_progress(void);
void force_launch_placement(void);
/* KMH, balance patch -- new function */
int uunstone(void);
bool uteetering_at_seen_pit(void);
bool uteetering_at_seen_hole(void);
bool uteetering_at_seen_trap(void);

/* ### u_init.c ### */

void u_init(void);

/* ### uhitm.c ### */

void hurtmarmor(struct monst *mdef, int attk);
int attack_checks(struct monst *mtmp, bool barehanded);
void check_caitiff(struct monst *mtmp);
int find_roll_to_hit(struct monst *mtmp, bool *monk_armor_penalty);
bool attack(struct monst *mtmp);
bool hmon(struct monst *mon, struct obj *obj, int thrown);
int damageum(struct monst *mdef, struct attack *mattk);
void missum(struct monst *mdef, int target, int roll, struct attack *mattk, bool encumbered_by_armor);
int passive(struct monst *mon, int mhit, int malive, uchar aatyp, bool wep_was_destroyed);
void passive_obj(struct monst *mon, struct obj *obj, struct attack *mattk);
void stumble_onto_mimic(struct monst *mtmp);
int flash_hits_mon(struct monst *mtmp, struct obj *otmp);

/* ### unixmain.c ### */

#if defined(UNIX) && defined(PORT_HELP)
void port_help(void);
#endif // UNIX && PORT_HELP

/* ### unixtty.c ### */

#ifdef UNIX
void gettty(void);
void settty(const char *);
void setftty(void);
void intron(void);
void introff(void);
void error(const char *, ...) PRINTF_F(1, 2);
#endif /* UNIX */

/* ### unixunix.c ### */

#ifdef UNIX
void getlock(void);
void regularize(char *);
#ifdef SHELL
int dosh(void);
#endif /* SHELL */
#if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
int child(int);
#endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
int hide_privileges(boolean);
#endif /* UNIX */

/* ### vault.c ### */

boolean grddead(struct monst *);
char vault_occupied(char *);
void invault(void);
int gd_move(struct monst *);
void paygd(void);
long hidden_gold(void);
boolean gd_sound(void);

/* ### version.c ### */

char *version_string_tmp(void);
char *full_version_string_tmp();
int doversion(void);
bool check_version(struct version_info *, const char *, boolean);
#ifdef RUNTIME_PORT_ID
void append_port_id(char *);
#endif

/* ### video.c ### */

#ifdef VIDEOSHADES
int assign_videoshades(char *);
int assign_videocolors(char *);
#endif

/* ### vision.c ### */

void vision_init(void);
int does_block(int, int, struct rm *);
void vision_reset(void);
void vision_recalc(int);
void block_point(int, int);
void unblock_point(int, int);
bool clear_path(int, int, int, int);
void do_clear_area(int, int, int,
		   void (*)(int, int, void *), void *);

/* ### weapon.c ### */

const char *weapon_descr(struct obj *obj);
int hitval(struct obj *, struct monst *);
int dmgval(struct obj *, struct monst *);
struct obj *select_rwep(struct monst *);
struct obj *select_hwep(struct monst *);
void possibly_unwield(struct monst *, boolean);
int mon_wield_item(struct monst *);
int abon(void);
int dbon(void);
int enhance_weapon_skill(void);
void unrestrict_weapon_skill(int);
void use_skill(int, int);
void add_weapon_skill(int);
void lose_weapon_skill(int);
int weapon_type(struct obj *);
int uwep_skill_type(void);
int weapon_hit_bonus(struct obj *);
int weapon_dam_bonus(struct obj *);
int skill_bonus(int);
void skill_init(const struct def_skill *);
void practice_weapon(void);

/* ### were.c ### */

int counter_were(int);
void were_change(struct monst *);
void new_were(struct monst *);
int were_summon(struct permonst *, boolean, int *, char *);
void you_were(void);
void you_unwere(boolean);

/* ### wield.c ### */

void setuwep(struct obj *, boolean);
void setuqwep(struct obj *);
void setuswapwep(struct obj *, boolean);
int dowield(void);
int doswapweapon(void);
int dowieldquiver(void);
boolean wield_tool(struct obj *, const char *);
int can_twoweapon(void);
void drop_uswapwep(void);
int dotwoweapon(void);
void uwepgone(void);
void uswapwepgone(void);
void uqwepgone(void);
void untwoweapon(void);
bool erode_obj(struct obj *target, bool acid_dmg, bool fade_scrolls, bool for_dip);
int chwepon(struct obj *, int);
int welded(struct obj *);
void weldmsg(struct obj *);
void setmnotwielded(struct monst *, struct obj *);
void unwield(struct obj *, boolean);

/* ### windows.c ### */

void choose_windows(const char *);
char genl_message_menu(char, int, const char *);
void genl_preference_update(const char *);
void dump_redirect(bool onoff_flag);
void dump_open_log(time_t now);
void dump_close_log(void);
void dump_forward_putstr(winid win, int attr, const char *str, int no_forward);

/* ### wizard.c ### */

void amulet(void);
int mon_has_amulet(struct monst *);
int mon_has_special(struct monst *);
int tactics(struct monst *);
void aggravate(void);
void clonewiz(void);
int pick_nasty(void);
int nasty(struct monst *);
void resurrect(void);
void intervene(void);
void wizdead(void);
void cuss(struct monst *);

/* ### worm.c ### */

int get_wormno(void);
void initworm(struct monst *, int);
void worm_move(struct monst *);
void worm_nomove(struct monst *);
void wormgone(struct monst *);
void wormhitu(struct monst *);
int cutworm(struct monst *, xchar, xchar, struct obj *);
void see_wsegs(struct monst *);
void detect_wsegs(struct monst *, boolean);
void save_worm(int, int);
void rest_worm(int);
void place_wsegs(struct monst *);
void remove_worm(struct monst *);
void place_worm_tail_randomly(struct monst *, xchar, xchar);
int count_wsegs(struct monst *);
boolean worm_known(struct monst *);

/* ### worn.c ### */

void setworn(struct obj *, long);
void setnotworn(struct obj *);
void mon_set_minvis(struct monst *);
void mon_adjust_speed(struct monst *, int, struct obj *);
void update_mon_intrinsics(struct monst *, struct obj *, boolean, boolean);
int find_mac(struct monst *);
void m_dowear(struct monst *, boolean);
struct obj *which_armor(struct monst *, long);
void mon_break_armor(struct monst *, boolean);
void bypass_obj(struct obj *);
void clear_bypasses(void);
int racial_exception(struct monst *, struct obj *);

/* ### write.c ### */

int dowrite(struct obj *);

/* ### zap.c ### */

int bhitm(struct monst *, struct obj *);
void probe_monster(struct monst *);
boolean get_obj_location(struct obj *, xchar *, xchar *, int);
boolean get_mon_location(struct monst *, xchar *, xchar *, int);
struct monst *get_container_location(struct obj *obj, int *, int *);
struct monst *montraits(struct obj *, coord *);
struct monst *revive(struct obj *);
int unturn_dead(struct monst *);
void cancel_item(struct obj *);
boolean drain_item(struct obj *); /* KMH */
boolean obj_resists(struct obj *, int, int);
boolean obj_shudders(struct obj *);
void do_osshock(struct obj *);
void puton_worn_item(struct obj *);
void obj_set_oinvis(struct obj *, boolean, boolean);
struct obj *poly_obj(struct obj *, int);
int bhito(struct obj *, struct obj *);
int bhitpile(struct obj *, int (*)(struct obj *, struct obj *), int, int);
int zappable(struct obj *);
void zapnodir(struct obj *);
int dozap(void);
uint zapyourself(struct obj *, boolean);
bool flashburn(long duration);
boolean cancel_monst(struct monst *, struct obj *,
		     boolean, boolean, boolean);
void weffects(struct obj *);
int spell_damage_bonus(int);
const char *exclam(int force);
void hit(const char *, struct monst *, const char *);
void miss(const char *, struct monst *);
struct monst *bhit(int, int, int, int, int (*)(struct monst *, struct obj *),
		   int (*)(struct obj *, struct obj *), struct obj **);
struct monst *boomhit(int, int);
int burn_floor_paper(int, int, boolean, boolean);
void buzz(int, int, xchar, xchar, int, int);
void melt_ice(xchar x, xchar y, const char *msg);
void start_melt_ice_timeout(xchar x, xchar y);
void melt_ice_away(void *arg, long timeout);
int zap_over_floor(xchar, xchar, int, boolean *);
void fracture_rock(struct obj *);
boolean break_statue(struct obj *);
void destroy_item(int, int);
int destroy_mitem(struct monst *, int, int);
int resist(struct monst *, char, int, int);
void makewish(void);
/* KMH -- xchar to xchar */
void zap_strike_fx(xchar, xchar, int);
void throwspell(void);

#endif /* !LEV_LEX_C */

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(val, lower, upper) MIN((upper), MAX((val), (lower)))

#endif /* EXTERN_H */
