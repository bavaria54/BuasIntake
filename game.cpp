#include "game.h"
#include "player.h"
#include "enemy.h"
#include "projectile_fireball.h"
#include "pickup_experience.h"
#include "surface.h"
#include "algorithm" //Quick maffs
#include "template.h"
#include <iostream> 
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

namespace Tmpl8
{
	bool debug = true;

	int gamePaused = 1;

	Player player; //Creates player variable player

	const int maxProjectiles = 25; //Projectiles array
	Projectile_Fireball projectile[maxProjectiles]; //Projectiles array
	int spawn_fireball = 0;
	float closest_enemy_direction = 0;

	const int enemyAmount = 25;		//Enemy array max amount, so 100 = max 100 enemies.
	Enemy enemy[enemyAmount];		//Enemy array
	int enemyAliveCount = 0;		//Enemies that are alive will be counted

	int waveNumber = 0;				//Which wave of enemies we're on!
	int enemiesSpawned = 0;
	int currentWaveTime = 30 * 165;	//How many seconds for the wave to start, 30 * 165 cuz my screen is 165hz, so 30 seconds.
	int waveTime = 0;				//How many seconds for the initial wave to start, 5 * 165 cuz my screen is 165hz, so 5 seconds.
	int spawnDelay = 10;			//Not every enemy needs to spawn instantly, some delay between em.

	float nearestEnemyDist = 9999;
	int nearestEnemyID = 0;

	Pickup_Exp pickup_exp[enemyAmount * 3]; //I need to find a more efficient way to do these things...
	int pickupExpCount = 0;			//Pickups on screen will be counted

	int keyUp = 0;					//Checks if certain keypress is released

	Sprite sprite_player(new Surface("assets/player_wizard.png"), 2);
	Sprite sprite_enemy(new Surface("assets/enemy_wizard.png"), 2);
	Sprite sprite_pickup_exp(new Surface("assets/BlueExpRupee.tga"), 12);
	Sprite sprite_projectile_fireball(new Surface("assets/fireball.tga"), 1);
	Sprite sprite_startscreen(new Surface("assets/start_screen.tga"), 1);
	Sprite sprite_levelupscreen(new Surface("assets/levelup_screen.tga"), 1);
	Sprite sprite_upgrade_damage(new Surface("assets/upgrade_damage.png"), 2);
	Sprite sprite_upgrade_healthspeed(new Surface("assets/upgrade_healthspeed.png"), 2);
	Sprite sprite_upgrade_attackspeed(new Surface("assets/upgrade_attackspeed.png"), 2);

	//Handles animation speed for xp rupees
	int spriteSpeed = 8; //Higher is slower, every 8 frames, go next.
	int spriteCycle = 0;
	int spriteNum = 0;

	const int player_drawx = (player.x - (sprite_player.GetWidth() / 2));	//if I change my sprite size, it automatically centers it to the screen
	const int player_drawy = (player.y - (sprite_player.GetHeight() / 2));	//if I change my sprite size, it automatically centers it to the screen

	void Game::Init()
	{
	}

