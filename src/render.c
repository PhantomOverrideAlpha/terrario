#include <gint/gray.h>
#include <gint/defs/util.h>
#include <stdbool.h>
#include <gint/bfile.h>
#include <gint/std/string.h>

#include "syscalls.h"
#include "render.h"
#include "defs.h"
#include "world.h"
#include "entity.h"

const unsigned int camMinX = SCREEN_WIDTH >> 1;
const unsigned int camMaxX = (WORLD_WIDTH << 3) - (SCREEN_WIDTH >> 1);
const unsigned int camMinY = SCREEN_HEIGHT >> 1;
const unsigned int camMaxY = (WORLD_HEIGHT << 3) - (SCREEN_HEIGHT >> 1);

int animFrames[][2] = {
	{0, 0},
	{1, 4},
	{5, 5},
	{6, 19}
};

void render(struct Player* player)
{
	extern bopti_image_t img_player, img_cursor, img_hotbar, img_hotbarselect;
	int camX = min(max(player->props.x + (player->props.width >> 1), camMinX), camMaxX);
	int camY = min(max(player->props.y + (player->props.height >> 1), camMinY), camMaxY);

//	Translating cam bounds to tile bounds is painful
	unsigned int tileLeftX = max(0, ((camX - (SCREEN_WIDTH >> 1)) >> 3) - 1);
	unsigned int tileRightX = min(WORLD_WIDTH - 1, tileLeftX + (SCREEN_WIDTH >> 3) + 1);
	unsigned int tileTopY = max(0, ((camY - (SCREEN_HEIGHT >> 1)) >> 3) - 1);
	unsigned int tileBottomY = min(WORLD_HEIGHT - 1, tileTopY + (SCREEN_HEIGHT >> 3) + 1);

	Tile* tile;
	const TileData* currTile;
	unsigned int currTileX, currTileY;
	int camOffsetX = (camX - (SCREEN_WIDTH >> 1));
	int camOffsetY = (camY - (SCREEN_HEIGHT >> 1));
	bool marginLeft, marginRight, marginTop, marginBottom;
	int flags;
	int subrectX, subrectY;
	int playerX, playerY;
	int playerSubrectX, playerSubrectY;

	if(player->props.xVel > 0)
	{
		player->anim.direction = 0;
	}
	else if(player->props.xVel < 0)
	{
		player->anim.direction = 1;
	}

	if(!player->props.touchingTileTop)
	{
		player->anim.animation = 2;
		player->anim.animationFrame = 5;
	}
	else if(player->props.xVel != 0 && player->anim.animation != 3)
	{
		player->anim.animation = 3;
		player->anim.animationFrame = 6;
	}
	else if(player->props.xVel == 0)
	{
		player->anim.animation = 0;
		player->anim.animationFrame = 0;
	}
	else 
	{
		player->anim.animationFrame++;
	}

	if(player->anim.animationFrame > animFrames[player->anim.animation][1]) 
	{
		player->anim.animationFrame = animFrames[player->anim.animation][0];
	}

//	This probably shouldn't be here but cam positions can't be accessed anywhere else right now
	player->cursorTile.x = (camX + player->cursor.x - (SCREEN_WIDTH >> 1)) >> 3;
	player->cursorTile.y = (camY + player->cursor.y - (SCREEN_HEIGHT >> 1)) >> 3;

	dclear(C_WHITE);

	for(unsigned int y = tileTopY; y <= tileBottomY; y++)
	{
		for(unsigned int x = tileLeftX; x <= tileRightX; x++)
		{
			tile = &world.tiles[y * WORLD_WIDTH + x];
			currTile = &tiles[tile->idx];
			currTileX = (x << 3) - camOffsetX;
			currTileY = (y << 3) - camOffsetY;
			if(currTile->render)
			{
				/* Disable clipping unless it's a block on the edges of the screen.
				This reduces rendering time a bit (edges still need clipping or
				we might crash trying to write outside the VRAM). */
				marginLeft = x - tileLeftX <= 1;
				marginRight = tileRightX - x <= 1;
				marginTop = y - tileTopY <= 1;
				marginBottom = tileBottomY - y <= 1;
				if(marginLeft | marginRight | marginTop | marginBottom)
				{
					flags = DIMAGE_NONE;
				} else
				{
					flags = DIMAGE_NOCLIP;
				}
				if(currTile->hasSpritesheet)
				{
//					Spritesheet layout allows for very fast calculation of the position of the sprite
					subrectX = ((tile->state & 3) << 3) + (tile->state & 3) + 1;
					subrectY = ((tile->state >> 2) << 3) + (tile->state >> 2) + 1;
					dsubimage(currTileX, currTileY, currTile->sprite, subrectX, subrectY, 8, 8, flags);
				}
				else
				{
					dsubimage(currTileX, currTileY, currTile->sprite, 0, 0, 8, 8, flags);
				}
				
			}
		}
	}
	playerX = player->props.x - (camX - (SCREEN_WIDTH >> 1)) - 2;
	playerY = player->props.y - (camY - (SCREEN_HEIGHT >> 1));
	playerSubrectX = (player->anim.direction == 0) ? 0 : 16;
	playerSubrectY = player->anim.animationFrame * (player->props.height + 2) + 1;
	dsubimage(playerX, playerY, &img_player, playerSubrectX, playerSubrectY, 16, 22, DIMAGE_NONE);
	dimage(player->cursor.x - 2, player->cursor.y - 2, &img_cursor);
	//dimage(0, 0, &img_hotbar);
	//dimage(0, 0, &img_hotbarselect);
}

void takeVRAMCapture()
{
	uint32_t* light;
	uint32_t* dark;

	uint16_t* pathLight = u"\\\\fls0\\light.vram";
	uint16_t* pathDark = u"\\\\fls0\\dark.vram";
	int descriptor;
	int size = 1024;

	dgray_getvram(&light, &dark);

	BFile_Remove(pathLight);
	BFile_Create(pathLight, BFile_File, &size);

	BFile_Remove(pathDark);
	BFile_Create(pathDark, BFile_File, &size);

	descriptor = BFile_Open(pathLight, BFile_WriteOnly);
	BFile_Write(descriptor, light, size);
	BFile_Close(descriptor);

	descriptor = BFile_Open(pathDark, BFile_WriteOnly);
	BFile_Write(descriptor, dark, size);
	BFile_Close(descriptor);
}