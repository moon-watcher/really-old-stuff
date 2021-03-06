/*
	Revenge of the Timelord
	by Haroldo de Oliveira pinheiro
*/

#include "genesis.h"
#include "utils.h"
#include "psgsound.h"
#include "conio.h"
#include "rdc.h"
#include "utils.h"
#include "sprhand.h"
#include "actor.h"
#include "rpgmap.h"
#include "randmap.h"
#include "rdctile.h"
#include "ai.h"
#include "shot.h"
#include "bkgs.h"
#include "timelord.h"

uint bkg_pal[]={
0x000,
0xA00,
0x0A0,
0xAA0,
0x00A,
0xA0A,
0x0AA,
0xAAA,
0x888,
0xE88,
0x8E8,
0xEE8,
0x88E,
0xE8E,
0x8EE,
0xEEE
};

uint saucer_pal[]={
0x000,
0x002,
0x006,
0x008,
0x00A,
0x00E,
0x022,
0x066,
0x088,
0x0AA,
0x0EE,
0x222,
0x666,
0x888,
0xAAA,
0xEEE
};

uint alt_pal[]={
0x000,
0x044,
0x088,
0x0EE,
0x440,
0x880,
0xEE0,
0x404,
0x808,
0xE0E,
0x8EE,
0xAEE,
0x022,
0x466,
0x8AA,
0xCEE
};

uint player_pal[]={
0x000,
0x200,
0x600,
0x800,
0xA00,
0xE00,
0x022,
0x066,
0x088,
0x0AA,
0x0EE,
0x222,
0x666,
0x888,
0xAAA,
0xEEE
};

uint vortex_pal[]={
0x000,
0x200,
0x220,
0x420,
0x440,
0x640,
0x662,
0x862,
0x882,
0xA86,
0xAA6,
0xCA6,
0xCCA,
0xECA,
0xEEA,
0xEEE
};

uint skull_pal[]={
0x000,
0x020,
0x040,
0x060,
0x080,
0x0A0,
0x0C0,
0x0E0,
0x282,
0x4A4,
0x6C6,
0x2E2,
0x4E4,
0x6E6,
0x8E8,
0xAEA
};

uint shot2[] = {
	SET_TONEX+350,
	SET_VOLX+10,
	SET_TONEX+300,
	SET_TONEX+250,
	SET_TONEX+200,
	SET_TONEX+270,
	SET_TONEX+350,
	SET_TONEX+440,
	SET_TONEX+460,
	SET_TONEX+480,
	SET_TONEX+500,
	SET_TONEX+520,
	SET_TONEX+540,
	SET_VOLX+0,
	END_SOUND
};

char taunts[][14] = {
"LASTFORL.SND",
"NOCHANCE.SND",
"PREPDIE.SND",
"REALHOT.SND",
"THNKDEFT.SND"
};

char praise[][14] = {
"BETTER.SND",
"IMPRESS.SND",
"NOTBAD.SND"
};

char *gameover[] = {
"ISMINE.SND",
"PATHETIC.SND",
"SOSOON.SND"
};

void play_level();
void choose_enemies();
void generate_map();
void handle_player(player, joy);

uint shot_dly;
uint tgt_angle, cur_angle;
sprite_rec *player;
uint gem_count;
uint lives;
uint level;
uint player_dead;
ulong score;

void main()
{
    register uint i,j,joy;
	uint x, y;
	ulong tmp;
	uint countdown;

    /* Initialize Genesis GFX registers */
    init_GFX();
	RAZ();

    /* Wait until GFX reg are initialized */
    wait_sync();

	/* Inicializa o Sega Genesis File System */
	init_GFS();

	init_joypad();

	get_GFS_fileaddr(&tmp, "FONT.CHR");
	loadfont((uchar *)tmp,0,256,15,0);
	init_text_mode();

	player = actor_sprites;

	while(TRUE){
		clrscr();
		scrollv(0, 0);
		scrollh(0, 0);
		sprite_prepare();
		show_sprite(1,80);

/*
		load_bkgnd_dly(0, 140);
		load_bkgnd_dly(1, 140);
		load_bkgnd_dly(2, 140);
*/

		lives = 3;
		level = 1;
		score = 0;
		while(lives) {
			play_level();
		}
	}
}

