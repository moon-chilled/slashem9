/*	SCCS Id: @(#)extern.h	3.4	2003/03/10	*/
/* Copyright (c) Steve Creps, 1988.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

/* ### alloc.c ### */

#if 0
extern long *FDECL(alloc, (unsigned int));
#endif
extern char *FDECL(fmt_ptr, (const genericptr,char *));

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### allmain.c ### */

extern void moveloop(void);
extern void stop_occupation(void);
extern void display_gamewindows(void);
extern void newgame(void);
extern void FDECL(welcome, (BOOLEAN_P));
extern time_t get_realtime(void);

/* ### apply.c ### */

extern int doapply(void);
extern int dorub(void);
extern int dojump(void);
extern int FDECL(jump, (int));
extern int FDECL(jump, (int));
extern int number_leashed(void);
extern void FDECL(o_unleash, (struct obj *));
extern void FDECL(m_unleash, (struct monst *,BOOLEAN_P));
extern void unleash_all(void);
extern boolean next_to_u(void);
extern struct obj *FDECL(get_mleash, (struct monst *));
extern void FDECL(check_leash, (struct monst *,XCHAR_P,XCHAR_P,BOOLEAN_P));
extern boolean FDECL(um_dist, (XCHAR_P,XCHAR_P,XCHAR_P));
extern boolean FDECL(snuff_candle, (struct obj *));
extern boolean FDECL(snuff_lit, (struct obj *));
extern boolean FDECL(catch_lit, (struct obj *));
extern void FDECL(use_unicorn_horn, (struct obj *));
extern boolean FDECL(tinnable, (struct obj *));
extern void reset_trapset(void);
extern void FDECL(fig_transform, (genericptr_t, long));
extern int FDECL(unfixable_trouble_count,(BOOLEAN_P));
extern int FDECL(wand_explode, (struct obj *,BOOLEAN_P));

/* ### artifact.c ### */

extern void init_artifacts(void);
extern void init_artifacts1(void);
extern void FDECL(save_artifacts, (int));
extern void FDECL(restore_artifacts, (int));
extern const char *FDECL(artiname, (int));
extern struct obj *FDECL(mk_artifact, (struct obj *,ALIGNTYP_P));
extern const char *FDECL(artifact_name, (const char *,short *));
extern boolean FDECL(exist_artifact, (int,const char *));
extern void FDECL(artifact_exists, (struct obj *,const char *,BOOLEAN_P));
extern int nartifact_exist(void);
extern boolean FDECL(spec_ability, (struct obj *,unsigned long));
extern boolean FDECL(confers_luck, (struct obj *));
extern boolean FDECL(arti_reflects, (struct obj *));
extern boolean FDECL(restrict_name, (struct obj *,const char *));
extern boolean FDECL(defends, (int,struct obj *));
extern boolean FDECL(protects, (int,struct obj *));
extern void FDECL(set_artifact_intrinsic, (struct obj *,BOOLEAN_P,long));
extern int FDECL(touch_artifact, (struct obj *,struct monst *));
extern int FDECL(spec_abon, (struct obj *,struct monst *));
extern int FDECL(spec_dbon, (struct obj *,struct monst *,int));
extern void FDECL(discover_artifact, (XCHAR_P));
extern boolean FDECL(undiscovered_artifact, (XCHAR_P));
extern int FDECL(disp_artifact_discoveries, (winid));
extern boolean FDECL(artifact_hit, (struct monst *,struct monst *,
				struct obj *,int *,int));
extern int doinvoke(void);
extern void FDECL(arti_speak, (struct obj *));
extern boolean FDECL(artifact_light, (struct obj *));
extern int FDECL(artifact_wet, (struct obj *, BOOLEAN_P));
extern void FDECL(arti_speak, (struct obj *));
extern boolean FDECL(artifact_light, (struct obj *));
extern long FDECL(spec_m2, (struct obj *));
extern boolean FDECL(artifact_has_invprop, (struct obj *,UCHAR_P));
extern long FDECL(arti_cost, (struct obj *));

/* ### attrib.c ### */

extern boolean FDECL(adjattrib, (int,int,int));
extern void FDECL(change_luck, (SCHAR_P));
extern int FDECL(stone_luck, (BOOLEAN_P));
extern void set_moreluck(void);
extern void FDECL(gainstr, (struct obj *,int));
extern void FDECL(losestr, (int));
extern void restore_attrib(void);
extern void FDECL(exercise, (int,BOOLEAN_P));
extern void exerchk(void);
extern void reset_attribute_clock(void);
extern void FDECL(init_attr, (int));
extern void redist_attr(void);
extern void FDECL(adjabil, (int,int));
extern int newhp(void);
extern schar FDECL(acurr, (int));
extern schar acurrstr(void);
extern void FDECL(adjalign, (int));
/* KMH, balance patch -- new function */
extern void recalc_health(void);

/* ### ball.c ### */

