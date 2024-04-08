#include "game.h"
#include "player.h"
#include "enemy.h"
#include "projectile_fireball.h"
#include "pickup_experience.h"
#include "surface.h"
#include <algorithm> //Quick maffs
#include "template.h"
#include <iostream> 
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <format>
#include <chrono>
#include <thread>


namespace Tmpl8
{
	bool debug = false; // if true it shows some extra variables and some extra visuals.

	int gamePaused = 1;
	const int maxFPS = 300;
	float elapsedTicks = 0;
	int collectablesCollected = 0;
	int enemiesDefeated = 0;
	float timeSurvived = 0;

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
	int currentWaveTime = 30 * maxFPS;	//How many seconds for the wave to start, 30 * FPS
	float waveTime = 0;					//How many seconds for the initial wave to start, 5 * 165 cuz my screen is 165hz, so 5 seconds.
	float spawnDelay = 0;					//Not every enemy needs to spawn instantly, some delay between em.

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
	Sprite sprite_deathscreen(new Surface("assets/death_screen.tga"), 1);
	Sprite sprite_levelupscreen(new Surface("assets/levelup_screen.tga"), 1);
	Sprite sprite_upgrade_damage(new Surface("assets/upgrade_damage.png"), 2);
	Sprite sprite_upgrade_healthspeed(new Surface("assets/upgrade_healthspeed.png"), 2);
	Sprite sprite_upgrade_attackspeed(new Surface("assets/upgrade_attackspeed.png"), 2);
	Sprite sprite_cursor(new Surface("assets/cursor.png"), 1);


	//Handles animation speed for xp rupees
	int spriteSpeed = 8; //Higher is slower, every 8 frames, go next.
	float spriteCycle = 0;
	int spriteNum = 0;

	//This is for a more average FPS
	float fpsAverage = 0.0f;
	float fpsAverageCalc = 0.0f;
	int fpsAveragePrecision = 300;
	int fpsAverageTiming = 0;

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
				if (spriteCycle > spriteSpeed) { spriteNum++;  spriteCycle = 0; }
				spriteCycle += 1 * deltaTime / 5;
				if (spriteNum > 11) { spriteNum = 0; }
				sprite_pickup_exp.SetFrame(spriteNum);

				// Handle Waves spawning
				if (waveTime > 0) { waveTime -= 1*deltaTime / 5; }
				if (waveTime <= 0 && spawnDelay <= 0) { waveNumber++; waveTime = 30 * 165; }
				if (spawnDelay > 0) { spawnDelay -= 1*deltaTime/5; }

				// Handle player attacks
				player.attackFrame += player.attackSpeed * deltaTime / 5;

				// Handle player level up!
				if (player.experience >= player.level * 100)
				{
					gamePaused = 2;
				}

				if (GetAsyncKeyState(VK_RIGHT)) { sprite_player.SetFrame(0); } // Set Sprite to right
				if (GetAsyncKeyState(VK_LEFT)) { sprite_player.SetFrame(1); } // Set Sprite to left

				timeSurvived += deltaTime;

