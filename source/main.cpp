#include <citro2d.h>
#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <string>

class Object {
public:
	float x;
	float y;
	float speed_x;
	float speed_y;
	int size;
	u32 colour;

	void drawyourself()
	{
		C2D_DrawCircleSolid(x, y, 0, size, colour);
	}

	void updateposition()
	{
		x += speed_x;
		y += speed_y;
	}

	Object(float x, float y, float speed_x, float speed_y, int size, u32 colour)
	{
		this->x = x;
		this->y = y;
		this->speed_x = speed_x;
		this->speed_y = speed_y;
		this->size = size;
		this->colour = colour;
	}

	

	~Object() {}

};

class Projectile : public Object
{
public:
	Projectile(float x, float y, float speed_x, float speed_y, int size, u32 colour, float direction) : Object(x, y, speed_x, speed_y, size, colour) {
		this->direction = direction;
	}

	float direction = 0;
	//baseline coordinates for triangle part. 0,0 is middle of bottom of triangle
	float triangle_x[3] = { 0, -size, size };
	float triangle_y[3] = { -10 * size, 0, 0 };

	void draw_circlepart()
	{
		C2D_DrawCircleSolid(x, y, 0, size, colour);
	}

	void draw_trianglepart()
	{
		float rotate_x[3], rotate_y[3];

		//rotate the coordinates
		for(int i = 0; i < 3; i++)
		{
			rotate_x[i] = triangle_x[i] * -sinf(direction) + triangle_y[i] * cosf(direction);
			rotate_y[i] = -(triangle_x[i] * cosf(direction) + triangle_y[i] * sinf(direction));
		}

		//translate to projectile loc
		for (int i = 0; i < 3; i++)
		{
			rotate_x[i] = rotate_x[i] + x;
			rotate_y[i] = rotate_y[i] + y;
		}

		//drawing triangle
		//C2D_DrawTriangle(
			//float x0, float y0, u32 clr0,
			//float x1, float y1, u32 clr1,
			//float x2, float y2, u32 clr2,
			//float depth);

		C2D_DrawTriangle(
			rotate_x[0], rotate_y[0], C2D_Color32(0x07, 0x03, 0x1A, 0x68),
			rotate_x[1], rotate_y[1], colour,
			rotate_x[2], rotate_y[2], colour,
			0.0);

	}

	~Projectile() {}

};

class Enemy : public Object
{
public:
	//check wether hit by projectile
	bool is_hit = false;
	
	//counter for when it changes direction
	int direction_counter = 60;
	//flag used to keep track which mode the direction is going
	//0 = continue in same direction, 1 = going right, 2 = going left
	int changing_direction = 0;
	//factor in how sharp the turns are, default = 0.02
	float base_direction_factor = 0.02;
	float current_direction_factor = 0.02;

	//current direction the enemy is facing
	float direction = atan2(speed_y, speed_x);

	//how fast the enemies move
	float velocity_factor = 2.0;



	//float acceleration_x = 0;
	//float acceleration_y = 0;

	Enemy(float x, float y, float speed_x, float speed_y, int size, u32 colour) : Object(x, y, speed_x, speed_y, size, colour) {}

	~Enemy() {}

	void updatespeed()
	{
		//changing acceleration direction if counter hits 0
		if (direction_counter == 0)
		{
			direction_counter = rand() % 60 + 90;
			//change direction
			changing_direction = rand() % 3;
		}

		//change_direction 0 = continuing in the same direction

		//change_direction 1 = going right (looking in the direction of movement)

		//change_direction 2 = going left

		

		
		switch (changing_direction)
		{
		case 1:
			direction += current_direction_factor;
			break;
		case 2:
			direction -= current_direction_factor;
			break;
		default:
			break;
		}
		
		speed_x = cosf(direction) * velocity_factor;
		speed_y = sinf(direction) * velocity_factor;



		direction_counter--;
	}


};