void play_level()
{
	uint i, j;
	uint joy;

	clrscr();

	tileset_load_RDC("SHOTS.RDC",  240, 16);
	tileset_load_RDC("PLAYER.RDC", 256, 144);
	tileset_load_RDC("EXPLO.RDC",  400, 64);
	tileset_load_RDC("SAUCER.RDC", 512, 64);
	tileset_load_RDC("MISSILES.RDC", 576, 64);
	tileset_load_RDC("GUIDED.RDC", 640, 64);
	tileset_load_RDC("TRACER.RDC", 704, 64);
	tileset_load_RDC("VORTEX.RDC", 768, 256);
	tileset_load_RDC("SKULL.RDC", 1024, 64);

    set_colors(0, bkg_pal);
    set_colors(1, player_pal);
    set_colors(2, saucer_pal);
    set_colors(3, vortex_pal);

	/* Main game loop */
	saucers_cnt = 1;
	while(lives && saucers_cnt){
		sprite_init(&actor_sprites[0], 1, 144, 192, 0xA00, BUILD_ATTR(1,0,0,0)|256, 9);
		actor_sprites[0].frm_cnt = 16;
		actor_sprites[0].frm_spd = 0xFFL;
		actor_sprites[0].xfric   = 0x3F;
		actor_sprites[0].yfric   = 0x3F;
		actor_sprites[0].xmaxac  = 0x600;
		actor_sprites[0].ymaxac  = 0x600;

		ai_reset();
		shot_reset();
		shot_dly = 0;

		actor_map_x = 0;
		actor_map_y = 0;

		tgt_angle = 0;
		cur_angle = 0;

/*		ai_spawn_enemy(152, 80, ENEMY_TYPE_VORTEX);*/

		for(i = 1; i != MAX_ACTORS; i++) {
			sprite_init(&actor_sprites[i], 0, -32, -32, 0, 0, 0);
		}

    	set_colors(2, skull_pal);

		sprite_init(&actor_sprites[35], 36, 144, 72, 0xF00, BUILD_ATTR(3,0,0,0)|768, 16);
		actor_sprites[35].frm_cnt = 16;
		actor_sprites[35].frm_spd = 0xFFL;

		sprite_init(&actor_sprites[34], 35, 145, 72, 0xF00, BUILD_ATTR(2,0,0,0)|1024, 16);
		actor_sprites[34].frm_cnt = 4;
		actor_sprites[34].frm_spd = 0x7FL;

		if (!(level % 3)) {
			play_GFS_sound(praise[random(3)]);
		} else {
			play_GFS_sound(taunts[random(5)]);
		}

		j = lastsamplesize / 120;
		while (j) {
			joy = read_joypad1();

			handle_player(player, joy);

			for(i = 1; i != MAX_ACTORS; i++) {
				sprite_move(&actor_sprites[i]);
			}

			for(i = 0; i != MAX_ACTORS; i++) {
				sprite_draw(&actor_sprites[i]);
			}

			wait_sync();

			show_sprite(1,80);
			sprite_prepare();

			j--;
		}

		sprite_init(&actor_sprites[34], 35, -32, -32, 0x00, 0, 0);

    	set_colors(2, saucer_pal);

		ai_spawn_enemy(152, 80, ENEMY_TYPE_SAUCER);

		player_dead = FALSE;
		while((!player_dead) && (saucers_cnt)){
			joy = read_joypad1();

			handle_player(player, joy);

			ai_handle();
			shot_move();

			for(i = 1; i != MAX_ACTORS; i++) {
				sprite_move(&actor_sprites[i]);
			}

			for(i = 0; i != MAX_ACTORS; i++) {
				sprite_draw(&actor_sprites[i]);
			}

			shot_draw();

			wait_sync();

			show_sprite(1,80);
			sprite_prepare();

			gotoxy(1, 28);
			cputs("SCORE: ");
			write_ulong(score, 8);
			cputs("  LEVEL: ");
			write_uint(level, 3);
			gotoxy(30, 28);
			cputs("LIVES: ");
			write_uint(lives, 3);
		}

		if(player_dead){
			for(j = 30; j; j--){
				for(i = 1; i != MAX_ACTORS; i++) {
					sprite_move(&actor_sprites[i]);
					sprite_draw(&actor_sprites[i]);
				}

				ai_handle();

				wait_sync();
				show_sprite(1,80);
				sprite_prepare();
			}
			lives--;
		}
	}

	if(lives) {
		score += 100 + level*50;
		level++;
	}
}

void handle_player(player, joy)
register sprite_rec *player;
register uint joy;
{
	/* Acceleration */
	player->xaccel = 0x00;
	if(joy & JOY_LEFT){
		player->xaccel -= 0x800;
	} else if(joy & JOY_RIGHT){
		player->xaccel += 0x800;
	}

	/* Handles shooting */
	if(joy & (BUTTON_A|BUTTON_C)) {
		if(saucers_deployed && (!shot_dly)) {
			shot_shoot(player->posx+8, player->posy+8, 
		               12, cur_angle+64, 0x640, BUILD_ATTR(0,0,0,0)|(240+5), 1);
			set_vol(1,0x0F);
			start_sound(1,shot2);
			shot_dly = 15;
		}
	}

	if(shot_dly) {
		if(shot_dly != 1) {
			shot_dly--;
		} else {
			if(!(joy & (BUTTON_A|BUTTON_C))) {
				shot_dly = 0;
			}
		}
	}

	sprite_move(player);

	/* Checks collision with borders */
	if(player->posx < 0) {
		player->posx = 0;
		player->xaccel = -player->xaccel;
 	} else if(player->posx > (320-24)) {
		player->posx = (320-24);
		player->xaccel = -player->xaccel;
	}

	/* Checks collision with shots */
	if(shot_at(player->posx+4, player->posy+4, player->posx+20, player->posy+20, 1)) {
		kill_player();
	}
}

void kill_player()
{
	register uint i, j;
	register uint ai_num;

	for (i = 16; i; i--){
		ai_num = ai_spawn_enemy(player->posx+4, player->posy+4, ENEMY_TYPE_KABOOM);
		if(ai_num){
			for(j = 3; j; j--){
				ai_accelerate(&ai_recs[ai_num-ENEMY_FIRST], i << 4, j << 9);
			}
		}
	}
	
	play_GFS_sound("BOOM5.SND");

	player_dead = TRUE;
}