				if (player.isDead())
				{
					gamePaused = 3;
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
						// Collision checking
						for (int j = 0; j < enemyAmount; j++)
						{
							if (i != j)
							{
								enemy[i].Collision(enemy[j]);
							}
						}

						// When enemy has no health, it kills the enemy, and spawns XP at his location.
						if (enemy[i].health < 0)
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
									enemiesDefeated++;
								}
							}
						}

						// Draw damage number above enemy when damaged
						if (enemy[i].recentlyDamaged > 0)
						{
							screen->Print(std::format("{}", enemy[i].recentDamage), enemy[i].x + 5, (enemy[i].y - 32) + (enemy[i].recentlyDamaged / 20), 0x000000, 4);
							enemy[i].recentlyDamaged -= 1 * deltaTime / 5;
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
						enemy[i].Move(player_drawx, player_drawy, deltaTime);

						// Checks wether enemy is too far outside of screen, then resets position
						if (enemy[i].x < -400) { enemy[i].alive = false; enemiesSpawned--; }					//Too far left
						if (enemy[i].x > (ScreenWidth + 400)) { enemy[i].alive = false; enemiesSpawned--; }		//Too far right
						if (enemy[i].y < -432) { enemy[i].alive = false; enemiesSpawned--; }					//Too far top
						if (enemy[i].y > (ScreenHeight + 450)) { enemy[i].alive = false; enemiesSpawned--; }	//Too far bottom

						//Damaging the player
						if (enemy[i].GetDistance(player_drawx, player_drawy) < 48)
						{
							player.currenthealth -= enemy[i].damage * deltaTime / 5;
						}

						//Killing enemy
						if (enemy[i].health <= 0)
						{
							enemy[i].isKilled();
						}

						// If player tries to move, enemies move the opposite way for the illusion of movement.
						if (GetAsyncKeyState(VK_RIGHT))
						{enemy[i].x -= player.speed*deltaTime/5;}
						if (GetAsyncKeyState(VK_LEFT))
						{enemy[i].x += player.speed*deltaTime/5;}
						if (GetAsyncKeyState(VK_DOWN))
						{enemy[i].y -= player.speed*deltaTime/5;}
						if (GetAsyncKeyState(VK_UP))
						{enemy[i].y += player.speed*deltaTime/5;}

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
							spawnDelay = 100 + IRand(150) - waveNumber * 10;
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
						projectile[i].Move(deltaTime);

						if (projectile[i].x > ScreenWidth + 16 || projectile[i].x < -16 || projectile[i].y < 16 || projectile[i].y > ScreenHeight + 16) // Check outside screen
						{
							projectile[i].alive = false;
						}

						for (int j = 0; j < enemyAmount; j++)
						{
							if (projectile[i].Collision(enemy[j].x, enemy[j].y) && enemy[j].alive == true) // If I collide with an enemy, then deal damage and fizzle
							{
								int recentDamage = projectile[i].damage * player.damageMultiplier;
								enemy[j].health -= recentDamage;
								enemy[j].recentDamage = recentDamage;
								enemy[j].recentlyDamaged = 330;
								projectile[i].alive = false;
							}
						}

						//Move the XP according to input
						if (GetAsyncKeyState(VK_RIGHT))
						{projectile[i].x -= player.speed * deltaTime / 5;}
						if (GetAsyncKeyState(VK_LEFT))
						{projectile[i].x += player.speed * deltaTime / 5;}
						if (GetAsyncKeyState(VK_DOWN))
						{projectile[i].y -= player.speed * deltaTime / 5;}
						if (GetAsyncKeyState(VK_UP))
						{projectile[i].y += player.speed * deltaTime / 5;}
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

						if (pickup_exp[i].GetDistance() < player.pickupRange) //If player ever got close, start moving the XP to the player
						{
							pickup_exp[i].magnet = true;
							if (pickup_exp[i].GetDistance() < 16) {
								pickup_exp[i].isPickedUp(); player.experience += pickup_exp[i].xpValue; pickup_exp[i].magnet = false;
								collectablesCollected++;
							} //If player gets close enough to XP, it picks the XP up.
						}

						if (pickup_exp[i].magnet == true) // Makes XP fly towards player
						{
							pickup_exp[i].isMagneted();
						}

						// Move the XP according to input
						if (GetAsyncKeyState(VK_RIGHT))
						{pickup_exp[i].x -= player.speed * deltaTime / 5;}
						if (GetAsyncKeyState(VK_LEFT))
						{pickup_exp[i].x += player.speed * deltaTime / 5;}
						if (GetAsyncKeyState(VK_DOWN))
						{pickup_exp[i].y -= player.speed * deltaTime / 5;}
						if (GetAsyncKeyState(VK_UP))
						{pickup_exp[i].y += player.speed * deltaTime / 5;}
					}


					if (debug == true)
					{
						if (GetAsyncKeyState(VK_SPACE) && keyUp < 15 && pickup_exp[i].alive == false) {
							pickup_exp[i].alive = true;
							pickup_exp[i].magnet = false;
							pickup_exp[i].x = IRand(ScreenWidth);
							pickup_exp[i].y = IRand(ScreenHeight);
							keyUp++;
						} // Pressing SPACE spawns XP gems randomly on my screen
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
				screen->Print(std::format("Level: {}", player.level), 12, 3, 0x000000, 2);
				screen->Box(0, 0, ScreenWidth - 1, 17, 0x000000); //Draw XP bar black outline

				// Draw wavenumber & Collectibles
				screen->Print(std::format("Wave: {}", waveNumber), 12, 20, 0x000000, 2);
				

				// Debug visuals
				if (debug == true)
				{
					screen->Box(enemy[nearestEnemyID].x, enemy[nearestEnemyID].y, enemy[nearestEnemyID].x + 64, enemy[nearestEnemyID].y + 64, 0x000000); //Draws a box for nearest target
					screen->Bar(player_drawx, player_drawy - 26, player_drawx + (player.attackInterval - player.attackFrame), player_drawy - 18, 0x00ffff);
					screen->Print(std::format("Collected Gems: {}", collectablesCollected), 12, 32, 0x000000, 2);
					screen->Print(std::format("Enemies Spawned so far: {}", enemiesSpawned), 12, 44, 0x000000, 2);
					screen->Print(std::format("Wave Delay: {}", static_cast<int>(waveTime)), 12, 56, 0x000000, 2);
					screen->Print(std::format("Enemy Spawn Delay: {}", static_cast<int>(spawnDelay)), 12, 68, 0x000000, 2);
				}
			}
		//startscreen pause
		if (gamePaused == 1)
			{
				sprite_startscreen.Draw(screen, (ScreenWidth / 2) - (sprite_startscreen.GetWidth() / 2), (ScreenHeight / 2) - (sprite_startscreen.GetHeight() / 2));
				if (GetAsyncKeyState(0x0001) || GetAsyncKeyState(0x0002)) // if either left or right mousebutton is clicked
				{
					gamePaused = 0;
				}
				sprite_cursor.Draw(screen, mousex, mousey);
			}
		//levelup pause
		if (gamePaused == 2)
			{
				sprite_levelupscreen.Draw(screen, (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2), (ScreenHeight / 2) - (sprite_levelupscreen.GetHeight() / 2));

				int hitbox1X = (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2) + 2;
				int hitbox1XX = (hitbox1X + sprite_upgrade_attackspeed.GetWidth());
				int hitbox1Y = (ScreenHeight / 2) - (sprite_levelupscreen.GetHeight() / 2) + 98;
				int hitbox1YY = (hitbox1Y + sprite_upgrade_attackspeed.GetHeight());
				int hitbox2X = (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2) + 112;
				int hitbox2XX = (hitbox2X + sprite_upgrade_attackspeed.GetWidth());
				int hitbox2Y = (ScreenHeight / 2) - (sprite_levelupscreen.GetHeight() / 2) + 98;
				int hitbox2YY = (hitbox2Y + sprite_upgrade_attackspeed.GetHeight());
				int hitbox3X = (ScreenWidth / 2) - (sprite_levelupscreen.GetWidth() / 2) + 221;
				int hitbox3XX = (hitbox3X + sprite_upgrade_attackspeed.GetWidth());
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
					if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0) // if left is clicked in certain hitbox
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

				sprite_cursor.Draw(screen, mousex, mousey);
			}
		//deathscreen pause
		if (gamePaused == 3)
			{
				sprite_deathscreen.Draw(screen, (ScreenWidth / 2) - (sprite_deathscreen.GetWidth() / 2), (ScreenHeight / 2) - (sprite_deathscreen.GetHeight() / 2));
				if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0)
				{
					player.Refresh(); //refresh the player stats

					// yeetus deleetus the enemies
					for (int i = 0; i < enemyAmount; i++)
					{
						enemy[i].alive = false;
					}
					enemyAliveCount = 0;		//Enemies that are alive will be counted
					waveNumber = 1;				//Which number wave of enemies we're on!
					enemiesSpawned = 0;			//Enemies spawned 0.
					waveTime = 10 * 165;		//How many seconds for the wave to start
					collectablesCollected = 0;
					enemiesDefeated = 0;
					timeSurvived = 0;

					// yeetus deleetus the XP gems
					for (int j = 0; j < enemyAmount * 3; j++)
					{
						pickup_exp[j].alive = false;
					}
					gamePaused = 0;
				}
				sprite_cursor.Draw(screen, mousex, mousey); 

				// Draw all the stats in the deathscreen
				screen->Print(std::format("Player level:"), 120, 500, 0x000000, 5); screen->Print(std::format("{}", player.level), 660, 500, 0x000000, 5);
				screen->Print(std::format("Player speed:"), 120, 550, 0x000000, 5); screen->Print(std::format("{:.01f}", player.speed), 660, 550, 0x000000, 5);
				screen->Print(std::format("Player attspd:"), 120, 600, 0x000000, 5); screen->Print(std::format("{:.01f}", player.attackSpeed), 660, 600, 0x000000, 5);
				screen->Print(std::format("Player health:"), 120, 650, 0x000000, 5); screen->Print(std::format("{}", player.maxhealth), 660, 650, 0x000000, 5);
				screen->Print(std::format("Enemies defeated:"), 120, 700, 0x000000, 5); screen->Print(std::format("{}", enemiesDefeated), 660, 700, 0x000000, 5);
				screen->Print(std::format("Wave reached:"), 120, 750, 0x000000, 5); screen->Print(std::format("{}", waveNumber), 660, 750, 0x000000, 5);
				screen->Print(std::format("Fireball damage:"), 120, 800, 0x000000, 5); screen->Print(std::format("{:.01f}", 35*player.damageMultiplier), 660, 800, 0x000000, 5);
				screen->Print(std::format("Time Surived:"), 120, 850, 0x000000, 5); screen->Print(std::format("{:.01f} seconds", timeSurvived/1000), 660, 850, 0x000000, 5);
			}

		//Calculate FPS over last fpsAveragePrecision amount of frames
		if (fpsAverageTiming++ < fpsAveragePrecision)
		{
			fpsAverageCalc += 1000 / deltaTime;
		}
		else
		{
			fpsAverage = fpsAverageCalc;
			fpsAverageCalc = 0;
			fpsAverageTiming = 0;
		}


		screen->Print(std::format("FPS: {}", static_cast<int>(fpsAverage/fpsAveragePrecision)), ScreenWidth - 120, 20, 0x000000, 2);
	}
};