#include "game.h"
#include "player.h"
#include "enemy.h"
#include "surface.h"
#include "algorithm" //Quick maffs
#include "template.h"
#include <iostream> 
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

namespace Tmpl8
{
	Player player; //Creates player variable player

	const int enemyAmount = 100;
	Enemy enemy[enemyAmount];

	Sprite sprite_player(new Surface("assets/player_wizard.png"), 2);
	Sprite sprite_enemy(new Surface("assets/enemy_wizard.png"), 2);

	int c_red = 255;
	int c_green = 255;

	const int player_drawx = (player.x - (sprite_player.GetWidth() / 2));
	const int player_drawy = (player.y - (sprite_player.GetHeight() / 2));

	// -----------------------------------------------------------
	// Initialize the application
	// -----------------------------------------------------------
	void Game::Init()
	{
	}
	
	// -----------------------------------------------------------
	// Close down application
	// -----------------------------------------------------------
	void Game::Shutdown()
	{
	}

	// -----------------------------------------------------------
	// Main application tick function
	// -----------------------------------------------------------
    void Game::Tick(float deltaTime)
    {
        // clear the graphics window
        screen->Clear(0xaaa0a0);

		if (GetAsyncKeyState(VK_RIGHT)) {sprite_player.SetFrame(0);} // Set Sprite to right
		if (GetAsyncKeyState(VK_LEFT)) {sprite_player.SetFrame(1);} // Set Sprite to left

		if (player.isDead())
		{
			player.Refresh(); //refresh the player stats

			// refresh the enemies
			for (int i = 0; i < enemyAmount; i++)
			{
				enemy[i].Reposition();
				enemy[i].Refresh();
			}
		}

		int redness = round(c_red*player.healthpercentage);
		int greenness = round(c_green*player.healthpercentage);

		// c_red & c_green are both 255, its not needed but for legibility
		if (player.healthpercentage>0.5)
		{
			player.healthcolor = (( - redness-1) * 2 << 16) + (c_green << 8); //green to yellow
		}
		else
		{
			player.healthcolor = (c_red << 16) + ((greenness-1)*2 << 8); //yellow to red
		}

		// Draws Player HP bar and player itself
		sprite_player.Draw(screen, player_drawx, player_drawy);	// Draw player 
		player.healthpercentage = player.currenthealth / player.maxhealth;	// Update player HP bar percentage && Color
		screen->Bar(player_drawx, player_drawy-16, (player_drawx+sprite_player.GetWidth()*player.healthpercentage), player_drawy-8, player.healthcolor);	// Healthbar percentage = length
		screen->Box(player_drawx - 1, player_drawy - 17, player_drawx + sprite_player.GetWidth() + 1, player_drawy - 7, 0); // Black box around HP bar

		//-------------------------------------------
		//     Handles Enemies behaviour loop       |
		//-------------------------------------------
		for (int i =  0; i < enemyAmount; i++)
		{
			if (enemy[i].isAlive())
			{
				//-------------------------------------------
				//  Handles Enemies bumping into eachother  |
				//-------------------------------------------
				for (int j = 0; j < enemyAmount; j++)
				{
					//Checks distance between all enemies entities, includes itself for now
					if (i != j)
					{
						float jdx = enemy[i].x - enemy[j].x, jdy = enemy[i].y - enemy[j].y;
						float jdist = sqrtf(jdx * jdx + jdy * jdy);

						if (jdist < 48) // if im close to another enemy, but not checking for myself
						{
							if (enemy[j].x > enemy[i].x)
							{
								enemy[i].x -= 1;
								enemy[j].x += 1;
							}
							if (enemy[j].x < enemy[i].x)
							{
								enemy[i].x += 1;
								enemy[j].x -= 1;
							}
							if (enemy[j].y > enemy[i].y)
							{
								enemy[i].y -= 1;
								enemy[j].y += 1;
							}
							if (enemy[j].y < enemy[i].y)
							{
								enemy[i].y += 1;
								enemy[j].y -= 1;
							}
						}
					}
				}

				// Checks where my player is and moves towards it. (center of screen)
				enemy[i].Move(player_drawx, player_drawy);

				//Damaging the player
				if (enemy[i].GetDistance() < 48)
				{
					player.currenthealth -= enemy[i].damage;
				}

				// If my player tries to move, enemies move the opposite way for the illusion of movement.
				if (GetAsyncKeyState(VK_RIGHT)){enemy[i].x -= player.speed;}
				if (GetAsyncKeyState(VK_LEFT)){enemy[i].x += player.speed;}
				if (GetAsyncKeyState(VK_UP)){enemy[i].y += player.speed;}
				if (GetAsyncKeyState(VK_DOWN)){enemy[i].y -= player.speed;}


				// Draw my enemy sprites either facing left or right
				if (enemy[i].x > player_drawx)
				{
					sprite_enemy.SetFrame(1);
					sprite_enemy.Draw(screen, enemy[i].x, enemy[i].y);	// Draws entity
				}
				else
				{
					sprite_enemy.SetFrame(0);
					sprite_enemy.Draw(screen, enemy[i].x, enemy[i].y);	// Draws entity
				}
			}
		}
    }
} 