class Player : public Object
{
public:
	Player(float x, float y, float speed_x, float speed_y, int size, u32 colour) : Object(x, y, speed_x, speed_y, size, colour) {}

	//removing speed as natural decay
	float flat_decay = 0.05;
	float relative_decay = 0.80;

	void updatespeed(float circlepad_x, float circlepad_y)
	{
		//circlepad default position varies a bit but ignoring the first 15 on all sides should do the trick
		//max position is about 160

		//first, x-direction
		if (circlepad_x > 15 || circlepad_x < -15)
		{
			speed_x += (circlepad_x / 160.0);
		}

		//second, y direction
		if (circlepad_y > 15 || circlepad_y < -15)
		{
			speed_y -= (circlepad_y / 160.0);
		}


		//x-direction
		if (speed_x > flat_decay) {
			speed_x = relative_decay * speed_x - flat_decay;
		}
		else if (speed_x < -flat_decay) {
			speed_x = relative_decay * speed_x + flat_decay;
		}
		else {
			speed_x = 0;
		}

		//y-direction
		if (speed_y > flat_decay) {
			speed_y = relative_decay * speed_y - flat_decay;
		}
		else if (speed_y < -flat_decay) {
			speed_y = relative_decay * speed_y + flat_decay;
		}
		else {
			speed_y = 0;
		}
	}


	void wallcollision()
	{
		if (x <= size && speed_x < 0) {
			speed_x = 0;
			speed_y *= relative_decay;
			x = size;
		}
		else if (x >= (400-size) && speed_x > 0) {
			speed_x = 0;
			speed_y *= relative_decay;
			x = (400-size);
		}

		if (y <= size && speed_y < 0) {
			speed_y = 0;
			speed_y *= relative_decay;
			y = size;
		}
		else if (y >= (240 - size) && speed_y > 0) {
			speed_y = 0;
			speed_y *= relative_decay;
			y = (240 - size);
		}
	}
};


bool iscircleinsidecircle(float c1x, float c1y, float radius1, float c2x, float c2y, float radius2)
{
	return sqrt((c1x - c2x) * (c1x - c2x) + (c1y - c2y) * (c1y - c2y)) < (radius1 + radius2-2);
}

void spawnEnemy(std::vector<Enemy>& vec, int size, u32 colour, Player p)
{
	
	//check in which quadrant the player is, 0 = topleft, 1 = topright, 2 = bottomright, 3 = bottomleft
	int player_quadrant;
	if (p.y < 120)
	{
		if (p.x < 200) player_quadrant = 0;
		else player_quadrant = 1;
	}
	else
	{
		if (p.x >= 200) player_quadrant = 2;
		else player_quadrant = 3;
	}
	
	//screen_edge defines near which screen edge the enemy will spawn
	//spawns enemies based on on screens edges of each of the quadrants, 8 in total. 
	//These are numbered in a clockwise fashion starting from the left edge to the topleft quadrant as follows: 0, 4, 1, 5, 2, 6, 3 and 7
	
	//random generation of a screen_edge that is not part of the player quadrant.
	int screen_edge = player_quadrant;
	while (screen_edge == player_quadrant || screen_edge == (player_quadrant + 4))
	{
		screen_edge = rand() % 8;
	}
	
	float x_offset = 0;
	float y_offset = 0;
	//spawns enemies just off-screen then makes them fly in
	

	
	switch (screen_edge)
	{
	case 0:
		x_offset = -size;
		y_offset = rand() % 120;
		break;
	case 1:
		x_offset = rand() % 200 + 200;
		y_offset = -size;
		break;
	case 2:
		x_offset = size + 400;
		y_offset = rand() % 120 + 120;
		break;
	case 3:
		x_offset = rand() % 200;
		y_offset = size + 240;
		break;
	case 4:
		x_offset = rand() % 200;
		y_offset = -size;
		break;
	case 5:
		x_offset = size + 400;
		y_offset = rand() % 120;
		break;
	case 6:
		x_offset = rand() % 200 + 200;
		y_offset = size + 240;
		break;
	case 7:
		x_offset = -size;
		y_offset = rand() % 120 + 120;
		break;
	}

	//setting begin speed to go in the direction of middle of screen
	float direction_begin = atan2f((120 - y_offset), (200 - x_offset));

	
	vec.push_back({ x_offset, y_offset, cosf(direction_begin) * 2.0, sinf(direction_begin) * 2.0, size, colour });


	//vec.push_back({ x_offset, y_offset, 5, 5, size, colour });
}




