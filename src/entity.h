#pragma once

#include <gint/display.h>

#include "inventory.h"
#include "menu.h"

enum PhysicsTypes {
	PHYS_NON_SOLID,
	PHYS_SOLID,
	PHYS_PLATFORM
};

typedef struct {
	enum Items item;

	int amountMin;
	int amountMax;

	int ratioLow;
	int ratioHigh;
} Drop;

struct EntityDrops {
	int num;

	const Drop* dropList;
};

struct Coords {
	int x;
	int y;
};

struct Rect {
	struct Coords TL;
	struct Coords BR;
};

struct EntityPhysicsProps {
	unsigned char width;
	unsigned char height;
	int x;
	int y;
	float xVel;
	float yVel;
	bool touchingTileTop;
	bool dropping;
};

struct AnimationData {
	int animation;
	int animationFrame;
	int direction;
};

enum EntityAlignments {
	ALIGN_PEACEFUL,
	ALIGN_NEUTRAL,
	ALIGN_HOSTILE,
	ALIGN_SCARED
};

struct Combat {
	int health;

	enum EntityAlignments alignment;

	int immuneFrames;
	int currImmuneFrames;
	
	int attack;
	int defense;
	float knockbackResist;
};

enum Entities {
	ENT_SLIME,

	ENTITIES_COUNT
};

struct EntityBase {
	int id;
	struct EntityPhysicsProps props;
	struct Combat combat;
	bopti_image_t *sprite;
	const struct EntityDrops *drops;

	bool (*behaviour)(struct EntityBase *self, int frames);
	void (*init)(struct EntityBase *self);

//	Initialised to 0
	struct AnimationData anim;
	int despawnCounter;
//	To store states in, generic so that entities can use it however
	int mem[4];
};

typedef struct EntityBase Entity;

struct Player {
	struct EntityPhysicsProps props;
	struct AnimationData anim;
	struct Combat combat;
	struct Coords cursor;
	struct Coords cursorTile;
	struct Inventory inventory;

	int swingFrame;
	bool swingDir;

	void (*physics)(struct EntityPhysicsProps *self);
};

extern const struct EntityBase entityTemplates[];

extern struct Player player;

void handlePhysics(struct EntityPhysicsProps *self);
bool checkCollision(struct EntityPhysicsProps *first, struct EntityPhysicsProps *second);

void attack(Entity *entity, bool isPlayerAttacking);
void doEntityCycle(int frames);

void doSpawningCycle();