extern void ballfall(void);
extern void placebc(void);
extern void unplacebc(void);
extern void FDECL(set_bc, (int));
extern void FDECL(move_bc, (int,int,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
extern boolean FDECL(drag_ball, (XCHAR_P,XCHAR_P,
		int *,xchar *,xchar *,xchar *,xchar *, boolean *,BOOLEAN_P));
extern void FDECL(drop_ball, (XCHAR_P,XCHAR_P));
extern void drag_down(void);

/* ### bones.c ### */

extern boolean can_make_bones(void);
extern void FDECL(savebones, (struct obj *));
extern int getbones(void);

/* ### borg.c ### */

/* extern char borg_on;
extern char borg_line[80];
extern char FDECL(borg_input, (void)); */

/* ### botl.c ### */

extern int FDECL(xlev_to_rank, (int));
extern int FDECL(title_to_mon, (const char *,int *,int *));
extern void max_rank_sz(void);
#ifdef SCORE_ON_BOTL
extern long botl_score(void);
#endif
extern int FDECL(describe_level, (char *, int));
extern const char *FDECL(rank_of, (int,SHORT_P,BOOLEAN_P));
extern void FDECL(bot_set_handler, (void (*)()));
extern void bot_reconfig(void);
extern void bot(void);
#ifdef DUMP_LOG
extern void FDECL(bot1str, (char *));
extern void FDECL(bot2str, (char *));
#endif

#if 0
extern const char * FDECL(shorten_bot1, (const char *, int));
#endif
#ifdef TTY_GRAPHICS
extern const char * FDECL(shorten_bot2, (const char *, unsigned int));
#endif

/* ### cmd.c ### */

#ifdef USE_TRAMPOLI
extern int doextcmd(void);
extern int domonability(void);
extern int domonability(void);
extern int polyatwill(void);
extern int playersteal(void);
extern int doprev_message(void);
extern int timed_occupation(void);
extern int wiz_attributes(void);
extern int enter_explore_mode(void);
# ifdef WIZARD
extern int wiz_detect(void);
extern int wiz_genesis(void);
extern int wiz_identify(void);
extern int wiz_level_tele(void);
extern int wiz_map(void);
extern int wiz_where(void);
extern int wiz_wish(void);
# endif /* WIZARD */
#endif /* USE_TRAMPOLI */
extern void reset_occupations(void);
extern void FDECL(set_occupation, (int (*)(void),const char *,int));
#ifdef REDO
extern char pgetchar(void);
extern void FDECL(pushch, (CHAR_P));
extern void FDECL(savech, (CHAR_P));
#endif
#ifdef WIZARD
extern void add_debug_extended_commands(void);
#endif /* WIZARD */
extern void FDECL(rhack, (char *));
extern int doextlist(void);
extern int extcmd_via_menu(void);
extern void FDECL(enlightenment, (int));
extern void FDECL(show_conduct, (int));
#ifdef DUMP_LOG
extern void FDECL(dump_enlightenment, (int));
extern void FDECL(dump_conduct, (int));
#endif
extern int FDECL(xytod, (SCHAR_P,SCHAR_P));
extern void FDECL(dtoxy, (coord *,int));
extern int FDECL(movecmd, (CHAR_P));
extern int FDECL(getdir, (const char *));
extern void confdir(void);
extern int FDECL(isok, (int,int));
extern int FDECL(get_adjacent_loc, (const char *, const char *, XCHAR_P, XCHAR_P, coord *));
extern const char *FDECL(click_to_cmd, (int,int,int));
extern char readchar(void);
#ifdef WIZARD
extern void sanity_check(void);
#endif
extern char FDECL(yn_function, (const char *, const char *, CHAR_P));

/* ### dbridge.c ### */

extern boolean FDECL(is_pool, (int,int));
extern boolean FDECL(is_lava, (int,int));
extern boolean FDECL(is_ice, (int,int));
extern int FDECL(is_drawbridge_wall, (int,int));
extern boolean FDECL(is_db_wall, (int,int));
extern boolean FDECL(find_drawbridge, (int *,int*));
extern boolean FDECL(create_drawbridge, (int,int,int,BOOLEAN_P));
extern void FDECL(open_drawbridge, (int,int));
extern void FDECL(close_drawbridge, (int,int));
extern void FDECL(destroy_drawbridge, (int,int));

/* ### decl.c ### */

extern void decl_init(void);

/* ### detect.c ### */

extern struct obj *FDECL(o_in, (struct obj*,CHAR_P));
extern struct obj *FDECL(o_material, (struct obj*,unsigned));
extern int FDECL(gold_detect, (struct obj *));
extern int FDECL(food_detect, (struct obj *));
extern int FDECL(object_detect, (struct obj *,int));
extern int FDECL(monster_detect, (struct obj *,int));
extern int FDECL(trap_detect, (struct obj *));
extern const char *FDECL(level_distance, (d_level *));
extern void FDECL(use_crystal_ball, (struct obj *));
extern void do_mapping(void);
extern void do_vicinity_map(void);
extern void FDECL(cvt_sdoor_to_door, (struct rm *));
#ifdef USE_TRAMPOLI
extern void FDECL(findone, (int,int,genericptr_t));
extern void FDECL(openone, (int,int,genericptr_t));
#endif
extern int findit(void);
extern int openit(void);
extern void FDECL(find_trap, (struct trap *));
extern int FDECL(dosearch0, (int));
extern int dosearch(void);
extern void sokoban_detect(void);
/* KMH -- Sokoban levels */
extern void sokoban_detect(void);

/* ### dig.c ### */

extern boolean is_digging(void);
#ifdef USE_TRAMPOLI
extern int dig(void);
#endif
extern int holetime(void);
extern boolean FDECL(dig_check, (struct monst *, BOOLEAN_P, int, int));
extern void FDECL(digactualhole, (int,int,struct monst *,int));
extern boolean FDECL(dighole, (BOOLEAN_P));
extern int FDECL(use_pick_axe, (struct obj *));
extern int FDECL(use_pick_axe2, (struct obj *));
extern boolean FDECL(mdig_tunnel, (struct monst *));
extern void FDECL(watch_dig, (struct monst *,XCHAR_P,XCHAR_P,BOOLEAN_P));
extern void zap_dig(void);
extern struct obj *FDECL(bury_an_obj, (struct obj *));
extern void FDECL(bury_objs, (int,int));
extern void FDECL(unearth_objs, (int,int));
extern void FDECL(rot_organic, (genericptr_t, long));
extern void FDECL(rot_corpse, (genericptr_t, long));
#if 0
extern void FDECL(bury_monst, (struct monst *));
extern void bury_you(void);
extern void unearth_you(void);
extern void escape_tomb(void);
extern void FDECL(bury_obj, (struct obj *));
#endif

/* ### display.c ### */

#ifdef INVISIBLE_OBJECTS
extern struct obj * FDECL(vobj_at, (XCHAR_P,XCHAR_P));
#endif /* INVISIBLE_OBJECTS */
extern void FDECL(magic_map_background, (XCHAR_P,XCHAR_P,int));
extern void FDECL(map_background, (XCHAR_P,XCHAR_P,int));
extern void FDECL(map_trap, (struct trap *,int));
extern void FDECL(map_object, (struct obj *,int));
extern void FDECL(map_invisible, (XCHAR_P,XCHAR_P));
extern void FDECL(unmap_object, (int,int));
extern void FDECL(map_location, (int,int,int));
extern int FDECL(memory_glyph, (int, int));
extern void FDECL(clear_memory_glyph, (int, int, int));
extern void FDECL(feel_location, (XCHAR_P,XCHAR_P));
extern void FDECL(newsym, (int,int));
extern void FDECL(shieldeff, (XCHAR_P,XCHAR_P));
extern void FDECL(tmp_at, (int,int));
#ifdef DISPLAY_LAYERS
extern int FDECL(glyph_is_floating, (int));
#endif
extern void FDECL(swallowed, (int));
extern void FDECL(under_ground, (int));
extern void FDECL(under_water, (int));
extern void see_monsters(void);
extern void set_mimic_blocking(void);
extern void see_objects(void);
extern void see_traps(void);
extern void curs_on_u(void);
extern int doredraw(void);
extern void docrt(void);
extern void FDECL(show_glyph, (int,int,int));
extern void clear_glyph_buffer(void);
extern void FDECL(row_refresh, (int,int,int));
extern void cls(void);
extern void FDECL(flush_screen, (int));
#ifdef DUMP_LOG
extern void dump_screen(void);
#endif
extern int FDECL(back_to_glyph, (XCHAR_P,XCHAR_P));
extern int FDECL(zapdir_to_glyph, (int,int,int));
extern int FDECL(glyph_at, (XCHAR_P,XCHAR_P));
extern void set_wall_state(void);

/* ### do.c ### */

#ifdef USE_TRAMPOLI
extern int FDECL(drop, (struct obj *));
extern int wipeoff(void);
#endif
extern int dodrop(void);
extern boolean FDECL(boulder_hits_pool, (struct obj *,int,int,BOOLEAN_P));
extern boolean FDECL(flooreffects, (struct obj *,int,int,const char *));
extern void FDECL(doaltarobj, (struct obj *));
extern boolean FDECL(canletgo, (struct obj *,const char *));
extern void FDECL(dropx, (struct obj *));
extern void FDECL(dropy, (struct obj *));
extern void FDECL(obj_no_longer_held, (struct obj *));
extern int doddrop(void);
extern int dodown(void);
extern int doup(void);
#ifdef INSURANCE
extern void save_currentstate(void);
#endif
extern void FDECL(goto_level, (d_level *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(schedule_goto, (d_level *,BOOLEAN_P,BOOLEAN_P,int,
			     const char *,const char *));
extern void deferred_goto(void);
extern boolean FDECL(revive_corpse, (struct obj *, BOOLEAN_P));
extern void FDECL(revive_mon, (genericptr_t, long));
extern void FDECL(moldy_corpse, (genericptr_t, long));
extern int donull(void);
extern int dowipe(void);
extern void FDECL(set_wounded_legs, (long,int));
extern void heal_legs(void);

/* ### do_name.c ### */

extern int FDECL(getpos, (coord *,BOOLEAN_P,const char *));
extern struct monst *FDECL(christen_monst, (struct monst *,const char *));
extern int do_mname(void);
extern struct obj *FDECL(oname, (struct obj *,const char *));
extern int ddocall(void);
extern void FDECL(docall, (struct obj *));
extern const char *rndghostname(void);
extern char *FDECL(x_monnam, (struct monst *,int,const char *,int,BOOLEAN_P));
extern char *FDECL(l_monnam, (struct monst *));
extern char *FDECL(mon_nam, (struct monst *));
extern char *FDECL(noit_mon_nam, (struct monst *));
extern char *FDECL(Monnam, (struct monst *));
extern char *FDECL(noit_Monnam, (struct monst *));
extern char *FDECL(m_monnam, (struct monst *));
extern char *FDECL(y_monnam, (struct monst *));
extern char *FDECL(Adjmonnam, (struct monst *,const char *));
extern char *FDECL(Amonnam, (struct monst *));
extern char *FDECL(a_monnam, (struct monst *));
extern char *FDECL(distant_monnam, (struct monst *,int,char *));
extern const char *rndmonnam(void);
extern const char *FDECL(hcolor, (const char *));
extern const char *rndcolor(void);
#ifdef REINCARNATION
extern const char *roguename(void);
#endif
extern struct obj *FDECL(realloc_obj,
		(struct obj *, int, genericptr_t, int, const char *));
extern char *FDECL(coyotename, (struct monst *,char *));


/* ### do_wear.c ### */

extern int Armor_on(void);
extern int Boots_on(void);
extern int Cloak_on(void);
extern int Helmet_on(void);
extern int Gloves_on(void);
extern int Shield_on(void);
#ifdef TOURIST
extern int Shirt_on(void);
#endif
extern void Amulet_on(void);
#ifdef USE_TRAMPOLI
extern int FDECL(select_off, (struct obj *));
extern int take_off(void);
#endif
extern void FDECL(off_msg, (struct obj *));
extern void set_wear(void);
extern boolean FDECL(donning, (struct obj *));
extern void cancel_don(void);
extern int Armor_off(void);
extern int Armor_gone(void);
extern int Helmet_off(void);
extern int Gloves_off(void);
extern int Boots_off(void);
extern int Cloak_off(void);
extern int Shield_off(void);
#ifdef TOURIST
extern int Shirt_off(void);
#endif
extern void Amulet_off(void);
extern void FDECL(Ring_on, (struct obj *));
extern void FDECL(Ring_off, (struct obj *));
extern void FDECL(Ring_gone, (struct obj *));
extern void FDECL(Blindf_on, (struct obj *));
extern void FDECL(Blindf_off, (struct obj *));
extern int dotakeoff(void);
extern int doremring(void);
extern int FDECL(cursed, (struct obj *));
extern int FDECL(armoroff, (struct obj *));
extern int FDECL(canwearobj, (struct obj *, long *, BOOLEAN_P));
extern int dowear(void);
extern int doputon(void);
extern void find_ac(void);
extern void glibr(void);
extern struct obj *FDECL(some_armor,(struct monst *));
extern void FDECL(erode_armor, (struct monst *,BOOLEAN_P));
extern struct obj *FDECL(stuck_ring, (struct obj *,int));
extern struct obj *unchanger(void);
extern void reset_remarm(void);
extern int doddoremarm(void);
extern int FDECL(destroy_arm, (struct obj *));
extern void FDECL(adj_abon, (struct obj *,SCHAR_P));
extern int FDECL(dowear2, (const char *, const char *));

/* ### dog.c ### */

extern void FDECL(initedog, (struct monst *));
extern struct monst *FDECL(make_familiar, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));
extern struct monst *FDECL(make_helper, (int,XCHAR_P,XCHAR_P));
extern struct monst *makedog(void);
extern void update_mlstmv(void);
extern void losedogs(void);
extern void FDECL(mon_arrive, (struct monst *,BOOLEAN_P));
extern void FDECL(mon_catchup_elapsed_time, (struct monst *,long));
extern void FDECL(keepdogs, (BOOLEAN_P));
extern void FDECL(migrate_to_level, (struct monst *,XCHAR_P,XCHAR_P,coord *));
extern int FDECL(dogfood, (struct monst *,struct obj *));
extern struct monst *FDECL(tamedog, (struct monst *,struct obj *));
extern int FDECL(make_pet_minion, (int,ALIGNTYP_P));
extern void FDECL(abuse_dog, (struct monst *));
extern void FDECL(wary_dog, (struct monst *, BOOLEAN_P));

/* ### dogmove.c ### */

extern int FDECL(dog_nutrition, (struct monst *,struct obj *));
extern int FDECL(dog_eat, (struct monst *,struct obj *,int,int,BOOLEAN_P));
extern int FDECL(dog_move, (struct monst *,int));
extern boolean FDECL(betrayed, (struct monst *));
#ifdef USE_TRAMPOLI
extern void FDECL(wantdoor, (int,int,genericptr_t));
#endif

/* ### dokick.c ### */

extern boolean FDECL(ghitm, (struct monst *,struct obj *));
extern void FDECL(container_impact_dmg, (struct obj *));
extern int dokick(void);
extern boolean FDECL(ship_object, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));
extern void obj_delivery(void);
extern schar FDECL(down_gate, (XCHAR_P,XCHAR_P));
extern void FDECL(impact_drop, (struct obj *,XCHAR_P,XCHAR_P,XCHAR_P));

/* ### dothrow.c ### */

extern struct obj *FDECL(splitoneoff, (struct obj **));
extern int dothrow(void);
extern int dofire(void);
extern void FDECL(hitfloor, (struct obj *));
extern void FDECL(hurtle, (int,int,int,BOOLEAN_P));
extern void FDECL(mhurtle, (struct monst *,int,int,int));
extern void FDECL(throwit, (struct obj *,long,BOOLEAN_P,int));
extern int FDECL(omon_adj, (struct monst *,struct obj *,BOOLEAN_P));
extern int FDECL(thitmonst, (struct monst *,struct obj *, int));
extern int FDECL(hero_breaks, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));
extern int FDECL(breaks, (struct obj *,XCHAR_P,XCHAR_P));
extern boolean FDECL(breaktest, (struct obj *));
extern boolean FDECL(walk_path, (coord *, coord *, boolean (*)(genericptr_t,int,int), genericptr_t));
extern boolean FDECL(hurtle_step, (genericptr_t, int, int));

/* ### drawing.c ### */
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern int FDECL(def_char_to_objclass, (CHAR_P));
extern int FDECL(def_char_to_monclass, (CHAR_P));
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void FDECL(assign_graphics, (uchar *,int,int,int));
extern void FDECL(switch_graphics, (int));
#ifdef REINCARNATION
extern void FDECL(assign_rogue_graphics, (BOOLEAN_P));
#endif
#ifdef USER_DUNGEONCOLOR
extern void FDECL(assign_colors, (uchar *,int,int,int));
#endif

/* ### dungeon.c ### */

extern void FDECL(save_dungeon, (int,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(restore_dungeon, (int));
extern void FDECL(insert_branch, (branch *,BOOLEAN_P));
extern void init_dungeons(void);
extern s_level *FDECL(find_level, (const char *));
extern s_level *FDECL(Is_special, (d_level *));
extern branch *FDECL(Is_branchlev, (d_level *));
extern xchar FDECL(ledger_no, (d_level *));
extern xchar maxledgerno(void);
extern schar FDECL(depth, (d_level *));
extern xchar FDECL(dunlev, (d_level *));
extern xchar FDECL(dunlevs_in_dungeon, (d_level *));
extern xchar FDECL(real_dunlevs_in_dungeon, (d_level *));
extern xchar FDECL(ledger_to_dnum, (XCHAR_P));
extern xchar FDECL(ledger_to_dlev, (XCHAR_P));
extern xchar FDECL(deepest_lev_reached, (BOOLEAN_P));
extern boolean FDECL(on_level, (d_level *,d_level *));
extern void FDECL(next_level, (BOOLEAN_P));
extern void FDECL(prev_level, (BOOLEAN_P));
extern void FDECL(u_on_newpos, (int,int));
extern void u_on_sstairs(void);
extern void u_on_upstairs(void);
extern void u_on_dnstairs(void);
extern boolean FDECL(On_stairs, (XCHAR_P,XCHAR_P));
extern void FDECL(get_level, (d_level *,int));
extern boolean FDECL(Is_botlevel, (d_level *));
extern boolean FDECL(Can_fall_thru, (d_level *));
extern boolean FDECL(Can_dig_down, (d_level *));
extern boolean FDECL(Can_rise_up, (int,int,d_level *));
extern boolean FDECL(In_quest, (d_level *));
extern boolean FDECL(In_mines, (d_level *));
extern boolean FDECL(In_spiders, (d_level *));
extern branch *FDECL(dungeon_branch, (const char *));
extern boolean FDECL(at_dgn_entrance, (const char *));
extern boolean FDECL(In_hell, (d_level *));
extern boolean FDECL(In_V_tower, (d_level *));
extern boolean FDECL(On_W_tower_level, (d_level *));
extern boolean FDECL(In_W_tower, (int,int,d_level *));
extern void FDECL(find_hell, (d_level *));
extern void FDECL(goto_hell, (BOOLEAN_P,BOOLEAN_P));
extern void FDECL(assign_level, (d_level *,d_level *));
extern void FDECL(assign_rnd_level, (d_level *,d_level *,int));
extern int FDECL(induced_align, (int));
extern boolean FDECL(Invocation_lev, (d_level *));
extern xchar level_difficulty(void);
extern schar FDECL(lev_by_name, (const char *));
#ifdef WIZARD
extern schar FDECL(print_dungeon, (BOOLEAN_P,schar *,xchar *));
#endif

/* ### eat.c ### */

#ifdef USE_TRAMPOLI
extern int eatmdone(void);
extern int eatfood(void);
extern int opentin(void);
extern int unfaint(void);
#endif
extern boolean FDECL(is_edible, (struct obj *));
extern void init_uhunger(void);
extern int Hear_again(void);
extern void reset_eat(void);
extern int doeat(void);
extern void gethungry(void);
extern void FDECL(morehungry, (int));
extern void FDECL(lesshungry, (int));
extern boolean is_fainted(void);
extern void reset_faint(void);
extern void violated_vegetarian(void);
#if 0
extern void sync_hunger(void);
#endif
extern void FDECL(newuhs, (BOOLEAN_P));
extern boolean can_reach_floorobj(void);
extern void vomit(void);
extern int FDECL(eaten_stat, (int,struct obj *));
extern void FDECL(food_disappears, (struct obj *));
extern void FDECL(food_substitution, (struct obj *,struct obj *));
extern boolean FDECL(bite_monster, (struct monst *mon));
extern void fix_petrification(void);
extern void FDECL(consume_oeaten, (struct obj *,int));
extern boolean FDECL(maybe_finished_meal, (BOOLEAN_P));

/* ### end.c ### */

extern void FDECL(done1, (int));
extern int done2(void);
#ifdef USE_TRAMPOLI
extern void FDECL(done_intr, (int));
#endif
extern void FDECL(done_in_by, (struct monst *));
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern void VDECL(panic, (const char *,...)) PRINTF_F(1,2);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void FDECL(done, (int));
extern void FDECL(container_contents, (struct obj *,BOOLEAN_P,BOOLEAN_P));
#ifdef DUMP_LOG
extern void FDECL(dump, (char *, char *));
extern void FDECL(do_containerconts, (struct obj *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
#endif
extern void FDECL(terminate, (int));
extern int dolistvanq(void);
extern int num_genocides(void);
/* KMH, ethics */
extern int doethics(void);


/* ### engrave.c ### */

extern char *FDECL(random_engraving, (char *));
extern void FDECL(wipeout_text, (char *,int,unsigned));
extern boolean can_reach_floor(void);
extern const char *FDECL(surface, (int,int));
extern const char *FDECL(ceiling, (int,int));
extern struct engr *FDECL(engr_at, (XCHAR_P,XCHAR_P));
#ifdef ELBERETH
extern int FDECL(sengr_at, (const char *,XCHAR_P,XCHAR_P));
#endif
extern void FDECL(u_wipe_engr, (int));
extern void FDECL(wipe_engr_at, (XCHAR_P,XCHAR_P,XCHAR_P));
extern boolean FDECL(sense_engr_at, (int,int,BOOLEAN_P));
extern void FDECL(make_engr_at, (int,int,const char *,long,XCHAR_P));
extern void FDECL(del_engr_at, (int,int));
extern int freehand(void);
extern int doengrave(void);
extern void FDECL(save_engravings, (int,int));
extern void FDECL(rest_engravings, (int));
extern void FDECL(del_engr, (struct engr *));
extern void FDECL(rloc_engr, (struct engr *));
extern void FDECL(make_grave, (int,int,const char *));

/* ### exper.c ### */

extern long FDECL(newuexp, (int));
extern int FDECL(experience, (struct monst *,int));
extern void FDECL(more_experienced, (int,int));
extern void FDECL(losexp, (const char *, BOOLEAN_P));
extern void newexplevel(void);
extern void FDECL(pluslvl, (BOOLEAN_P));
extern long FDECL(rndexp, (BOOLEAN_P));

/* ### explode.c ### */

extern void FDECL(explode, (int,int,int,int,CHAR_P,int));
extern long FDECL(scatter, (int, int, int, unsigned int, struct obj *));
extern void FDECL(splatter_burning_oil, (int, int));
#ifdef FIREARMS
extern void FDECL(grenade_explode, (struct obj *, int, int, BOOLEAN_P, int));
extern void FDECL(arm_bomb, (struct obj *, BOOLEAN_P));
#endif

/* ### extralev.c ### */

#ifdef REINCARNATION
extern void makeroguerooms(void);
extern void FDECL(corr, (int,int));
extern void makerogueghost(void);
#endif

/* ### files.c ### */

extern char *FDECL(fname_encode, (const char *, CHAR_P, char *, char *, int));
extern char *FDECL(fname_decode, (CHAR_P, char *, char *, int));
extern const char *FDECL(fqname, (const char *, int, int));
#ifndef FILE_AREAS
extern FILE *FDECL(fopen_datafile, (const char *,const char *,int));
#endif
extern boolean FDECL(uptodate, (int,const char *));
extern void FDECL(store_version, (int));
#ifdef MFLOPPY
extern void set_lock_and_bones(void);
#endif
extern void FDECL(set_levelfile_name, (char *,int));
extern int FDECL(create_levelfile, (int,char *));
extern int FDECL(open_levelfile, (int,char *));
extern void FDECL(delete_levelfile, (int));
extern void clearlocks(void);
extern int FDECL(create_bonesfile, (d_level*,char **, char *));
#ifdef MFLOPPY
extern void cancel_bonesfile(void);
#endif
extern void FDECL(commit_bonesfile, (d_level *));
extern int FDECL(open_bonesfile, (d_level*,char **));
extern int FDECL(delete_bonesfile, (d_level*));
extern void compress_bonesfile(void);
extern void set_savefile_name(void);
#ifdef INSURANCE
extern void FDECL(save_savefile_name, (int));
#endif
#if defined(WIZARD) && !defined(MICRO)
extern void set_error_savefile(void);
#endif
extern int create_savefile(void);
extern int open_savefile(void);
extern int delete_savefile(void);
extern int restore_saved_game(void);
extern void FDECL(compress_area, (const char *, const char *));
extern void FDECL(uncompress_area, (const char *, const char *));
#ifndef FILE_AREAS
extern boolean FDECL(lock_file, (const char *,int,int));
extern void FDECL(unlock_file, (const char *));
#endif
#ifdef USER_SOUNDS
extern boolean FDECL(can_read_file, (const char *));
#endif
extern void FDECL(read_config_file, (const char *));
extern void FDECL(check_recordfile, (const char *));
#if defined(WIZARD)
extern void read_wizkit(void);
#endif
extern void FDECL(paniclog, (const char *, const char *));
extern int FDECL(validate_prefix_locations, (char *));
extern char** get_saved_games(void);
extern void FDECL(free_saved_games, (char**));
#ifdef SELF_RECOVER
extern boolean recover_savefile(void);
#endif
#ifdef HOLD_LOCKFILE_OPEN
extern void really_close(void);
#endif

/* ### fountain.c ### */

extern void FDECL(floating_above, (const char *));
extern void FDECL(dogushforth, (int));
# ifdef USE_TRAMPOLI
extern void FDECL(gush, (int,int,genericptr_t));
# endif
extern void FDECL(dryup, (XCHAR_P,XCHAR_P, BOOLEAN_P));
extern void drinkfountain(void);
extern void FDECL(dipfountain, (struct obj *));
extern void FDECL(whetstone_fountain_effects, (struct obj *));
extern void FDECL(diptoilet, (struct obj *));
extern void FDECL(breaksink, (int,int));
extern void FDECL(breaktoilet, (int,int));
extern void drinksink(void);
extern void drinktoilet(void);
extern void FDECL(whetstone_sink_effects, (struct obj *));
extern void FDECL(whetstone_toilet_effects, (struct obj *));

/* ### gypsy.c ### */

extern void FDECL(gypsy_init, (struct monst *));
extern void FDECL(gypsy_chat, (struct monst *));


/* ### hack.c ### */

#ifdef DUNGEON_GROWTH
extern void FDECL(catchup_dgn_growths, (int));
extern void FDECL(dgn_growths, (BOOLEAN_P,BOOLEAN_P));
#endif
extern boolean FDECL(revive_nasty, (int,int,const char*));
extern void FDECL(movobj, (struct obj *,XCHAR_P,XCHAR_P));
extern boolean FDECL(may_dig, (XCHAR_P,XCHAR_P));
extern boolean FDECL(may_passwall, (XCHAR_P,XCHAR_P));
extern boolean FDECL(bad_rock, (struct monst *,XCHAR_P,XCHAR_P));
extern boolean FDECL(invocation_pos, (XCHAR_P,XCHAR_P));
extern boolean FDECL(test_move, (int, int, int, int, int));
extern void domove(void);
extern void invocation_message(void);
extern void FDECL(spoteffects, (BOOLEAN_P));
extern char *FDECL(in_rooms, (XCHAR_P,XCHAR_P,int));
extern boolean FDECL(in_town, (int,int));
extern void FDECL(check_special_room, (BOOLEAN_P));
extern int dopickup(void);
extern void lookaround(void);
extern int monster_nearby(void);
extern void FDECL(nomul, (int));
extern void FDECL(unmul, (const char *));
#ifdef SHOW_DMG
extern void FDECL(showdmg, (int));
#endif
extern void FDECL(losehp, (int,const char *, int));
extern int weight_cap(void);
extern int inv_weight(void);
extern int near_capacity(void);
extern int FDECL(calc_capacity, (int));
extern int max_capacity(void);
extern boolean FDECL(check_capacity, (const char *));
extern int inv_cnt(void);
#ifdef GOLDOBJ
extern long FDECL(money_cnt, (struct obj *));
#endif

/* ### hacklib.c ### */

extern boolean FDECL(digit, (CHAR_P));
extern boolean FDECL(letter, (CHAR_P));
extern char FDECL(highc, (CHAR_P));
extern char FDECL(lowc, (CHAR_P));
extern char *FDECL(lcase, (char *));
extern char *FDECL(upstart, (char *));
extern char *FDECL(mungspaces, (char *));
extern char *FDECL(eos, (char *));
extern char *FDECL(strkitten, (char *,CHAR_P));
extern char *FDECL(s_suffix, (const char *));
extern char *FDECL(xcrypt, (const char *,char *));
extern boolean FDECL(onlyspace, (const char *));
extern char *FDECL(tabexpand, (char *));
extern char *FDECL(visctrl, (CHAR_P));
extern const char *FDECL(ordin, (int));
extern char *FDECL(sitoa, (int));
extern int FDECL(sgn, (int));
extern int FDECL(rounddiv, (long,int));
extern int FDECL(dist2, (int,int,int,int));
extern int FDECL(distmin, (int,int,int,int));
extern boolean FDECL(online2, (int,int,int,int));
extern boolean FDECL(pmatch, (const char *,const char *));
#ifndef STRNCMPI
extern int FDECL(strncmpi, (const char *,const char *,int));
#endif
#ifndef STRSTRI
extern char *FDECL(strstri, (const char *,const char *));
#endif
extern boolean FDECL(fuzzymatch, (const char *,const char *,const char *,BOOLEAN_P));
extern void setrandom(void);
extern int getyear(void);
extern int getmonth(void);	/* KMH -- Used by gypsies */
#if 0
extern char *FDECL(yymmdd, (time_t));
#endif
extern long FDECL(yyyymmdd, (time_t));
extern int phase_of_the_moon(void);
extern boolean friday_13th(void);
extern boolean groundhog_day(void);	/* KMH -- February 2 */
extern int night(void);
extern int midnight(void);

/* ### invent.c ### */

extern void FDECL(assigninvlet, (struct obj *));
extern struct obj *FDECL(merge_choice, (struct obj *,struct obj *));
extern int FDECL(merged, (struct obj **,struct obj **));
#ifdef USE_TRAMPOLI
extern int FDECL(ckunpaid, (struct obj *));
#endif
extern void FDECL(addinv_core1, (struct obj *));
extern void FDECL(addinv_core2, (struct obj *));
extern struct obj *FDECL(addinv, (struct obj *));
extern struct obj *FDECL(hold_another_object,
			(struct obj *,const char *,const char *,const char *));
extern void FDECL(useupall, (struct obj *));
extern void FDECL(useup, (struct obj *));
extern void FDECL(consume_obj_charge, (struct obj *,BOOLEAN_P));
extern void FDECL(freeinv_core, (struct obj *));
extern void FDECL(freeinv, (struct obj *));
extern void FDECL(delallobj, (int,int));
extern void FDECL(delobj, (struct obj *));
extern struct obj *FDECL(sobj_at, (int,int,int));
extern struct obj *FDECL(carrying, (int));
extern boolean have_lizard(void);
extern struct obj *FDECL(o_on, (unsigned int,struct obj *));
extern boolean FDECL(obj_here, (struct obj *,int,int));
extern boolean wearing_armor(void);
extern boolean FDECL(is_worn, (struct obj *));
extern struct obj *FDECL(g_at, (int,int));
extern struct obj *FDECL(mkgoldobj, (long));
extern struct obj *FDECL(getobj, (const char *,const char *));
extern int FDECL(ggetobj, (const char *,int (*)(OBJ_P),int,BOOLEAN_P,unsigned *));
extern void FDECL(fully_identify_obj, (struct obj *));
extern int FDECL(identify, (struct obj *));
extern void FDECL(identify_pack, (int));
extern int FDECL(askchain, (struct obj **,const char *,int,int (*)(OBJ_P),
			int (*)(OBJ_P),int,const char *));
extern void FDECL(prinv, (const char *,struct obj *,long));
extern char *FDECL(xprname, (struct obj *,const char *,CHAR_P,BOOLEAN_P,long,long));
extern int ddoinv(void);
extern char FDECL(display_inventory, (const char *,BOOLEAN_P));
#ifdef DUMP_LOG
extern char FDECL(dump_inventory, (const char *,BOOLEAN_P));
#endif
extern int FDECL(display_binventory, (int,int,BOOLEAN_P));
extern struct obj *FDECL(display_cinventory,(struct obj *));
extern struct obj *FDECL(display_minventory,(struct monst *,int,char *));
extern int dotypeinv(void);
extern const char *FDECL(dfeature_at, (int,int,char *));
extern int FDECL(look_here, (int,BOOLEAN_P));
extern int dolook(void);
extern boolean FDECL(will_feel_cockatrice, (struct obj *,BOOLEAN_P));
extern void FDECL(feel_cockatrice, (struct obj *,BOOLEAN_P));
extern void FDECL(stackobj, (struct obj *));
extern int doprgold(void);
extern int doprwep(void);
extern int doprarm(void);
extern int doprring(void);
extern int dopramulet(void);
extern int doprtool(void);
extern int doprinuse(void);
extern void FDECL(useupf, (struct obj *,long));
extern char *FDECL(let_to_name, (CHAR_P,BOOLEAN_P));
extern void free_invbuf(void);
extern void reassign(void);
extern int doorganize(void);
extern int FDECL(count_unpaid, (struct obj *));
extern int FDECL(count_buc, (struct obj *,int));
extern void FDECL(carry_obj_effects, (struct monst *, struct obj *));
extern const char *FDECL(currency, (long));
extern void FDECL(silly_thing, (const char *,struct obj *));
extern int doinvinuse(void);
/* KMH, balance patch -- new function */
extern int jumble_pack(void);

/* ### ioctl.c ### */

#if defined(UNIX) || defined(__BEOS__)
extern void getwindowsz(void);
extern void getioctls(void);
extern void setioctls(void);
# ifdef SUSPEND
extern int dosuspend(void);
# endif /* SUSPEND */
#endif /* UNIX || __BEOS__ */

/* ### light.c ### */

extern void FDECL(new_light_source, (XCHAR_P, XCHAR_P, int, int, genericptr_t));
extern void FDECL(del_light_source, (int, genericptr_t));
extern void FDECL(do_light_sources, (char **));
extern struct monst *FDECL(find_mid, (unsigned, unsigned));
extern void FDECL(save_light_sources, (int, int, int));
extern void FDECL(restore_light_sources, (int));
extern void FDECL(relink_light_sources, (BOOLEAN_P));
extern void FDECL(obj_move_light_source, (struct obj *, struct obj *));
extern boolean any_light_source(void);
extern void FDECL(snuff_light_source, (int, int));
extern boolean FDECL(obj_sheds_light, (struct obj *));
extern boolean FDECL(obj_is_burning, (struct obj *));
extern boolean FDECL(obj_permanent_light, (struct obj *));
extern void FDECL(obj_split_light_source, (struct obj *, struct obj *));
extern void FDECL(obj_merge_light_sources, (struct obj *,struct obj *));
extern int FDECL(candle_light_range, (struct obj *));
#ifdef WIZARD
extern int wiz_light_sources(void);
#endif

/* ### lock.c ### */

#ifdef USE_TRAMPOLI
extern int forcelock(void);
extern int picklock(void);
#endif
extern boolean FDECL(picking_lock, (int *,int *));
extern boolean FDECL(picking_at, (int,int));
extern void reset_pick(void);
extern int FDECL(pick_lock, (struct obj **));
extern int doforce(void);
extern boolean FDECL(boxlock, (struct obj *,struct obj *));
extern boolean FDECL(doorlock, (struct obj *,int,int));
extern int doopen(void);
extern int doclose(void);
extern int FDECL(artifact_door, (int,int));

#ifdef MAC
/* These declarations are here because the main code calls them. */

/* ### macfile.c ### */

extern int FDECL(maccreat, (const char *,long));
extern int FDECL(macopen, (const char *,int,long));
extern int FDECL(macclose, (int));
extern int FDECL(macread, (int,void *,unsigned));
extern int FDECL(macwrite, (int,void *,unsigned));
extern long FDECL(macseek, (int,long,short));
extern int FDECL(macunlink, (const char *));

/* ### macsnd.c ### */

extern void FDECL(mac_speaker, (struct obj *,char *));

/* ### macunix.c ### */

extern void FDECL(regularize, (char *));
extern void getlock(void);

/* ### macwin.c ### */

extern void FDECL(lock_mouse_cursor, (Boolean));
extern int SanePositions(void);

/* ### mttymain.c ### */

extern void FDECL(getreturn, (const char *));
extern void VDECL(msmsg, (const char *,...));
extern void gettty(void);
extern void setftty(void);
extern void FDECL(settty, (const char *));
extern int tgetch(void);
extern void FDECL(cmov, (int x, int y));
extern void FDECL(nocmov, (int x, int y));

#endif /* MAC */

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
extern void getmailstatus(void);
# endif
extern void ckmailstatus(void);
extern void FDECL(readmail, (struct obj *));
#endif /* MAIL */

/* ### makemon.c ### */

extern boolean FDECL(is_home_elemental, (struct permonst *));
extern struct monst *FDECL(clone_mon, (struct monst *,XCHAR_P,XCHAR_P));
extern struct monst *FDECL(makemon, (struct permonst *,int,int,int));
extern boolean FDECL(create_critters, (int,struct permonst *));
extern struct permonst *rndmonst(void);
extern void FDECL(reset_rndmonst, (int));
extern struct permonst *FDECL(mkclass, (CHAR_P,int));
extern int FDECL(pm_mkclass, (CHAR_P,int));
extern int FDECL(adj_lev, (struct permonst *));
extern struct permonst *FDECL(grow_up, (struct monst *,struct monst *));
extern int FDECL(mongets, (struct monst *,int));
extern int FDECL(golemhp, (int));
extern boolean FDECL(peace_minded, (struct permonst *));
extern void FDECL(set_malign, (struct monst *));
extern void FDECL(set_mimic_sym, (struct monst *));
extern int FDECL(mbirth_limit, (int));
extern void FDECL(mimic_hit_msg, (struct monst *, SHORT_P));
#ifdef GOLDOBJ
extern void FDECL(mkmonmoney, (struct monst *, long));
#endif
extern void FDECL(bagotricks, (struct obj *));
extern boolean FDECL(propagate, (int, BOOLEAN_P,BOOLEAN_P));

/* ### mapglyph.c ### */

extern void FDECL(mapglyph, (int, int *, int *, unsigned *, int, int));

/* ### mcastu.c ### */

extern int FDECL(castmu, (struct monst *,struct attack *,BOOLEAN_P,BOOLEAN_P));
extern int FDECL(buzzmu, (struct monst *,struct attack *));

/* ### mhitm.c ### */

extern int FDECL(fightm, (struct monst *));
extern int FDECL(mattackm, (struct monst *,struct monst *));
extern int FDECL(noattacks, (struct permonst *));
extern int FDECL(sleep_monst, (struct monst *,int,int));
extern void FDECL(slept_monst, (struct monst *));
extern long FDECL(attk_protection, (int));

/* ### mhitu.c ### */

extern const char *FDECL(mpoisons_subj, (struct monst *,struct attack *));
extern void u_slow_down(void);
extern struct monst *cloneu(void);
extern void FDECL(expels, (struct monst *,struct permonst *,BOOLEAN_P));
extern struct attack *FDECL(getmattk, (struct permonst *,int,int *,struct attack *));
extern int FDECL(mattacku, (struct monst *));
extern int FDECL(magic_negation, (struct monst *));
extern int FDECL(gazemu, (struct monst *,struct attack *));
extern void FDECL(mdamageu, (struct monst *,int));
extern int FDECL(could_seduce, (struct monst *,struct monst *,struct attack *));
#ifdef SEDUCE
extern int FDECL(doseduce, (struct monst *));
#endif

/* ### minion.c ### */

extern void FDECL(msummon, (struct monst *));
extern void FDECL(summon_minion, (ALIGNTYP_P,BOOLEAN_P));
extern int FDECL(demon_talk, (struct monst *));
extern int FDECL(lawful_minion, (int));
extern int FDECL(neutral_minion, (int));
extern int FDECL(chaotic_minion, (int));
extern long FDECL(bribe, (struct monst *));
extern int FDECL(dprince, (ALIGNTYP_P));
extern int FDECL(dlord, (ALIGNTYP_P));
extern int llord(void);
extern int FDECL(ndemon, (ALIGNTYP_P));
extern int lminion(void);

/* ### mklev.c ### */

#ifdef USE_TRAMPOLI
extern int FDECL(do_comp, (genericptr_t,genericptr_t));
#endif
extern void sort_rooms(void);
extern void FDECL(add_room, (int,int,int,int,BOOLEAN_P,SCHAR_P,BOOLEAN_P));
extern void FDECL(add_subroom, (struct mkroom *,int,int,int,int,
			   BOOLEAN_P,SCHAR_P,BOOLEAN_P));
extern void makecorridors(void);
extern int FDECL(add_door, (int,int,struct mkroom *));
extern void mklev(void);
#ifdef SPECIALIZATION
extern void FDECL(topologize, (struct mkroom *,BOOLEAN_P));
#else
extern void FDECL(topologize, (struct mkroom *));
#endif
extern void FDECL(place_branch, (branch *,XCHAR_P,XCHAR_P));
extern boolean FDECL(occupied, (XCHAR_P,XCHAR_P));
extern int FDECL(okdoor, (XCHAR_P,XCHAR_P));
extern void FDECL(dodoor, (int,int,struct mkroom *));
extern void FDECL(mktrap, (int,int,struct mkroom *,coord*));
extern void FDECL(mkstairs, (XCHAR_P,XCHAR_P,CHAR_P,struct mkroom *));
extern void mkinvokearea(void);

/* ### mkmap.c ### */

void FDECL(flood_fill_rm, (int,int,int,BOOLEAN_P,BOOLEAN_P));
void FDECL(remove_rooms, (int,int,int,int));

/* ### mkmaze.c ### */

extern void FDECL(wallification, (int,int,int,int, BOOLEAN_P));
extern void FDECL(walkfrom, (int,int));
extern void FDECL(makemaz, (const char *));
extern void FDECL(mazexy, (coord *));
extern void bound_digging(void);
extern void FDECL(mkportal, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
extern boolean FDECL(bad_location, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
extern void FDECL(place_lregion, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			     XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			     XCHAR_P,d_level *));
extern void movebubbles(void);
extern void water_friction(void);
extern void FDECL(save_waterlevel, (int,int));
extern void FDECL(restore_waterlevel, (int));
extern const char *FDECL(waterbody_name, (XCHAR_P,XCHAR_P));

/* ### mkobj.c ### */

extern struct obj *FDECL(mkobj_at, (CHAR_P,int,int,BOOLEAN_P));
extern struct obj *FDECL(mksobj_at, (int,int,int,BOOLEAN_P,BOOLEAN_P));
extern struct obj *FDECL(mkobj, (CHAR_P,BOOLEAN_P));
extern int rndmonnum(void);
extern struct obj *FDECL(splitobj, (struct obj *,long));
extern void FDECL(replace_object, (struct obj *,struct obj *));
extern void FDECL(bill_dummy_object, (struct obj *));
extern struct obj *FDECL(mksobj, (int,BOOLEAN_P,BOOLEAN_P));
extern int FDECL(bcsign, (struct obj *));
extern int FDECL(weight, (struct obj *));
extern struct obj *FDECL(mkgold, (long,int,int));
extern struct obj *FDECL(mkcorpstat,
		(int,struct monst *,struct permonst *,int,int,BOOLEAN_P));
extern struct obj *FDECL(obj_attach_mid, (struct obj *, unsigned));
extern struct monst *FDECL(get_mtraits, (struct obj *, BOOLEAN_P));
extern struct obj *FDECL(mk_tt_object, (int,int,int));
extern struct obj *FDECL(mk_named_object,
			(int,struct permonst *,int,int,const char *));
extern struct obj *FDECL(rnd_treefruit_at, (int, int));
extern void FDECL(start_corpse_timeout, (struct obj *));
extern void FDECL(bless, (struct obj *));
extern void FDECL(unbless, (struct obj *));
extern void FDECL(curse, (struct obj *));
extern void FDECL(uncurse, (struct obj *));
extern void FDECL(blessorcurse, (struct obj *,int));
extern boolean FDECL(is_flammable, (struct obj *));
extern boolean FDECL(is_rottable, (struct obj *));
extern void FDECL(place_object, (struct obj *,int,int));
extern void FDECL(remove_object, (struct obj *));
extern void FDECL(discard_minvent, (struct monst *));
extern void FDECL(obj_extract_self, (struct obj *));
extern struct obj *FDECL(container_extract_indestructable, (struct obj *obj));
extern void FDECL(extract_nobj, (struct obj *, struct obj **));
extern void FDECL(extract_nexthere, (struct obj *, struct obj **));
extern int FDECL(add_to_minv, (struct monst *, struct obj *));
extern struct obj *FDECL(add_to_container, (struct obj *, struct obj *));
extern void FDECL(add_to_migration, (struct obj *));
extern void FDECL(add_to_buried, (struct obj *));
extern void FDECL(dealloc_obj, (struct obj *));
extern void FDECL(obj_ice_effects, (int, int, BOOLEAN_P));
extern long FDECL(peek_at_iced_corpse_age, (struct obj *));
#ifdef WIZARD
extern void obj_sanity_check(void);
#endif

/* ### mkroom.c ### */

extern void FDECL(mkroom, (int));
extern void FDECL(fill_zoo, (struct mkroom *));
extern boolean FDECL(nexttodoor, (int,int));
extern boolean FDECL(has_dnstairs, (struct mkroom *));
extern boolean FDECL(has_upstairs, (struct mkroom *));
extern int FDECL(somex, (struct mkroom *));
extern int FDECL(somey, (struct mkroom *));
extern boolean FDECL(inside_room, (struct mkroom *,XCHAR_P,XCHAR_P));
extern boolean FDECL(somexy, (struct mkroom *,coord *));
extern void FDECL(mkundead, (coord *,BOOLEAN_P,int));
extern struct permonst *courtmon(void);
extern struct permonst *antholemon(void);
extern struct permonst *realzoomon(void);
extern void FDECL(save_rooms, (int));
extern void FDECL(rest_rooms, (int));
extern struct mkroom *FDECL(search_special, (SCHAR_P));

/* ### mon.c ### */

extern int FDECL(undead_to_corpse, (int));
extern int FDECL(genus, (int,int));
extern int FDECL(pm_to_cham, (int));
extern int FDECL(minliquid, (struct monst *));
extern int movemon(void);
extern int FDECL(meatmetal, (struct monst *));
extern void FDECL(meatcorpse, (struct monst *));
extern int FDECL(meatobj, (struct monst *));
extern void FDECL(mpickgold, (struct monst *));
extern boolean FDECL(mpickstuff, (struct monst *,const char *));
extern int FDECL(curr_mon_load, (struct monst *));
extern int FDECL(max_mon_load, (struct monst *));
extern boolean FDECL(can_carry, (struct monst *,struct obj *));
extern int FDECL(mfndpos, (struct monst *,coord *,long *,long));
extern boolean FDECL(monnear, (struct monst *,int,int));
extern void dmonsfree(void);
extern int FDECL(mcalcmove, (struct monst*));
extern void mcalcdistress(void);
extern void FDECL(replmon, (struct monst *,struct monst *));
extern void FDECL(relmon, (struct monst *));
extern struct obj *FDECL(mlifesaver, (struct monst *));
extern boolean FDECL(corpse_chance,(struct monst *,struct monst *,BOOLEAN_P));
extern void FDECL(mondead, (struct monst *));
extern void FDECL(mondied, (struct monst *));
extern void FDECL(mongone, (struct monst *));
extern void FDECL(monstone, (struct monst *));
extern void FDECL(monkilled, (struct monst *,const char *,int));
extern void FDECL(mon_xkilled, (struct monst *,const char *,int));
extern void FDECL(unstuck, (struct monst *));
extern void FDECL(killed, (struct monst *));
extern void FDECL(xkilled, (struct monst *,int));
extern void FDECL(mon_to_stone, (struct monst*));
extern void FDECL(mnexto, (struct monst *));
extern boolean FDECL(mnearto, (struct monst *,XCHAR_P,XCHAR_P,BOOLEAN_P));
extern void FDECL(poisontell, (int));
extern void FDECL(poisoned, (const char *,int,const char *,int));
extern void FDECL(m_respond, (struct monst *));
extern void FDECL(setmangry, (struct monst *));
extern void FDECL(wakeup, (struct monst *));
extern void wake_nearby(void);
extern void FDECL(wake_nearto, (int,int,int));
extern void FDECL(seemimic, (struct monst *));
extern void rescham(void);
extern void restartcham(void);
extern void FDECL(restore_cham, (struct monst *));
extern void FDECL(mon_animal_list, (BOOLEAN_P));
extern int FDECL(newcham, (struct monst *,struct permonst *,BOOLEAN_P,BOOLEAN_P));
extern int FDECL(can_be_hatched, (int));
extern int FDECL(egg_type_from_parent, (int,BOOLEAN_P));
extern boolean FDECL(dead_species, (int,BOOLEAN_P));
extern void kill_genocided_monsters(void);
extern void FDECL(golemeffects, (struct monst *,int,int));
extern boolean FDECL(angry_guards, (BOOLEAN_P));
extern void pacify_guards(void);

/* ### mondata.c ### */

extern void FDECL(set_mon_data, (struct monst *,struct permonst *,int));
extern struct attack *FDECL(attacktype_fordmg, (struct permonst *,int,int));
extern boolean FDECL(attacktype, (struct permonst *,int));
extern boolean FDECL(poly_when_stoned, (struct permonst *));
extern boolean FDECL(resists_drli, (struct monst *));
extern boolean FDECL(resists_magm, (struct monst *));
extern boolean FDECL(resists_blnd, (struct monst *));
extern boolean FDECL(can_blnd, (struct monst *,struct monst *,UCHAR_P,struct obj *));
extern boolean FDECL(ranged_attk, (struct permonst *));
extern boolean FDECL(passes_bars, (struct permonst *));
extern boolean FDECL(can_track, (struct permonst *));
extern boolean FDECL(breakarm, (struct permonst *));
extern boolean FDECL(sliparm, (struct permonst *));
extern boolean FDECL(sticks, (struct permonst *));
extern int FDECL(num_horns, (struct permonst *));
/* extern boolean FDECL(canseemon, (struct monst *)); */
extern struct attack *FDECL(dmgtype_fromattack, (struct permonst *,int,int));
extern boolean FDECL(dmgtype, (struct permonst *,int));
extern int FDECL(max_passive_dmg, (struct monst *,struct monst *));
extern int FDECL(monsndx, (struct permonst *));
extern int FDECL(name_to_mon, (const char *));
extern int FDECL(gender, (struct monst *));
extern int FDECL(pronoun_gender, (struct monst *));
extern boolean FDECL(levl_follower, (struct monst *));
extern int FDECL(little_to_big, (int));
extern int FDECL(big_to_little, (int));
extern const char *FDECL(locomotion, (const struct permonst *,const char *));
extern const char *FDECL(stagger, (const struct permonst *,const char *));
extern const char *FDECL(on_fire, (struct permonst *,struct attack *));
extern const struct permonst *FDECL(raceptr, (struct monst *));

/* ### monmove.c ### */

extern boolean FDECL(itsstuck, (struct monst *));
extern boolean FDECL(mb_trapped, (struct monst *));
extern void FDECL(mon_regen, (struct monst *,BOOLEAN_P));
extern int FDECL(dochugw, (struct monst *));
extern boolean FDECL(onscary, (int,int,struct monst *));
extern void FDECL(monflee, (struct monst *, int, BOOLEAN_P, BOOLEAN_P));
extern int FDECL(dochug, (struct monst *));
extern int FDECL(m_move, (struct monst *,int));
extern boolean FDECL(closed_door, (int,int));
extern boolean FDECL(accessible, (int,int));
extern void FDECL(set_apparxy, (struct monst *));
extern boolean FDECL(can_ooze, (struct monst *));

/* ### monst.c ### */

extern void monst_init(void);

/* ### monstr.c ### */

extern void monstr_init(void);

/* ### mplayer.c ### */

extern struct monst *FDECL(mk_mplayer, (struct permonst *,XCHAR_P,
				   XCHAR_P,BOOLEAN_P));
extern void FDECL(create_mplayers, (int,BOOLEAN_P));
extern void FDECL(mplayer_talk, (struct monst *));

#if defined(MICRO) || defined(WIN32)

/* ### msdos.c,os2.c,tos.c,winnt.c ### */

#  ifndef WIN32
extern int tgetch(void);
#  endif
#  ifndef TOS
extern char switchar(void);
#  endif
# ifndef __GO32__
extern long FDECL(freediskspace, (char *));
#  ifdef MSDOS
extern int FDECL(findfirst_file, (char *));
extern int findnext_file(void);
extern long FDECL(filesize_nh, (char *));
#  else
extern int FDECL(findfirst, (char *));
extern int findnext(void);
extern long FDECL(filesize, (char *));
#  endif /* MSDOS */
extern char *foundfile_buffer(void);
# endif /* __GO32__ */
# ifndef __CYGWIN__
extern void FDECL(chdrive, (char *));
# endif
# ifndef TOS
extern void disable_ctrlP(void);
extern void enable_ctrlP(void);
# endif
# if defined(MICRO) && !defined(WINNT)
extern void get_scr_size(void);
#  ifndef TOS
extern void FDECL(gotoxy, (int,int));
#  endif
# endif
# ifdef TOS
extern int FDECL(_copyfile, (char *,char *));
extern int kbhit(void);
extern void set_colors(void);
extern void restore_colors(void);
#  ifdef SUSPEND
extern int dosuspend(void);
#  endif
# endif /* TOS */
# ifdef WIN32
extern char *FDECL(get_username, (int *));
extern int FDECL(set_binary_mode, (int, int));
extern void FDECL(nt_regularize, (char *));
extern int (*(void)t_kbhit));
extern void FDECL(Delay, (int));
# endif /* WIN32 */
#endif /* MICRO || WIN32 */

/* ### mthrowu.c ### */

extern int FDECL(thitu, (int,int,struct obj *,const char *));
extern int FDECL(ohitmon, (struct monst *,struct monst *,struct obj *,int,BOOLEAN_P));
extern void FDECL(thrwmu, (struct monst *));
extern int FDECL(spitmu, (struct monst *,struct attack *));
extern int FDECL(breamu, (struct monst *,struct attack *));
extern boolean FDECL(breamspot, (struct monst *, struct attack *, XCHAR_P, XCHAR_P));
extern boolean FDECL(linedup, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
extern boolean FDECL(lined_up, (struct monst *));
extern struct obj *FDECL(m_carrying, (struct monst *,int));
extern void FDECL(m_useup, (struct monst *,struct obj *));
extern void FDECL(m_throw, (struct monst *,int,int,int,int,int,struct obj *));
extern boolean FDECL(hits_bars, (struct obj **,int,int,int,int));

/* ### muse.c ### */

extern boolean FDECL(find_defensive, (struct monst *));
extern int FDECL(use_defensive, (struct monst *));
extern int FDECL(rnd_defensive_item, (struct monst *));
extern boolean FDECL(find_offensive, (struct monst *));
#ifdef USE_TRAMPOLI
extern int FDECL(mbhitm, (struct monst *,struct obj *));
#endif
extern int FDECL(use_offensive, (struct monst *));
extern int FDECL(rnd_offensive_item, (struct monst *));
extern boolean FDECL(find_misc, (struct monst *));
extern int FDECL(use_misc, (struct monst *));
extern int FDECL(rnd_misc_item, (struct monst *));
extern boolean FDECL(searches_for_item, (struct monst *,struct obj *));
extern boolean FDECL(mon_reflects, (struct monst *,const char *));
extern boolean FDECL(ureflects, (const char *,const char *));
extern boolean FDECL(munstone, (struct monst *,BOOLEAN_P));

/* ### music.c ### */

extern void awaken_soldiers(void);
extern int FDECL(do_play_instrument, (struct obj *));

/* ### nhlan.c ### */
#ifdef LAN_FEATURES
extern void init_lan_features(void);
extern char *lan_username(void);
# ifdef LAN_MAIL
extern boolean lan_mail_check(void);
extern void FDECL(lan_mail_read, (struct obj *));
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
extern int FDECL(ntposkey,(int *, int *, int *));
extern void FDECL(set_output_mode, (int));
extern void synch_cursor(void);
#endif

/* ### o_init.c ### */

extern void init_objects(void);
extern int find_skates(void);
extern void oinit(void);
extern void FDECL(savenames, (int,int));
extern void FDECL(restnames, (int));
extern void FDECL(discover_object, (int,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(undiscover_object, (int));
extern int dodiscovered(void);

/* ### objects.c ### */

extern void objects_init(void);

/* ### objnam.c ### */

extern char *FDECL(obj_typename, (int));
extern char *FDECL(simple_typename, (int));
extern boolean FDECL(obj_is_pname, (struct obj *));
extern char *FDECL(distant_name, (struct obj *,char *(*)(OBJ_P)));
extern char *FDECL(fruitname, (BOOLEAN_P));
extern char *FDECL(xname, (struct obj *));
extern char *FDECL(mshot_xname, (struct obj *));
extern boolean FDECL(the_unique_obj, (struct obj *obj));
extern char *FDECL(doname, (struct obj *));
extern boolean FDECL(not_fully_identified, (struct obj *));
extern char *FDECL(corpse_xname, (struct obj *,BOOLEAN_P));
extern char *FDECL(cxname, (struct obj *));
extern char *FDECL(killer_xname, (struct obj *));
extern char *FDECL(killer_cxname, (struct obj *,BOOLEAN_P));
extern const char *FDECL(singular, (struct obj *,char *(*)(OBJ_P)));
extern char *FDECL(an, (const char *));
extern char *FDECL(An, (const char *));
extern char *FDECL(The, (const char *));
extern char *FDECL(the, (const char *));
extern char *FDECL(aobjnam, (struct obj *,const char *));
extern char *FDECL(Tobjnam, (struct obj *,const char *));
extern char *FDECL(otense, (struct obj *,const char *));
extern char *FDECL(vtense, (const char *,const char *));
extern char *FDECL(Doname2, (struct obj *));
extern char *FDECL(yname, (struct obj *));
extern char *FDECL(Yname2, (struct obj *));
extern char *FDECL(ysimple_name, (struct obj *));
extern char *FDECL(Ysimple_name2, (struct obj *));
extern char *FDECL(makeplural, (const char *));
extern char *FDECL(makesingular, (const char *));
extern struct obj *FDECL(readobjnam, (char *,struct obj *,BOOLEAN_P));
extern int FDECL(rnd_class, (int,int));
extern const char *FDECL(cloak_simple_name, (struct obj *));
extern const char *FDECL(mimic_obj_name, (struct monst *));

/* ### options.c ### */

extern boolean FDECL(match_optname, (const char *,const char *,int,BOOLEAN_P));
extern void initoptions(void);
extern void FDECL(parseoptions, (char *,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(parsetileset, (char *));
extern int doset(void);
extern int dotogglepickup(void);
extern void option_help(void);
extern void FDECL(next_opt, (winid,const char *));
extern int FDECL(fruitadd, (char *));
extern int FDECL(choose_classes_menu, (const char *,int,BOOLEAN_P,char *,char *));
extern void FDECL(add_menu_cmd_alias, (CHAR_P, CHAR_P));
extern char FDECL(map_menu_cmd, (CHAR_P));
extern void FDECL(assign_warnings, (uchar *));
extern char *FDECL(nh_getenv, (const char *));
extern void FDECL(set_duplicate_opt_detection, (int));
extern void FDECL(set_wc_option_mod_status, (unsigned long, int));
extern void FDECL(set_wc2_option_mod_status, (unsigned long, int));
extern void FDECL(set_option_mod_status, (const char *, int));
#ifdef MENU_COLOR
extern boolean FDECL(add_menu_coloring, (char *));
#endif
extern int FDECL(add_autopickup_exception, (const char *));
extern void free_autopickup_exceptions(void);

/* ### pager.c ### */

extern int dowhatis(void);
extern int doquickwhatis(void);
extern int doidtrap(void);
extern int dowhatdoes(void);
extern char *FDECL(dowhatdoes_core,(CHAR_P, char *));
extern int dohelp(void);
extern int dohistory(void);

/* ### pcmain.c ### */

#if defined(MICRO) || defined(WIN32)
# ifdef CHDIR
extern void FDECL(chdirx, (char *,BOOLEAN_P));
# endif /* CHDIR */
#endif /* MICRO || WIN32 */

/* ### pcsys.c ### */

#if defined(MICRO) || defined(WIN32)
extern void flushout(void);
extern int dosh(void);
# ifdef MFLOPPY
extern void FDECL(eraseall, (const char *,const char *));
extern void FDECL(copybones, (int));
extern void playwoRAMdisk(void);
extern int FDECL(saveDiskPrompt, (int));
extern void gameDiskPrompt(void);
# endif
extern void FDECL(append_slash, (char *));
extern void FDECL(getreturn, (const char *));
# ifndef AMIGA
extern void VDECL(msmsg, (const char *,...));
# endif
extern FILextern *FDECL(fopenp, (const char *,const char *));
#endif /* MICRO || WIN32 */

/* ### pctty.c ### */

#if defined(MICRO) || defined(WIN32)
extern void gettty(void);
extern void FDECL(settty, (const char *));
extern void setftty(void);
extern void VDECL(error, (const char *,...));
#if defined(TIMED_DELAY) && defined(_MSC_VER)
extern void FDECL(msleep, (unsigned));
#endif
#endif /* MICRO || WIN32 */

/* ### pcunix.c ### */

#if defined(MICRO)
extern void FDECL(regularize, (char *));
#endif /* MICRO */
#if defined(PC_LOCKING)
extern void getlock(void);
#endif

/* ### pickup.c ### */

#ifdef GOLDOBJ
extern int FDECL(collect_obj_classes,
	(char *,struct obj *,BOOLEAN_P,boolean FDECL((*),(OBJ_P)), int *));
#else
extern int FDECL(collect_obj_classes,
	(char *,struct obj *,BOOLEAN_P,BOOLEAN_P,boolean FDECL((*),(OBJ_P)), int *));
#endif
extern void FDECL(add_valid_menu_class, (int));
extern boolean FDECL(allow_all, (struct obj *));
extern boolean FDECL(allow_category, (struct obj *));
extern boolean FDECL(is_worn_by_type, (struct obj *));
extern boolean FDECL(mbag_explodes, (struct obj *, int));
extern void FDECL(destroy_mbag, (struct obj *, BOOLEAN_P));
#ifdef USE_TRAMPOLI
extern int FDECL(ck_bag, (struct obj *));
extern int FDECL(in_container, (struct obj *));
extern int FDECL(out_container, (struct obj *));
#endif
extern int FDECL(pickup, (int));
extern int FDECL(pickup_object, (struct obj *, long, BOOLEAN_P));
extern int FDECL(query_category, (const char *, struct obj *, int,
				menu_item **, int));
extern int FDECL(query_objlist, (const char *, struct obj *, int,
				menu_item **, int, boolean (*)(OBJ_P)));
extern struct obj *FDECL(pick_obj, (struct obj *));
extern int encumber_msg(void);
extern int doloot(void);
extern boolean FDECL(container_gone, (int (*)(OBJ_P)));
extern int FDECL(use_container, (struct obj **,int));
extern int FDECL(loot_mon, (struct monst *,int *,boolean *));
extern const char *FDECL(safe_qbuf, (const char *,unsigned,
				const char *,const char *,const char *));
extern boolean FDECL(is_autopickup_exception, (struct obj *, BOOLEAN_P));

/* ### pline.c ### */

extern void FDECL(msgpline_add, (int, char *));
extern void msgpline_free(void);
extern void VDECL(pline, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(Norep, (const char *,...)) PRINTF_F(1,2);
extern void free_youbuf(void);
extern void VDECL(You, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(Your, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(You_feel, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(You_cant, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(You_hear, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(pline_The, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(There, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(verbalize, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(raw_printf, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(impossible, (const char *,...)) PRINTF_F(1,2);
extern const char *FDECL(align_str, (ALIGNTYP_P));
extern void FDECL(mstatusline, (struct monst *));
extern void ustatusline(void);
extern void self_invis_message(void);

/* ### polyself.c ### */

extern void init_uasmon(void);
extern void set_uasmon(void);
extern void change_sex(void);
extern void FDECL(polyself, (BOOLEAN_P));
extern int FDECL(polymon, (int));
extern void rehumanize(void);
extern int dobreathe(void);
extern int dospit(void);
extern int doremove(void);
extern int dospinweb(void);
extern int dosummon(void);
extern int dogaze(void);
extern int dohide(void);
extern int domindblast(void);
extern void FDECL(skinback, (BOOLEAN_P));
extern const char *FDECL(mbodypart, (struct monst *,int));
extern const char *FDECL(body_part, (int));
extern int poly_gender(void);
extern void FDECL(ugolemeffects, (int,int));
extern int polyatwill(void);

/* ### potion.c ### */

extern void FDECL(set_itimeout, (long *,long));
extern void FDECL(incr_itimeout, (long *,int));
extern void FDECL(make_confused, (long,BOOLEAN_P));
extern void FDECL(make_stunned, (long,BOOLEAN_P));
extern void FDECL(make_blinded, (long,BOOLEAN_P));
extern void FDECL(make_sick, (long, const char *, BOOLEAN_P,int));
extern void FDECL(make_vomiting, (long,BOOLEAN_P));
extern boolean FDECL(make_hallucinated, (long,BOOLEAN_P,long));
extern int dodrink(void);
extern int FDECL(dopotion, (struct obj *));
extern int FDECL(peffects, (struct obj *));
extern void FDECL(healup, (int,int,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(strange_feeling, (struct obj *,const char *));
extern void FDECL(potionhit, (struct monst *,struct obj *,BOOLEAN_P));
extern void FDECL(potionbreathe, (struct obj *));
extern boolean FDECL(get_wet, (struct obj *, BOOLEAN_P));
extern int dodip(void);
extern void FDECL(djinni_from_bottle, (struct obj *));
/* KMH, balance patch -- new function */
extern int FDECL(upgrade_obj, (struct obj *));
extern struct monst *FDECL(split_mon, (struct monst *,struct monst *));
extern const char *bottlename(void);

/* ### pray.c ### */

#ifdef USE_TRAMPOLI
extern int prayer_done(void);
#endif
extern int dosacrifice(void);
extern boolean FDECL(can_pray, (BOOLEAN_P));
extern int dopray(void);
extern const char *u_gname(void);
extern int doturn(void);
extern int turn_undead(void);
extern const char *a_gname(void);
extern const char *FDECL(a_gname_at, (XCHAR_P x,XCHAR_P y));
extern const char *FDECL(align_gname, (ALIGNTYP_P));
extern const char *FDECL(halu_gname, (ALIGNTYP_P));
extern const char *FDECL(align_gtitle, (ALIGNTYP_P));
extern void FDECL(altar_wrath, (int,int));


/* ### priest.c ### */

extern int FDECL(move_special, (struct monst *,BOOLEAN_P,SCHAR_P,BOOLEAN_P,BOOLEAN_P,
			   XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
extern char FDECL(temple_occupied, (char *));
extern int FDECL(pri_move, (struct monst *));
extern void FDECL(priestini, (d_level *,struct mkroom *,int,int,BOOLEAN_P));
extern char *FDECL(priestname, (struct monst *,char *));
extern boolean FDECL(p_coaligned, (struct monst *));
extern struct monst *FDECL(findpriest, (CHAR_P));
extern void FDECL(intemple, (int));
extern void FDECL(priest_talk, (struct monst *));
extern struct monst *FDECL(mk_roamer, (struct permonst *,ALIGNTYP_P,
				  XCHAR_P,XCHAR_P,BOOLEAN_P));
extern void FDECL(reset_hostility, (struct monst *));
extern boolean FDECL(in_your_sanctuary, (struct monst *,XCHAR_P,XCHAR_P));
extern void FDECL(ghod_hitsu, (struct monst *));
extern void angry_priest(void);
extern void clearpriests(void);
extern void FDECL(restpriest, (struct monst *,BOOLEAN_P));

/* ### quest.c ### */

extern void onquest(void);
extern void nemdead(void);
extern void artitouch(void);
extern boolean ok_to_quest(void);
extern void FDECL(leader_speaks, (struct monst *));
extern void nemesis_speaks(void);
extern void FDECL(quest_chat, (struct monst *));
extern void FDECL(quest_talk, (struct monst *));
extern void FDECL(quest_stat_check, (struct monst *));
extern void FDECL(finish_quest, (struct obj *));

/* ### questpgr.c ### */

extern void load_qtlist(void);
extern void unload_qtlist(void);
extern short FDECL(quest_info, (int));
extern const char *ldrname(void);
extern boolean FDECL(is_quest_artifact, (struct obj*));
extern void FDECL(com_pager, (int));
extern void FDECL(qt_pager, (int));
extern struct permonst *qt_montype(void);

/* ### random.c ### */

#if defined(RANDOM) && !defined(__GO32__) /* djgpp has its own random */
extern void FDECL(srandom, (unsigned));
extern char *FDECL(initstate, (unsigned,char *,int));
extern char *FDECL(setstate, (char *));
extern long random(void);
#endif /* RANDOM */

/* ### read.c ### */

extern int doread(void);
extern boolean FDECL(is_chargeable, (struct obj *));
extern void FDECL(recharge, (struct obj *,int));
extern void FDECL(forget, (int));
extern void FDECL(forget_objects, (int));
extern void FDECL(forget_levels, (int));
extern void forget_traps(void);
extern void FDECL(forget_map, (int));
extern int FDECL(seffects, (struct obj *));
#ifdef USE_TRAMPOLI
extern void FDECL(set_lit, (int,int,genericptr_t));
#endif
extern void FDECL(litroom, (BOOLEAN_P,struct obj *));
extern void FDECL(do_genocide, (int));
extern void FDECL(punish, (struct obj *));
extern void unpunish(void);
extern boolean FDECL(cant_create, (int *, BOOLEAN_P));
#ifdef WIZARD
extern struct monst *create_particular(void);
#endif

/* ### rect.c ### */

extern void init_rect(void);
extern NhRect *FDECL(get_rect, (NhRect *));
extern NhRect *rnd_rect(void);
extern void FDECL(remove_rect, (NhRect *));
extern void FDECL(add_rect, (NhRect *));
extern void FDECL(split_rects, (NhRect *,NhRect *));

/* ## region.c ### */
extern void clear_regions(void);
extern void run_regions(void);
extern boolean FDECL(in_out_region, (XCHAR_P,XCHAR_P));
extern boolean FDECL(m_in_out_region, (struct monst *,XCHAR_P,XCHAR_P));
extern void update_player_regions(void);
extern void FDECL(update_monster_region, (struct monst *));
extern NhRegion *FDECL(visible_region_at, (XCHAR_P,XCHAR_P));
extern void FDECL(show_region, (NhRegion*, XCHAR_P, XCHAR_P));
extern void FDECL(save_regions, (int,int));
extern void FDECL(rest_regions, (int,BOOLEAN_P));
extern NhRegion* FDECL(create_gas_cloud, (XCHAR_P, XCHAR_P, int, int));
extern NhRegion* FDECL(create_cthulhu_death_cloud, (XCHAR_P, XCHAR_P, int, int));

/* ### restore.c ### */

extern void FDECL(inven_inuse, (BOOLEAN_P));
extern int FDECL(dorecover, (int));
extern void FDECL(trickery, (char *));
extern void FDECL(getlev, (int,int,XCHAR_P,BOOLEAN_P));
extern void minit(void);
extern boolean FDECL(lookup_id_mapping, (unsigned, unsigned *));
#ifdef ZEROCOMP
extern int FDECL(mread, (int,genericptr_t,unsigned int));
#else
extern void FDECL(mread, (int,genericptr_t,unsigned int));
#endif

/* ### rip.c ### */

extern void FDECL(genl_outrip, (winid,int));

/* ### rnd.c ### */

extern int FDECL(rn2, (int));
extern int FDECL(rnl, (int));
extern int FDECL(rnd, (int));
extern int FDECL(d, (int,int));
extern int FDECL(rne, (int));
extern int FDECL(rnz, (int));

/* ### role.c ### */

extern boolean FDECL(validrole, (int));
extern boolean FDECL(validrace, (int, int));
extern boolean FDECL(validgend, (int, int, int));
extern boolean FDECL(validalign, (int, int, int));
extern int randrole(void);
extern int FDECL(randrace, (int));
extern int FDECL(randgend, (int, int));
extern int FDECL(randalign, (int, int));
extern int FDECL(str2role, (char *));
extern int FDECL(str2race, (char *));
extern int FDECL(str2gend, (char *));
extern int FDECL(str2align, (char *));
extern int FDECL(mrace2race, (int));
extern boolean FDECL(ok_role, (int, int, int, int));
extern int FDECL(pick_role, (int, int, int, int));
extern boolean FDECL(ok_race, (int, int, int, int));
extern int FDECL(pick_race, (int, int, int, int));
extern boolean FDECL(ok_gend, (int, int, int, int));
extern int FDECL(pick_gend, (int, int, int, int));
extern boolean FDECL(ok_align, (int, int, int, int));
extern int FDECL(pick_align, (int, int, int, int));
extern void role_init(void);
extern void rigid_role_checks(void);
extern void plnamesuffix(void);
extern const char *FDECL(Hello, (struct monst *));
extern const char *Goodbye(void);
extern char *FDECL(build_plselection_prompt, (char *, int, int, int, int, int));
extern char *FDECL(root_plselection_prompt, (char *, int, int, int, int, int));

/* ### rumors.c ### */

extern char *FDECL(getrumor, (int,char *, BOOLEAN_P));
extern void FDECL(outrumor, (int,int));
extern void FDECL(outoracle, (BOOLEAN_P, BOOLEAN_P));
extern void FDECL(save_oracles, (int,int));
extern void FDECL(restore_oracles, (int));
extern int FDECL(doconsult, (struct monst *));

/* ### save.c ### */

extern int dosave(void);
#if defined(UNIX) || defined(VMS) || defined(__EMX__) || defined(WIN32)
extern void FDECL(hangup, (int));
#endif
extern int dosave0(void);
#ifdef INSURANCE
extern void savestateinlock(void);
#endif
#ifdef MFLOPPY
extern boolean FDECL(savelev, (int,XCHAR_P,int));
extern boolean FDECL(swapin_file, (int));
extern void co_false(void);
#else
extern void FDECL(savelev, (int,XCHAR_P,int));
#endif
extern void FDECL(bufon, (int));
extern void FDECL(bufoff, (int));
extern void FDECL(bflush, (int));
extern void FDECL(bwrite, (int,genericptr_t,unsigned int));
extern void FDECL(bclose, (int));
extern void FDECL(savefruitchn, (int,int));
extern void free_dungeons(void);
extern void freedynamicdata(void);

/* ### shk.c ### */

#ifdef GOLDOBJ
extern long FDECL(money2mon, (struct monst *, long));
extern void FDECL(money2u, (struct monst *, long));
#endif
extern char *FDECL(shkname, (struct monst *));
extern void FDECL(shkgone, (struct monst *));
extern void FDECL(set_residency, (struct monst *,BOOLEAN_P));
extern void FDECL(replshk, (struct monst *,struct monst *));
extern void FDECL(save_shk_bill, (int, struct monst *));
extern void FDECL(restore_shk_bill, (int, struct monst *));
extern void FDECL(restshk, (struct monst *,BOOLEAN_P));
extern char FDECL(inside_shop, (XCHAR_P,XCHAR_P));
extern void FDECL(u_left_shop, (char *,BOOLEAN_P));
extern void FDECL(remote_burglary, (XCHAR_P,XCHAR_P));
extern void FDECL(u_entered_shop, (char *));
extern boolean FDECL(same_price, (struct obj *,struct obj *));
extern void shopper_financial_report(void);
extern int FDECL(inhishop, (struct monst *));
extern struct monst *FDECL(shop_keeper, (CHAR_P));
extern boolean FDECL(tended_shop, (struct mkroom *));
extern void FDECL(delete_contents, (struct obj *));
extern void FDECL(obfree, (struct obj *,struct obj *));
extern void FDECL(shk_free, (struct monst *));
extern void FDECL(home_shk, (struct monst *,BOOLEAN_P));
extern void FDECL(make_happy_shk, (struct monst *,BOOLEAN_P));
extern void FDECL(hot_pursuit, (struct monst *));
extern void FDECL(make_angry_shk, (struct monst *,XCHAR_P,XCHAR_P));
extern int dopay(void);
extern boolean FDECL(paybill, (int));
extern void finish_paybill(void);
extern struct obj *FDECL(find_oid, (unsigned));
extern long FDECL(contained_cost, (struct obj *,struct monst *,long,BOOLEAN_P, BOOLEAN_P));
extern long FDECL(contained_gold, (struct obj *));
extern void FDECL(picked_container, (struct obj *));
extern long FDECL(unpaid_cost, (struct obj *));
extern void FDECL(addtobill, (struct obj *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(splitbill, (struct obj *,struct obj *));
extern void FDECL(subfrombill, (struct obj *,struct monst *));
extern long FDECL(stolen_value, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P,BOOLEAN_P,
			    BOOLEAN_P));
extern void FDECL(sellobj_state, (int));
extern void FDECL(sellobj, (struct obj *,XCHAR_P,XCHAR_P));
extern int FDECL(doinvbill, (int));
extern struct monst *FDECL(shkcatch, (struct obj *,XCHAR_P,XCHAR_P));
extern void FDECL(add_damage, (XCHAR_P,XCHAR_P,long));
extern int FDECL(repair_damage, (struct monst *,struct damage *,BOOLEAN_P));
extern int FDECL(shk_move, (struct monst *));
extern void FDECL(after_shk_move, (struct monst *));
extern boolean FDECL(is_fshk, (struct monst *));
extern void FDECL(shopdig, (int));
extern void FDECL(pay_for_damage, (const char *,BOOLEAN_P));
extern boolean FDECL(costly_spot, (XCHAR_P,XCHAR_P));
extern struct obj *FDECL(shop_object, (XCHAR_P,XCHAR_P));
extern void FDECL(price_quote, (struct obj *));
extern void FDECL(shk_chat, (struct monst *));
extern void FDECL(check_unpaid_usage, (struct obj *,BOOLEAN_P));
extern void FDECL(check_unpaid, (struct obj *));
extern void FDECL(costly_gold, (XCHAR_P,XCHAR_P,long));
extern boolean FDECL(block_door, (XCHAR_P,XCHAR_P));
extern boolean FDECL(block_entry, (XCHAR_P,XCHAR_P));
extern boolean FDECL(block_entry, (XCHAR_P,XCHAR_P));
extern void FDECL(blkmar_guards, (struct monst *));
extern char *FDECL(shk_your, (char *,struct obj *));
extern char *FDECL(Shk_Your, (char *,struct obj *));

/* ### shknam.c ### */

extern void FDECL(stock_room, (int,struct mkroom *));
extern boolean FDECL(saleable, (struct monst *,struct obj *));
extern int FDECL(get_shop_item, (int));

/* ### sit.c ### */

extern void take_gold(void);
extern int dosit(void);
extern void rndcurse(void);
extern void attrcurse(void);

/* ### sounds.c ### */

extern void dosounds(void);
extern void FDECL(pet_distress, (struct monst *, int));
extern const char *FDECL(growl_sound, (struct monst *));
/* JRN: converted growl,yelp,whimper to macros based on pet_distress.
  Putting them here since I don't know where else (TOFIX) */
#define growl(mon) pet_distress((mon),3)
#define yelp(mon) pet_distress((mon),2)
#define whimper(mon) pet_distress((mon),1)
extern void FDECL(beg, (struct monst *));
extern int dotalk(void);
#ifdef USER_SOUNDS
extern int FDECL(add_sound_mapping, (const char *));
extern void FDECL(play_sound_for_message, (const char *));
#endif

/* ### sys/msdos/sound.c ### */

#ifdef MSDOS
extern int FDECL(assign_soundcard, (char *));
#endif

/* ### sp_lev.c ### */

extern boolean FDECL(check_room, (xchar *,xchar *,xchar *,xchar *,BOOLEAN_P));
extern boolean FDECL(create_room, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			      XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
extern void FDECL(create_secret_door, (struct mkroom *,XCHAR_P));
extern boolean FDECL(dig_corridor, (coord *,coord *,BOOLEAN_P,SCHAR_P,SCHAR_P));
extern void FDECL(fill_room, (struct mkroom *,BOOLEAN_P));
extern boolean FDECL(load_special, (const char *));

/* ### spell.c ### */

#ifdef USE_TRAMPOLI
extern int learn(void);
#endif
extern int FDECL(study_book, (struct obj *));
extern void FDECL(book_disappears, (struct obj *));
extern void FDECL(book_substitution, (struct obj *,struct obj *));
extern void age_spells(void);
extern int docast(void);
extern int FDECL(spell_skilltype, (int));
extern int FDECL(spelleffects, (int,BOOLEAN_P));
extern void losespells(void);
extern int dovspell(void);
extern void FDECL(learnspell, (struct obj *));
extern boolean studyspell(void);
extern void FDECL(initialspell, (struct obj *));

/* ### steal.c ### */

#ifdef USE_TRAMPOLI
extern int stealarm(void);
#endif
#ifdef GOLDOBJ
extern long FDECL(somegold, (long));
#else
extern long somegold(void);
#endif
extern void FDECL(stealgold, (struct monst *));
extern void FDECL(remove_worn_item, (struct obj *,BOOLEAN_P));
extern int FDECL(steal, (struct monst *, char *));
extern int FDECL(mpickobj, (struct monst *,struct obj *));
extern void FDECL(stealamulet, (struct monst *));
extern void FDECL(mdrop_special_objs, (struct monst *));
extern void FDECL(relobj, (struct monst *,int,BOOLEAN_P));
#ifdef GOLDOBJ
extern struct obj *FDECL(findgold, (struct obj *));
#endif

/* ### steed.c ### */

#ifdef STEED
extern void rider_cant_reach(void);
extern boolean FDECL(can_saddle, (struct monst *));
extern int FDECL(use_saddle, (struct obj *));
extern boolean FDECL(can_ride, (struct monst *));
extern int doride(void);
extern boolean FDECL(mount_steed, (struct monst *, BOOLEAN_P));
extern void exercise_steed(void);
extern void kick_steed(void);
extern void FDECL(dismount_steed, (int));
extern void FDECL(place_monster, (struct monst *,int,int));
#endif

/* ### tech.c ### */

extern void FDECL(adjtech, (int,int));
extern int dotech(void);
extern void docalm(void);
extern int FDECL(tech_inuse, (int));
extern void tech_timeout(void);
extern boolean FDECL(tech_known, (SHORT_P));
extern void FDECL(learntech, (SHORT_P,long,int));

/* ### teleport.c ### */

extern boolean FDECL(goodpos, (int,int,struct monst *,unsigned));
extern boolean FDECL(enexto, (coord *,XCHAR_P,XCHAR_P,struct permonst *));
extern boolean FDECL(enexto_core, (coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned));
extern int FDECL(epathto, (coord *,int,XCHAR_P,XCHAR_P,struct permonst *));
extern boolean FDECL(wpathto, (coord *, coord *, boolean (*)(genericptr_t, int, int),
			  genericptr_t, struct permonst *, int));
extern void FDECL(xpathto, (int,XCHAR_P,XCHAR_P,int (*)(genericptr_t,int,int),void *));
extern void FDECL(teleds, (int,int,BOOLEAN_P));
extern boolean FDECL(safe_teleds, (BOOLEAN_P));
extern boolean FDECL(teleport_pet, (struct monst *,BOOLEAN_P));
extern void tele(void);
extern int dotele(void);
extern void level_tele(void);
extern void FDECL(domagicportal, (struct trap *));
extern void FDECL(tele_trap, (struct trap *));
extern void FDECL(level_tele_trap, (struct trap *));
extern void FDECL(rloc_to, (struct monst *,int,int));
extern boolean FDECL(rloc, (struct monst *, BOOLEAN_P));
extern boolean FDECL(tele_restrict, (struct monst *));
extern void FDECL(mtele_trap, (struct monst *, struct trap *,int));
extern int FDECL(mlevel_tele_trap, (struct monst *, struct trap *,BOOLEAN_P,int));
extern void FDECL(rloco, (struct obj *));
extern int random_teleport_level(void);
extern boolean FDECL(u_teleport_mon, (struct monst *,BOOLEAN_P));

/* ### tile.c ### */
#ifdef USE_TILES
extern void FDECL(substitute_tiles, (d_level *));
#endif

/* ### timeout.c ### */

extern void burn_away_slime(void);
extern void nh_timeout(void);
extern void FDECL(fall_asleep, (int, BOOLEAN_P));
#ifdef UNPOLYPILE
extern void FDECL(set_obj_poly, (struct obj *, struct obj *));
extern void FDECL(unpoly_obj, (genericptr_t, long));
#endif
extern int FDECL(mon_poly, (struct monst *, BOOLEAN_P, const char *));
extern int FDECL(mon_spec_poly, (struct monst *, struct permonst *, long,
			    BOOLEAN_P, BOOLEAN_P, BOOLEAN_P, BOOLEAN_P));
extern void FDECL(unpoly_mon, (genericptr_t, long));
extern void FDECL(attach_bomb_blow_timeout, (struct obj *, int, BOOLEAN_P));
extern void FDECL(attach_egg_hatch_timeout, (struct obj *));
extern void FDECL(attach_fig_transform_timeout, (struct obj *));
extern void FDECL(kill_egg, (struct obj *));
extern void FDECL(hatch_egg, (genericptr_t, long));
extern void FDECL(learn_egg_type, (int));
extern void FDECL(burn_object, (genericptr_t, long));
extern void FDECL(begin_burn, (struct obj *, BOOLEAN_P));
extern void FDECL(end_burn, (struct obj *, BOOLEAN_P));
extern void FDECL(burn_faster, (struct obj *, long));
#ifdef LIGHTSABERS
extern void FDECL(lightsaber_deactivate, (struct obj *, BOOLEAN_P));
#endif
extern void do_storms(void);
extern boolean FDECL(start_timer, (long, SHORT_P, SHORT_P, genericptr_t));
extern long FDECL(stop_timer, (SHORT_P, genericptr_t));
extern void run_timers(void);
extern void FDECL(obj_move_timers, (struct obj *, struct obj *));
extern void FDECL(obj_split_timers, (struct obj *, struct obj *));
extern void FDECL(obj_stop_timers, (struct obj *));
extern void FDECL(mon_stop_timers, (struct monst *));
extern boolean FDECL(obj_is_local, (struct obj *));
extern void FDECL(save_timers, (int,int,int));
extern void FDECL(restore_timers, (int,int,BOOLEAN_P,long));
extern void FDECL(relink_timers, (BOOLEAN_P));
#ifdef WIZARD
extern int wiz_timeout_queue(void);
extern void timer_sanity_check(void);
#endif

/* ### topten.c ### */

extern void FDECL(topten, (int));
extern void FDECL(prscore, (int,char **));
extern struct obj *FDECL(tt_oname, (struct obj *));
#ifdef GTK_GRAPHICS
extern winid create_toptenwin(void);
extern void destroy_toptenwin(void);
#endif

/* ### track.c ### */

extern void initrack(void);
extern void settrack(void);
extern coord *FDECL(gettrack, (int,int));

/* ### trap.c ### */

extern boolean FDECL(burnarmor,(struct monst *));
extern boolean FDECL(rust_dmg, (struct obj *,const char *,int,BOOLEAN_P,struct monst *));
extern void FDECL(grease_protect, (struct obj *,const char *,struct monst *));
extern struct trap *FDECL(maketrap, (int,int,int));
extern void FDECL(fall_through, (BOOLEAN_P));
extern struct monst *FDECL(animate_statue, (struct obj *,XCHAR_P,XCHAR_P,int,int *));
extern struct monst *FDECL(activate_statue_trap,
			(struct trap *,XCHAR_P,XCHAR_P,BOOLEAN_P));
extern void FDECL(dotrap, (struct trap *, unsigned));
extern void FDECL(seetrap, (struct trap *));
extern int FDECL(mintrap, (struct monst *));
extern void FDECL(instapetrify, (const char *));
extern void FDECL(minstapetrify, (struct monst *,BOOLEAN_P));
extern void FDECL(selftouch, (const char *));
extern void FDECL(mselftouch, (struct monst *,const char *,BOOLEAN_P));
extern void float_up(void);
extern void FDECL(fill_pit, (int,int));
extern int FDECL(float_down, (long, long));
extern int FDECL(fire_damage, (struct obj *,BOOLEAN_P,BOOLEAN_P,XCHAR_P,XCHAR_P));
extern void FDECL(water_damage, (struct obj *,BOOLEAN_P,BOOLEAN_P));
extern boolean drown(void);
extern void FDECL(mon_drain_en, (struct monst *, int));
extern void FDECL(drain_en, (int));
extern int dountrap(void);
extern int FDECL(untrap, (BOOLEAN_P));
extern boolean FDECL(chest_trap, (struct obj *,int,BOOLEAN_P));
extern void FDECL(deltrap, (struct trap *));
extern boolean FDECL(delfloortrap, (struct trap *));
extern struct trap *FDECL(t_at, (int,int));
extern void FDECL(b_trapped, (const char *,int));
extern boolean unconscious(void);
extern boolean lava_effects(void);
extern void FDECL(blow_up_landmine, (struct trap *));
extern int FDECL(launch_obj,(SHORT_P,int,int,int,int,int));
/* KMH, balance patch -- new function */
extern int uunstone(void);

/* ### u_init.c ### */

extern void u_init(void);

/* ### uhitm.c ### */

extern void FDECL(hurtmarmor,(struct monst *,int));
extern int FDECL(attack_checks, (struct monst *,BOOLEAN_P));
extern void FDECL(check_caitiff, (struct monst *));
extern schar FDECL(find_roll_to_hit, (struct monst *));
extern boolean FDECL(attack, (struct monst *));
extern boolean FDECL(hmon, (struct monst *,struct obj *,int));
extern int FDECL(damageum, (struct monst *,struct attack *));
extern void FDECL(missum, (struct monst *,int, int, struct attack *));
extern int FDECL(passive, (struct monst *,int,int,UCHAR_P));
extern void FDECL(passive_obj, (struct monst *,struct obj *,struct attack *));
extern void FDECL(stumble_onto_mimic, (struct monst *));
extern int FDECL(flash_hits_mon, (struct monst *,struct obj *));

/* ### unixmain.c ### */

#ifdef UNIX
# ifdef PORT_HELP
extern void port_help(void);
# endif
#endif /* UNIX */

/* ### unixtty.c ### */

#if defined(UNIX) || defined(__BEOS__)
extern void gettty(void);
extern void FDECL(settty, (const char *));
extern void setftty(void);
extern void intron(void);
extern void introff(void);
extern void VDECL(error, (const char *,...)) PRINTF_F(1,2);
#endif /* UNIX || __BEOS_ */

/* ### unixunix.c ### */

#ifdef UNIX
extern void getlock(void);
extern void FDECL(regularize, (char *));
# if defined(TIMED_DELAY) && !defined(msleep) && defined(SYSV)
extern void FDECL(msleep, (unsigned));
# endif
# ifdef SHELL
extern int dosh(void);
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
extern int FDECL(child, (int));
# endif
#ifdef FILE_AREAS
extern char *FDECL(make_file_name, (const char *, const char *));
extern FILextern *FDECL(fopen_datafile_area, (const char *,const char *,const char *,
				BOOLEAN_P));
extern FILextern *FDECL(freopen_area, (const char *,const char *,const char *, FILextern *));
extern int FDECL(chmod_area, (const char *, const char *, int));
extern int FDECL(open_area, (const char *, const char *, int, int));
extern int FDECL(creat_area, (const char *, const char *, int));
extern boolean FDECL(lock_file_area, (const char *, const char *,int));
extern void FDECL(unlock_file_area, (const char *, const char *));
#endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
extern int FDECL(hide_privileges, (BOOLEAN_P));
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
# ifdef GNOME_GRAPHICS 
extern int FDECL(hide_privileges, (BOOLEAN_P));
# endif
#endif /* UNIX */

/* ### vault.c ### */

extern boolean FDECL(grddead, (struct monst *));
extern char FDECL(vault_occupied, (char *));
extern void invault(void);
extern int FDECL(gd_move, (struct monst *));
extern void paygd(void);
extern long hidden_gold(void);
extern boolean gd_sound(void);

/* ### version.c ### */

extern char *FDECL(version_string, (char *));
extern char *FDECL(getversionstring, (char *));
extern int doversion(void);
extern int doextversion(void);
#ifdef MICRO
extern boolean FDECL(comp_times, (long));
#endif
extern boolean FDECL(check_version, (struct version_info *,
				const char *,BOOLEAN_P));
extern unsigned long FDECL(get_feature_notice_ver, (char *));
extern unsigned long get_current_feature_ver(void);
#ifdef RUNTIME_PORT_ID
extern void FDECL(append_port_id, (char *));
#endif

/* ### video.c ### */

#ifdef MSDOS
extern int FDECL(assign_video, (char *));
# ifdef NO_TERMS
extern void gr_init(void);
extern void gr_finish(void);
# endif
extern void FDECL(tileview,(BOOLEAN_P));
#endif
#ifdef VIDEOSHADES
extern int FDECL(assign_videoshades, (char *));
extern int FDECL(assign_videocolors, (char *));
#endif

/* ### vis_tab.c ### */

#ifdef VISION_TABLES
extern void vis_tab_init(void);
#endif

/* ### vision.c ### */

extern void vision_init(void);
extern int FDECL(does_block, (int,int,struct rm*));
extern void vision_reset(void);
extern void FDECL(vision_recalc, (int));
extern void FDECL(block_point, (int,int));
extern void FDECL(unblock_point, (int,int));
extern boolean FDECL(clear_path, (int,int,int,int));
extern void FDECL(do_clear_area, (int,int,int,
			     void (*)(int,int,genericptr_t),genericptr_t));

#ifdef VMS

/* ### vmsfiles.c ### */

extern int FDECL(vms_link, (const char *,const char *));
extern int FDECL(vms_unlink, (const char *));
extern int FDECL(vms_creat, (const char *,unsigned int));
extern int FDECL(vms_open, (const char *,int,unsigned int));
extern boolean FDECL(same_dir, (const char *,const char *));
extern int FDECL(c__translate, (int));
extern char *FDECL(vms_basename, (const char *));

/* ### vmsmail.c ### */

extern unsigned long init_broadcast_trapping(void);
extern unsigned long enable_broadcast_trapping(void);
extern unsigned long disable_broadcast_trapping(void);
# if 0
extern struct mail_info *parse_next_broadcast(void);
# endif /*0*/

/* ### vmsmain.c ### */

extern int FDECL(main, (int, char **));
# ifdef CHDIR
extern void FDECL(chdirx, (const char *,BOOLEAN_P));
# endif /* CHDIR */

/* ### vmsmisc.c ### */

extern void vms_abort(void);
extern void FDECL(vms_exit, (int));

/* ### vmstty.c ### */

extern int vms_getchar(void);
extern void gettty(void);
extern void FDECL(settty, (const char *));
extern void FDECL(shuttty, (const char *));
extern void setftty(void);
extern void intron(void);
extern void introff(void);
extern void VDECL(error, (const char *,...)) PRINTF_F(1,2);
#ifdef TIMED_DELAY
extern void FDECL(msleep, (unsigned));
#endif

/* ### vmsunix.c ### */

extern void getlock(void);
extern void FDECL(regularize, (char *));
extern int vms_getuid(void);
extern boolean FDECL(file_is_stmlf, (int));
extern int FDECL(vms_define, (const char *,const char *,int));
extern int FDECL(vms_putenv, (const char *));
extern char *verify_termcap(void);
# if defined(CHDIR) || defined(SHELL) || defined(SECURE)
extern void privoff(void);
extern void privon(void);
# endif
# ifdef SHELL
extern int dosh(void);
# endif
# if defined(SHELL) || defined(MAIL)
extern int FDECL(vms_doshell, (const char *,BOOLEAN_P));
# endif
# ifdef SUSPEND
extern int dosuspend(void);
# endif

#endif /* VMS */

/* ### weapon.c ### */

extern int FDECL(hitval, (struct obj *,struct monst *));
extern int FDECL(dmgval, (struct obj *,struct monst *));
extern struct obj *FDECL(select_rwep, (struct monst *));
extern struct obj *FDECL(select_hwep, (struct monst *));
extern void FDECL(possibly_unwield, (struct monst *,BOOLEAN_P));
extern int FDECL(mon_wield_item, (struct monst *));
extern int abon(void);
extern int dbon(void);
extern int enhance_weapon_skill(void);
#ifdef DUMP_LOG
extern void dump_weapon_skill(void);
#endif
extern void FDECL(unrestrict_weapon_skill, (int));
extern void FDECL(use_skill, (int,int));
extern void FDECL(add_weapon_skill, (int));
extern void FDECL(lose_weapon_skill, (int));
extern int FDECL(weapon_type, (struct obj *));
extern int uwep_skill_type(void);
extern int FDECL(weapon_hit_bonus, (struct obj *));
extern int FDECL(weapon_dam_bonus, (struct obj *));
extern int FDECL(skill_bonus, (int));
extern void FDECL(skill_init, (const struct def_skill *));
extern void practice_weapon(void);

/* ### were.c ### */

extern int FDECL(counter_were,(int));
extern void FDECL(were_change, (struct monst *));
extern void FDECL(new_were, (struct monst *));
extern int FDECL(were_summon, (struct permonst *,BOOLEAN_P,int *,char *));
extern void you_were(void);
extern void FDECL(you_unwere, (BOOLEAN_P));

/* ### wield.c ### */

extern void FDECL(setuwep, (struct obj *,BOOLEAN_P));
extern void FDECL(setuqwep, (struct obj *));
extern void FDECL(setuswapwep, (struct obj *,BOOLEAN_P));
extern int dowield(void);
extern int doswapweapon(void);
extern int dowieldquiver(void);
extern boolean FDECL(wield_tool, (struct obj *,const char *));
extern int can_twoweapon(void);
extern void drop_uswapwep(void);
extern int dotwoweapon(void);
extern void uwepgone(void);
extern void uswapwepgone(void);
extern void uqwepgone(void);
extern void untwoweapon(void);
extern void FDECL(erode_obj, (struct obj *,BOOLEAN_P,BOOLEAN_P));
extern int FDECL(chwepon, (struct obj *,int));
extern int FDECL(welded, (struct obj *));
extern void FDECL(weldmsg, (struct obj *));
extern void FDECL(setmnotwielded, (struct monst *,struct obj *));
extern void FDECL(unwield, (struct obj *,BOOLEAN_P));

/* ### windows.c ### */

extern void FDECL(choose_windows, (const char *));
extern char FDECL(genl_message_menu, (CHAR_P,int,const char *));
extern void FDECL(genl_preference_update, (const char *));

/* ### wizard.c ### */

extern void amulet(void);
extern int FDECL(mon_has_amulet, (struct monst *));
extern int FDECL(mon_has_special, (struct monst *));
extern int FDECL(tactics, (struct monst *));
extern void aggravate(void);
extern void clonewiz(void);
extern int pick_nasty(void);
extern int FDECL(nasty, (struct monst*));
extern void resurrect(void);
extern void intervene(void);
extern void wizdead(void);
extern void FDECL(cuss, (struct monst *));

/* ### worm.c ### */

extern int get_wormno(void);
extern void FDECL(initworm, (struct monst *,int));
extern void FDECL(worm_move, (struct monst *));
extern void FDECL(worm_nomove, (struct monst *));
extern void FDECL(wormgone, (struct monst *));
extern void FDECL(wormhitu, (struct monst *));
extern int FDECL(cutworm, (struct monst *,XCHAR_P,XCHAR_P,struct obj *));
extern void FDECL(see_wsegs, (struct monst *));
extern void FDECL(detect_wsegs, (struct monst *,BOOLEAN_P));
extern void FDECL(save_worm, (int,int));
extern void FDECL(rest_worm, (int));
extern void FDECL(place_wsegs, (struct monst *));
extern void FDECL(remove_worm, (struct monst *));
extern void FDECL(place_worm_tail_randomly, (struct monst *,XCHAR_P,XCHAR_P));
extern int FDECL(count_wsegs, (struct monst *));
extern boolean FDECL(worm_known, (struct monst *));

/* ### worn.c ### */

extern void FDECL(setworn, (struct obj *,long));
extern void FDECL(setnotworn, (struct obj *));
extern void FDECL(mon_set_minvis, (struct monst *));
extern void FDECL(mon_adjust_speed, (struct monst *,int,struct obj *));
extern void FDECL(update_mon_intrinsics,
		(struct monst *,struct obj *,BOOLEAN_P,BOOLEAN_P));
extern int FDECL(find_mac, (struct monst *));
extern void FDECL(m_dowear, (struct monst *,BOOLEAN_P));
extern struct obj *FDECL(which_armor, (struct monst *,long));
extern void FDECL(mon_break_armor, (struct monst *,BOOLEAN_P));
extern void FDECL(bypass_obj, (struct obj *));
extern void clear_bypasses(void);
extern int FDECL(racial_exception, (struct monst *, struct obj *));

/* ### write.c ### */

extern int FDECL(dowrite, (struct obj *));

/* ### zap.c ### */

extern int FDECL(bhitm, (struct monst *,struct obj *));
extern void FDECL(probe_monster, (struct monst *));
extern boolean FDECL(get_obj_location, (struct obj *,xchar *,xchar *,int));
extern boolean FDECL(get_mon_location, (struct monst *,xchar *,xchar *,int));
extern struct monst *FDECL(get_container_location, (struct obj *obj, int *, int *));
extern struct monst *FDECL(montraits, (struct obj *,coord *));
extern struct monst *FDECL(revive, (struct obj *));
extern int FDECL(unturn_dead, (struct monst *));
extern void FDECL(cancel_item, (struct obj *));
extern boolean FDECL(drain_item, (struct obj *));	/* KMH */
extern boolean FDECL(obj_resists, (struct obj *,int,int));
extern boolean FDECL(obj_shudders, (struct obj *));
extern void FDECL(do_osshock, (struct obj *));
extern void FDECL(puton_worn_item, (struct obj *));
#ifdef INVISIBLE_OBJECTS
extern void FDECL(obj_set_oinvis, (struct obj *, BOOLEAN_P, BOOLEAN_P));
#endif
extern struct obj *FDECL(poly_obj, (struct obj *, int));
extern int FDECL(bhito, (struct obj *,struct obj *));
extern int FDECL(bhitpile, (struct obj *,int (*)(OBJ_P,OBJ_P),int,int));
extern int FDECL(zappable, (struct obj *));
extern void FDECL(zapnodir, (struct obj *));
extern int dozap(void);
extern int FDECL(zapyourself, (struct obj *,BOOLEAN_P));
extern boolean FDECL(cancel_monst, (struct monst *,struct obj *,
			       BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(weffects, (struct obj *));
extern int FDECL(spell_damage_bonus, (int));
extern const char *FDECL(exclam, (int force));
extern void FDECL(hit, (const char *,struct monst *,const char *));
extern void FDECL(miss, (const char *,struct monst *));
extern struct monst *FDECL(bhit, (int,int,int,int,int (*)(MONST_P,OBJ_P),
			     int (*)(OBJ_P,OBJ_P),struct obj **));
extern struct monst *FDECL(boomhit, (int,int));
extern int FDECL(burn_floor_paper, (int,int,BOOLEAN_P,BOOLEAN_P));
extern void FDECL(buzz, (int,int,XCHAR_P,XCHAR_P,int,int));
extern void FDECL(melt_ice, (XCHAR_P,XCHAR_P));
extern int FDECL(zap_over_floor, (XCHAR_P,XCHAR_P,int,boolean *));
extern void FDECL(fracture_rock, (struct obj *));
extern boolean FDECL(break_statue, (struct obj *));
extern void FDECL(destroy_item, (int,int));
extern int FDECL(destroy_mitem, (struct monst *,int,int));
extern int FDECL(resist, (struct monst *,CHAR_P,int,int));
extern void makewish(void);
/* KMH -- xchar to XCHAR_P */
extern void FDECL(zap_strike_fx, (XCHAR_P, XCHAR_P, int));
extern void throwspell(void);

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

#endif /* EXTERN_H */