//-------------------------------------------------------------
int main(int argc, char* argv[]) {
	//-------------------------------------------------------------


		//init libs
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	//consoleInit(GFX_BOTTOM, NULL);

	//create screens
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	static C2D_SpriteSheet spriteSheet;
	// Load graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);
	
	std::vector<C2D_Sprite> vsprites;

	
	for (size_t i = 0; i < C2D_SpriteSheetCount(spriteSheet); i++)
	{
		C2D_Sprite testsprite;
		C2D_SpriteFromSheet(&testsprite, spriteSheet, i);
		vsprites.push_back(testsprite);
	}
	
	C2D_SpriteSetCenter(&vsprites[0], 0.5, 0.5); 
	//setting gameover sprite coordinates
	C2D_SpriteSetPos(&vsprites[2], 59, 20);
	

	//create colors
	u32 clrprojectile = C2D_Color32(244, 246, 255, 0xFF);
	u32 clrBackground = C2D_Color32(0x07, 0x03, 0x1A, 0x68);
	u32 clrenemy = C2D_Color32(0x4F, 0x8A, 0x8B, 0xFF);
	u32 clrPlayer = C2D_Color32(0xFB, 0xD4, 0x6D, 0x68);
	u32 clrreticle = C2D_Color32(255, 20, 24, 255);

	//create variables

	float old_touch_x = 200;
	float old_touch_y = 120;
	float projectile_speed = 8;
	int projectile_size = 3;
	bool death_flag = false;
	float direction = 0;

	//the amount of frames inbetween the firing of each projectile while L is held
	int projectile_interval = 8;

	//starting amount of enemies
	int n_enemies = 1;


	//setup vector for holding Projectiles;
	std::vector<Projectile> vProjectile;

	//setup vector for holding Enemies
	std::vector<Enemy> vEnemy;


	//creating text buffers
	C2D_TextBuf staticbuf = C2D_TextBufNew(4096);
	C2D_TextBuf dynamicbuf = C2D_TextBufNew(4096);
	//creating text
	C2D_Text tutorial_txt[3];
	C2D_Text score_txt;
	C2D_Text highscore_txt;

	// Parse the tutorial text strings
	C2D_TextParse(&tutorial_txt[0], staticbuf, "This is you. ->\n\nUse the circlepad to move.");
	C2D_TextParse(&tutorial_txt[1], staticbuf, "This is an enemy. ->\n\nKill it before it kills you!");
	C2D_TextParse(&tutorial_txt[2], staticbuf, "Use L to shoot and the touch screen to aim.\n      Touch the touch screen to start");

	// Optimize the static text strings
	C2D_TextOptimize(&tutorial_txt[0]);
	C2D_TextOptimize(&tutorial_txt[1]); 
	C2D_TextOptimize(&tutorial_txt[2]);

	char chighscore[10];

	FILE* phighscore = fopen("romfs:/score.txt", "r");
	if (phighscore)
	{
		
		if (fgets(chighscore, 10, phighscore))
		{
			// Clear the static text buffer
			//C2D_TextBufClear(staticbuf);

			//adding highscore to static buffer
			C2D_TextParse(&highscore_txt, staticbuf, chighscore);
			C2D_TextOptimize(&highscore_txt);
		}
		fclose(phighscore);
	}

	//Titlescreen loop

	while (true)
	{
		hidScanInput();

		// Respond to user input
		u32 kDown = hidKeysDown();

		if (kDown & KEY_A) {
			break;
		}

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, clrBackground);
		C2D_SceneBegin(top);

		
		C2D_DrawSprite(&vsprites[1]);

		C2D_DrawText(&highscore_txt, C2D_AtBaseline | C2D_WithColor, 320.0f, 215.0f, 0.5f, 0.7f, 0.7f, clrreticle);

		C2D_TargetClear(bottom, clrBackground);
		C2D_SceneBegin(bottom);

		C3D_FrameEnd(0);
	}

	//set the starting position of the reticle
	C2D_SpriteSetPos(&vsprites[0], old_touch_x, old_touch_y);

	//tutorial screen loop

	while (true)
	{
		hidScanInput();

		// Respond to user input
		u32 kDown = hidKeysDown();

		if (kDown & KEY_L) {
			break;
		}

		touchPosition touch;

		//read the touch screen coordinates
		//touch screen coordinates are stored in touch.px and touch.py
		hidTouchRead(&touch);
		if (touch.px != 0 && touch.py != 0) break;

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, clrBackground);
		C2D_SceneBegin(top);

		C2D_DrawText(&tutorial_txt[0], C2D_AtBaseline | C2D_WithColor, 4.0f, 100.0f, 0.5f, 0.5f, 0.5f, clrprojectile);
		C2D_DrawText(&tutorial_txt[1], C2D_AtBaseline | C2D_WithColor, 180.0f, 50.0f, 0.5f, 0.5f, 0.5f, clrprojectile);
		C2D_DrawText(&tutorial_txt[2], C2D_AtBaseline | C2D_WithColor, 70.0f, 200.0f, 0.5f, 0.5f, 0.5f, clrprojectile);
		
		C2D_DrawCircleSolid(105, 95, 0, 10, clrPlayer);
		C2D_DrawCircleSolid(320, 45, 0, 10, clrenemy);
		C2D_DrawSprite(&vsprites[0]);

		C3D_FrameEnd(0);
	}


	// main game loop
	while (true)
	{

		bool exit_game = false;
		death_flag = false;

		int projectile_interval_counter = projectile_interval;

		//score system
		unsigned int player_score = 0;

		//spawning player character
		Player player(105, 95, 0, 0, 10, clrPlayer);

		//spawning first enemy

		vEnemy.push_back({ 320, 45, cosf(2.0) * 2.0, sinf(2.0) * 2.0, 10, clrenemy});

		//setting random seed
		srand(time(NULL));

	
		//game loop of the current run
		while (aptMainLoop())
		{
			hidScanInput();

			// Respond to user input
			u32 kDown = hidKeysDown();
			u32 kHeld = hidKeysHeld();
			if (kDown & KEY_START) {
				exit_game = true;
				break; //return to hbmenu
			}
			touchPosition touch;

			//read the touch screen coordinates
			//touch screen coordinates are stored in touch.px and touch.py
			hidTouchRead(&touch);

			//1.25 is for the difference between top screen width and bottom screen width (400 vs 320)
			float touch_x = 1.25 * touch.px;
			float touch_y = (float)touch.py;

			//checking is touch screen is touched, if not then changing touch coordinates to coordinates last frame
			if (touch_x == 0 && touch_y == 0)
			{
				touch_x = old_touch_x;
				touch_y = old_touch_y;
			}
			else
			{
				old_touch_x = touch_x;
				old_touch_y = touch_y;
			}

			//setting reticule to touch coordinates
			//C2D_SpriteSetPos(&sprite_reticle, touch_x, touch_y);
			C2D_SpriteSetPos(&vsprites[0], touch_x, touch_y);


			//analog inputs
			circlePosition pos;

			//Read the CirclePad position
			hidCircleRead(&pos);
			//circlepad position is stored in pos.dx and pos.dy

			player.updatespeed(pos.dx, pos.dy);
			player.wallcollision();
			player.updateposition();

			//checking if the right amount of enemies are present. spawning enemies if too few are present.
			//including when no enemies are present as in the beginning
			if (vEnemy.size() < n_enemies + player_score/10)
			{
				spawnEnemy(vEnemy, 10, clrenemy, player);
			}

			



			//create Projectiles if L is held

			if (kHeld & KEY_L)
			{
				if (projectile_interval_counter == projectile_interval)
				{

					//determining the direction between actor and pointer
					direction = atan2((-1.0 * (touch_y - player.y)), (touch_x - player.x));

					vProjectile.push_back({ player.x, player.y,
						cosf(direction) * projectile_speed,
						-sinf(direction) * projectile_speed, projectile_size, clrprojectile, direction});
					player.speed_x += 2.0 * -cosf(direction);
					player.speed_y += 2.0 * sinf(direction);
					projectile_interval_counter = 0;
				}
				else projectile_interval_counter++;
			}

			for (auto& a : vProjectile) {
				a.updateposition();
			}


			for (auto& a : vEnemy) {
				
				//updating enemies turning speed and movement speed based on player_score
				if (player_score > 10)
					a.current_direction_factor = (1.0 + ((player_score - 10.0) / 100.0)) * a.base_direction_factor;
				else a.current_direction_factor = a.base_direction_factor;

				a.updatespeed();
				a.updateposition();
				//checking if Enemy kills player
				if (iscircleinsidecircle(a.x, a.y, a.size, player.x, player.y, player.size))
				{
					death_flag = true;
				}

				//wrapping enemies around the screen if they dissapear off-screen

				if (a.x <= -(a.size)) a.x = a.size + 400.0;
				else if (a.x >= (400.0 + a.size)) a.x = (-1.0 * a.size);

				if (a.y <= -(a.size)) a.y = a.size + 240.0;
				else if (a.y >= 240.0 + a.size) a.y = (-1.0 * a.size);




				//checking if projectile hits enemy
				for (auto& b : vProjectile)
				{
					if (iscircleinsidecircle(a.x, a.y, a.size, b.x, b.y, b.size))
					{//if hit, sets bullet and enemy to off-screen
						b.x = -100;
						a.is_hit = true;
						player_score++;

						
					}
				}

			}
		

			//remove off-screen projectiles
			if (vProjectile.size() > 0)
			{
				auto i = remove_if(vProjectile.begin(), vProjectile.end(), [&](Projectile p) {return (p.x < -10 || p.x > 410 || p.y < -10 || p.y > 250); });
				if (i != vProjectile.end())
					vProjectile.erase(i);
			}

			//remove far off-screen enemies
			if (vEnemy.size() > 0)
			{
				auto i = remove_if(vEnemy.begin(), vEnemy.end(), [&](Enemy p) {return (p.is_hit); });
				if (i != vEnemy.end())
					vEnemy.erase(i);
			}

			//converting player_score to cstring for display
			std::string splayer_score = std::to_string(player_score);
			char const* pplayer_score = splayer_score.c_str();


			// Clear the dynamic text buffer
			C2D_TextBufClear(dynamicbuf);

			//adding player score to dynamic buffer
			C2D_TextParse(&score_txt, dynamicbuf, pplayer_score);
			C2D_TextOptimize(&score_txt);

			/*
			//print touch position
			printf("\x1b[1;1Hpointer (x,y): %.1f; %.1f", touch_x, touch_y);
			printf("\x1b[2;1Hplayer loc(x,y): %.1f, %.1f", player.x, player.y);
			printf("\x1b[3;1Hcircle_pad: %04d; %04d", pos.dx, pos.dy);
			printf("\x1b[4;1Hplayer_speed(x,y): %03.2f; %03.2f", player.speed_x, player.speed_y);
			printf("\x1b[5;1H%d Projectiles", vProjectile.size());
			printf("\x1b[6;1H%d Enemies", vEnemy.size());
			printf("\x1b[7;1H%d Points", player_score);
			printf("\x1b[8;1H%0.2f Direction, %0.2f cos, %0.2f sin", direction, cosf(direction), sinf(direction));
			printf("\x1b[9;1H%d highscore", atoi(chighscore));
			*/

			// Render the scene
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C2D_TargetClear(top, clrBackground);
			C2D_SceneBegin(top);

			for (auto& y : vProjectile) y.draw_trianglepart();

			//drawing pointer
			//C2D_DrawCircleSolid(
			//	touch_x, touch_y, 0, 10,
			//	clrpointer);

			for (auto& y : vProjectile) y.draw_circlepart();

			for (auto& z : vEnemy) z.drawyourself();

			player.drawyourself();

			//drawing reticle
			C2D_DrawSprite(&vsprites[0]);


			//only drawing score if player is alive
			if (!death_flag)
			{
				//drawing player_score
				C2D_DrawText(&score_txt, C2D_AtBaseline | C2D_WithColor, 16.0f, 224.0f, 0.5f, 1.5f, 1.5f, clrreticle);
			}
			//C2D_DrawText(&score_txt, 0, 100, 100, 1.0, 0.0, 0.0);
			//C2D_DrawText(&g_staticText[1], C2D_AtBaseline | C2D_WithColor, 16.0f, 210.0f, 0.5f, 0.5f, 0.75f, C2D_Color32f(1.0f, 0.0f, 0.0f, 1.0f));
			//C2D_DrawText(&g_staticText[0], 0, 8.0f, 8.0f, 0.5f, size, size);


			C3D_FrameEnd(0);
			if (death_flag) break;
		}

		//printf("\x1b[2J");

		if (player_score > atoi(chighscore))
		{
			//making making highscore equal to the playerscore
			
			//converting player_score to cstring for display
			std::string snewhighscore = std::to_string(player_score);
			char const* pnewhighscore = snewhighscore.c_str();


			// Clear the static text buffer
			C2D_TextBufClear(staticbuf);

			//adding player score to static buffer
			C2D_TextParse(&highscore_txt, staticbuf, pnewhighscore);
			C2D_TextOptimize(&highscore_txt);

			//updating highscore file
			FILE* f = fopen("romfs:/score.txt", "w");
			if (f)
			{

				fputs(pnewhighscore, phighscore);
				fclose(f);
			}
		}


		while (true)
		{
			hidScanInput();

			// Respond to user input
			u32 kDown = hidKeysDown();

			if (kDown & KEY_A)
			{
				
				vEnemy.erase(vEnemy.begin(), vEnemy.end());
				vProjectile.erase(vProjectile.begin(), vProjectile.end());
				break; //restart
			}

			if (kDown & KEY_START) {
				exit_game = true;
			}
			//printf("\x1b[1;1HYou have just died! \npress A to restart the game\nor press Start to return to hbmenu");
			//printf("\x1b[4;1HYou earned %d Points", player_score);

			// Render the scene
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C2D_TargetClear(top, clrBackground);
			C2D_SceneBegin(top);


			C2D_DrawSprite(&vsprites[2]);

			//drawing player_score
			C2D_DrawText(&score_txt, C2D_AtBaseline | C2D_WithColor, 225.0f, 123.0f, 0.5f, 1.0f, 1.0f, clrreticle);

			C2D_DrawText(&highscore_txt, C2D_AtBaseline | C2D_WithColor, 225.0f, 155.0f, 0.5f, 1.0f, 1.0f, clrreticle);

			C3D_FrameEnd(0);

			if (exit_game) break;
		}

		if (exit_game) break;

			
	}
	//De-init buffers
	C2D_TextBufDelete(staticbuf);
	C2D_TextBufDelete(dynamicbuf);


	// De-init libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}