	void Game::Shutdown()
	{
	}

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Main application tick function
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Game::Tick(float deltaTime)
	{
		screen->Clear(0xaaa0a0); // clear the graphics window

		// gameloop
		if (gamePaused == 0)
		{
			// Makes the sprite of XP animate
			if (spriteCycle++ > spriteSpeed) { spriteNum++;  spriteCycle = 0; }
			if (spriteNum > 11) { spriteNum = 0; }
			sprite_pickup_exp.SetFrame(spriteNum);

			// Handle Waves spawning and whatnot.
			if (waveTime > 0) { waveTime--; }
			if ((waveTime == 0 && spawnDelay == 0) && (enemiesSpawned == waveNumber * 10)) { waveNumber++; waveTime = 1 * 165; }
			if (spawnDelay > 0) { spawnDelay--; }

			// Handle player attacks
			player.attackFrame += player.attackSpeed;
			if (debug == true)
			{
				screen->Box(enemy[nearestEnemyID].x, enemy[nearestEnemyID].y, enemy[nearestEnemyID].x + 64, enemy[nearestEnemyID].y + 64, 0x000000); //Draws a box for nearest target
				screen->Bar(player_drawx, player_drawy - 26, player_drawx + (player.attackInterval - player.attackFrame), player_drawy - 18, 0x00ffff);
			}

			// Handle player level up!
			if (player.experience >= player.level * 100)
			{
				gamePaused = 2;
			}

			if (GetAsyncKeyState(VK_RIGHT)) { sprite_player.SetFrame(0); } // Set Sprite to right
			if (GetAsyncKeyState(VK_LEFT)) { sprite_player.SetFrame(1); } // Set Sprite to left

			if (player.isDead())
			{
				player.Refresh(); //refresh the player stats

				// refresh the enemies
				for (int i = 0; i < enemyAmount; i++)
				{
					enemy[i].alive = false;
					enemyAliveCount = 0;		//Enemies that are alive will be counted
					waveNumber = 0;				//Which number wave of enemies we're on!
					enemiesSpawned = 0;
					waveTime = 1 * 165;			//How many seconds for the wave to start, 30 * 165 cuz my screen is 165hz, so 30 seconds.
					spawnDelay = 50;			//Not every enemy needs to spawn instantly, some delay between em.
				}
				// refresh the XP
				for (int j = 0; j < enemyAmount * 3; j++)
				{
					pickup_exp[j].alive = false;
				}
			}

			int redness = round(255 * player.healthpercentage);
			int greenness = round(255 * player.healthpercentage);

			// Healtbar from Green to yellow to red.
			if (player.healthpercentage > 0.5) {
				player.healthcolor = ((-redness - 1) * 2 << 16) + (255 << 8); //green to yellow
			}
			else {
				player.healthcolor = (255 << 16) + ((greenness - 1) * 2 << 8); //yellow to red
			}
			// Draws Player HP bar and player itself
			sprite_player.Draw(screen, player_drawx, player_drawy);	// Draw player 
			player.healthpercentage = player.currenthealth / player.maxhealth;	// Update player HP bar percentage && Color
			screen->Bar(player_drawx, player_drawy - 16, (player_drawx + sprite_player.GetWidth() * player.healthpercentage), player_drawy - 8, player.healthcolor);	// Healthbar percentage = length
			screen->Box(player_drawx - 1, player_drawy - 17, player_drawx + sprite_player.GetWidth() + 1, player_drawy - 7, 0); // Black box around HP bar


			//ENEMY LOOP============================================================================================================================================================================
			for (int i = 0; i < enemyAmount; i++)
			{
				if (enemy[i].isAlive())
				{
					for (int j = 0; j < enemyAmount; j++) //Checks distance between all enemies entities, includes itself for now
					{
						if (i != j)
						{
							enemy[i].Collision(enemy[j]);
						}
					}

					if (enemy[i].health < 0) // When enemy has no health, it kills the enemy, and spawns XP at his location.
					{
						enemy[i].isKilled();
						int xpDropped = false;
						for (int j = 0; j < enemyAmount * 3; j++)
						{
							if (pickup_exp[j].alive == false && xpDropped == false)
							{
								pickup_exp[j].x = enemy[i].x + 24;
								pickup_exp[j].y = enemy[i].y + 24;
								pickup_exp[j].alive = true;
								xpDropped = true;
							}
						}
					}

					if (enemy[nearestEnemyID].alive == false)
					{
						nearestEnemyID = i;
					}

					if (enemy[i].GetDistance(player_drawx, player_drawy) < enemy[nearestEnemyID].GetDistance(player_drawx, player_drawy)) // Marks nearest enemy as target for projectiles
					{
						nearestEnemyDist = enemy[i].GetDistance(player_drawx, player_drawy);
						nearestEnemyID = i;
					}


					// Checks where player is and moves towards it. (center of screen)
					enemy[i].Move(player_drawx, player_drawy);

					// Checks wether enemy is too far outside of screen, then resets position
					if (enemy[i].x < -400) { enemy[i].alive = false; enemiesSpawned--; }					//Too far left
					if (enemy[i].x > (ScreenWidth + 400)) { enemy[i].alive = false; enemiesSpawned--; }		//Too far right
					if (enemy[i].y < -432) { enemy[i].alive = false; enemiesSpawned--; }					//Too far top
					if (enemy[i].y > (ScreenHeight + 450)) { enemy[i].alive = false; enemiesSpawned--; }	//Too far bottom

					//Damaging the player
					if (enemy[i].GetDistance(player_drawx, player_drawy) < 48)
					{
						player.currenthealth -= enemy[i].damage;
					}

					//Killing enemy
					if (enemy[i].health <= 0)
					{
						enemy[i].isKilled();
					}

					// If player tries to move, enemies move the opposite way for the illusion of movement. Also makes it so if I move diagonally, I dont move as fast.
					if (GetAsyncKeyState(VK_RIGHT) && !(GetAsyncKeyState(VK_UP)) && !(GetAsyncKeyState(VK_DOWN)))
					{
						enemy[i].x -= player.speed;
					}
					else if (GetAsyncKeyState(VK_RIGHT))
					{
						enemy[i].x -= sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_LEFT) && !(GetAsyncKeyState(VK_UP)) && !(GetAsyncKeyState(VK_DOWN)))
					{
						enemy[i].x += player.speed;
					}
					else if (GetAsyncKeyState(VK_LEFT))
					{
						enemy[i].x += sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_DOWN) && !(GetAsyncKeyState(VK_LEFT)) && !(GetAsyncKeyState(VK_RIGHT)))
					{
						enemy[i].y -= player.speed;
					}
					else if (GetAsyncKeyState(VK_DOWN))
					{
						enemy[i].y -= sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_UP) && !(GetAsyncKeyState(VK_LEFT)) && !(GetAsyncKeyState(VK_RIGHT)))
					{
						enemy[i].y += player.speed;
					}
					else if (GetAsyncKeyState(VK_UP))
					{
						enemy[i].y += sqrtf(player.speed * player.speed) / 1.6;
					}

					// Draw enemy sprites either facing left or right
					if (enemy[i].x > player_drawx) {
						sprite_enemy.SetFrame(1);
						sprite_enemy.Draw(screen, enemy[i].x, enemy[i].y);	// Draws entity
					}
					else {
						sprite_enemy.SetFrame(0);
						sprite_enemy.Draw(screen, enemy[i].x, enemy[i].y);	// Draws entity
					}
				}
				else //If enemy is NOT alive
				{
					if ((enemiesSpawned < waveNumber * 10) && spawnDelay <= 0) //If there are enemies needing 2 be spawned.
					{
						enemy[i].Refresh();
						spawnDelay = 200 + IRand(200);
						enemiesSpawned++;
						//Sets Alive to true and repositions the enemy
					}
				}
			}

			//PROJECTILES LOOP ====================================================================================================================================================================
			for (int i = 0; i < maxProjectiles; i++)
			{
				if (projectile[i].alive == true)
				{
					sprite_projectile_fireball.Draw(screen, projectile[i].x - 8, projectile[i].y - 8);	// Draws entity
					projectile[i].Move();

					if (projectile[i].x > ScreenWidth + 16 || projectile[i].x < -16 || projectile[i].y < 16 || projectile[i].y > ScreenHeight + 16) // Check outside screen
					{
						projectile[i].alive = false;
					}

					for (int j = 0; j < enemyAmount; j++)
					{
						if (projectile[i].Collision(enemy[j].x, enemy[j].y) && enemy[j].alive == true) // If I collide with an enemy, then deal damage and fizzle
						{
							enemy[j].health -= projectile[i].damage * player.damageMultiplier;
							projectile[i].alive = false;
						}
					}

					// If player tries to move, projectiles move the opposite way for the illusion of movement. Also makes it so if I move diagonally, I dont move as fast.
					if (GetAsyncKeyState(VK_RIGHT) && !(GetAsyncKeyState(VK_UP)) && !(GetAsyncKeyState(VK_DOWN)))
					{
						projectile[i].x -= player.speed;
					}
					else if (GetAsyncKeyState(VK_RIGHT))
					{
						projectile[i].x -= sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_LEFT) && !(GetAsyncKeyState(VK_UP)) && !(GetAsyncKeyState(VK_DOWN)))
					{
						projectile[i].x += player.speed;
					}
					else if (GetAsyncKeyState(VK_LEFT))
					{
						projectile[i].x += sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_DOWN) && !(GetAsyncKeyState(VK_LEFT)) && !(GetAsyncKeyState(VK_RIGHT)))
					{
						projectile[i].y -= player.speed;
					}
					else if (GetAsyncKeyState(VK_DOWN))
					{
						projectile[i].y -= sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_UP) && !(GetAsyncKeyState(VK_LEFT)) && !(GetAsyncKeyState(VK_RIGHT)))
					{
						projectile[i].y += player.speed;
					}
					else if (GetAsyncKeyState(VK_UP))
					{
						projectile[i].y += sqrtf(player.speed * player.speed) / 1.6;
					}
				}
				else //If fireball is NOT alive
				{
					// Create Fireballs
					if (player.attackFrame >= player.attackInterval && enemiesSpawned > 0)
					{
						projectile[i].x = ScreenWidth / 2;
						projectile[i].y = ScreenHeight / 2;
						projectile[i].SetDirection(enemy[nearestEnemyID].x, enemy[nearestEnemyID].y);
						player.attackFrame = 0;
						projectile[i].alive = true;
					}
				}
			}

			//EXP LOOP =============================================================================================================================================================================
			for (int i = 0; i < enemyAmount * 3; i++)
			{
				if (pickup_exp[i].alive == true)
				{
					sprite_pickup_exp.Draw(screen, pickup_exp[i].x, pickup_exp[i].y);	// Draws entity

					if (pickup_exp[i].GetDistance() < player.pickupRange) //If player ever got close, start moving the xp to the player
					{
						pickup_exp[i].magnet = true;
						if (pickup_exp[i].GetDistance() < 16) {
							pickup_exp[i].isPickedUp(); player.experience += pickup_exp[i].xpValue; pickup_exp[i].magnet = false;
						} //If player touches xp, it picks the XP up.
					}

					if (pickup_exp[i].magnet == true) // Makes EXP fly towards player
					{
						pickup_exp[i].isMagneted();
					}

					//Move the XP according to input
					if (GetAsyncKeyState(VK_RIGHT) && !(GetAsyncKeyState(VK_UP)) && !(GetAsyncKeyState(VK_DOWN)))
					{
						pickup_exp[i].x -= player.speed;
					}
					else if (GetAsyncKeyState(VK_RIGHT))
					{
						pickup_exp[i].x -= sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_LEFT) && !(GetAsyncKeyState(VK_UP)) && !(GetAsyncKeyState(VK_DOWN)))
					{
						pickup_exp[i].x += player.speed;
					}
					else if (GetAsyncKeyState(VK_LEFT))
					{
						pickup_exp[i].x += sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_DOWN) && !(GetAsyncKeyState(VK_LEFT)) && !(GetAsyncKeyState(VK_RIGHT)))
					{
						pickup_exp[i].y -= player.speed;
					}
					else if (GetAsyncKeyState(VK_DOWN))
					{
						pickup_exp[i].y -= sqrtf(player.speed * player.speed) / 1.6;
					}
					if (GetAsyncKeyState(VK_UP) && !(GetAsyncKeyState(VK_LEFT)) && !(GetAsyncKeyState(VK_RIGHT)))
					{
						pickup_exp[i].y += player.speed;
					}
					else if (GetAsyncKeyState(VK_UP))
					{
						pickup_exp[i].y += sqrtf(player.speed * player.speed) / 1.6;
					}
				}


				if (debug == true)
				{
					if (GetAsyncKeyState(VK_SPACE) && keyUp < 15 && pickup_exp[i].alive == false) {
						pickup_exp[i].alive = true;
						pickup_exp[i].magnet = false;
						pickup_exp[i].x = IRand(ScreenWidth);
						pickup_exp[i].y = IRand(ScreenHeight);
						keyUp++;
					}
					else
					{
						if (!GetAsyncKeyState(VK_SPACE))
						{
							keyUp = 0;
						}
					}
				}
			}


			//Draw experience bar on top of screen.
			if (player.experience > 0 && player.level > 0)
			{
				screen->Bar(1, 1, (ScreenWidth - 2) * ((player.experience * 100) / (player.level * 10000)), 16, 0x0077ee); //Draw XP bar, blueish.
			}
			screen->Box(0, 0, ScreenWidth - 1, 17, 0x000000); //Draw XP bar black outline
		}
		//startscreen pause
		if (gamePaused == 1)
		{
			sprite_startscreen.Draw(screen, (ScreenWidth / 2) - (sprite_startscreen.GetWidth() / 2), (ScreenHeight / 2) - (sprite_startscreen.GetHeight() / 2));
			if (GetAsyncKeyState(0x0001) || GetAsyncKeyState(0x0002)) // if either left or right mousebutton is clicked
			{
				gamePaused = 0;
			}
		}
		//levelup pause
		if (gamePaused == 2)
		{
			sprite_levelupscreen.Draw(screen, (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2), (ScreenHeight / 2) - (sprite_levelupscreen.GetHeight() / 2));

			int hitbox1X = (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2) + 2;
			int hitbox1XX=(hitbox1X + sprite_upgrade_attackspeed.GetWidth());
			int hitbox1Y = (ScreenHeight / 2) - (sprite_levelupscreen.GetHeight() / 2) + 98;
			int hitbox1YY= (hitbox1Y + sprite_upgrade_attackspeed.GetHeight());
			int hitbox2X = (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2) + 112;
			int hitbox2XX= (hitbox2X + sprite_upgrade_attackspeed.GetWidth());
			int hitbox2Y = (ScreenHeight / 2) - (sprite_levelupscreen.GetHeight() / 2) + 98;
			int hitbox2YY = (hitbox2Y + sprite_upgrade_attackspeed.GetHeight());
			int hitbox3X = (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2) + 221;
			int hitbox3XX= (hitbox3X + sprite_upgrade_attackspeed.GetWidth());
			int hitbox3Y = (ScreenHeight / 2) - (sprite_levelupscreen.GetHeight() / 2) + 98;
			int hitbox3YY = (hitbox3Y + sprite_upgrade_attackspeed.GetHeight());

			if (debug == true) // Draw Click Hitbox
			{
				screen->Box(hitbox1X, hitbox1Y, hitbox1XX, hitbox1YY, 0);
				screen->Box(hitbox2X, hitbox2Y, hitbox2XX, hitbox2YY, 0);
				screen->Box(hitbox3X, hitbox3Y, hitbox3XX, hitbox3YY, 0);

				screen->Line(mousex, 0, mousex, ScreenHeight, 0xff0000); //line mouse
				screen->Line(0, mousey, ScreenWidth, mousey, 0xff0000); //line mouse
			}

			sprite_upgrade_attackspeed.Draw(screen, hitbox1X, hitbox1Y);
			sprite_upgrade_damage.Draw(screen, hitbox2X, hitbox2Y);
			sprite_upgrade_healthspeed.Draw(screen, hitbox3X, hitbox3Y);


			if ((mousex > hitbox1X) && (mousex < hitbox1XX) && (mousey > hitbox1Y) && (mousey < hitbox1YY))
			{	
				sprite_upgrade_attackspeed.SetFrame(1);
				if((GetKeyState(VK_LBUTTON) & 0x8000) != 0) // if left is clicked in certain hitbox
				{
					player.LevelUp(1);
					gamePaused = 0;
				}
			}
			else { sprite_upgrade_attackspeed.SetFrame(0); }


			if ((mousex > hitbox2X) && (mousex < hitbox2XX) && (mousey > hitbox2Y) && (mousey < hitbox2YY))
			{
				sprite_upgrade_damage.SetFrame(1);
				if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0) // if left is clicked in certain hitbox
				{
					sprite_upgrade_damage.Draw(screen, hitbox1X, hitbox1Y);
					player.LevelUp(2);
					gamePaused = 0;
				}
			}
			else { sprite_upgrade_damage.SetFrame(0); }


			if ((mousex > hitbox3X) && (mousex < hitbox3XX) && (mousey > hitbox3Y) && (mousey < hitbox3YY))
			{
				sprite_upgrade_healthspeed.SetFrame(1);
				if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0) // if left is clicked in certain hitbox
				{
					sprite_upgrade_healthspeed.Draw(screen, hitbox1X, hitbox1Y);
					player.LevelUp(3);
					gamePaused = 0;
				}
			}
			else { sprite_upgrade_healthspeed.SetFrame(0); }
		}
	}
}