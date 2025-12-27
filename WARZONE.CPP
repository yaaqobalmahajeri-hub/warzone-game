/*
 * ========================================================
 *     W A R Z O N E   E L I T E
 *     
 * ========================================================
 */

#include <graphics.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* =============== CONSTANTS =============== */
#define SCREEN_W 640
#define SCREEN_H 350
#define GROUND_Y 320

#define MAX_PLAYERS 6
#define MAX_BULLETS 50
#define MAX_GRENADES 10
#define MAX_PLATFORMS 15
#define MAX_EFFECTS 30
#define MAX_PARTICLES 80
#define MAX_CLOUDS 8
#define MAX_PICKUPS 10
#define MAX_KILLFEED 4

#define PI 3.14159265

/* Game States */
#define GS_SPLASH 0
#define GS_MENU 1
#define GS_DIFFICULTY 2
#define GS_PLAYING 3
#define GS_PAUSED 4
#define GS_GAMEOVER 5
#define GS_VICTORY 6
#define GS_ROUNDEND 7

/* Win Conditions */
#define KILLS_TO_WIN 10
#define TIME_LIMIT 5400

/* Weapon Types */
#define WP_PISTOL 0
#define WP_RIFLE 1
#define WP_SHOTGUN 2
#define WP_SNIPER 3
#define WP_ROCKET 4

/* AI States */
#define AI_PATROL 0
#define AI_CHASE 1
#define AI_ATTACK 2
#define AI_FLEE 3

/* Pickup Types */
#define PK_HEALTH 0
#define PK_AMMO 1
#define PK_WEAPON 2
#define PK_JETPACK 3

/* Keys */
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_SPACE 32
#define KEY_ENTER 13
#define KEY_ESC 27

/* =============== DATA STRUCTURES =============== */

struct Weapon
{
    int type;
    int damage;
    int fireRate;
    int ammo;
    int maxAmmo;
    int cooldown;
    int pellets;
    int speed;
};

struct Player
{
    float x, y;
    float vx, vy;
    int hp, maxHp;
    int alive;
    int respawnTimer;
    int direction;
    int walking;
    int jumping;
    int shooting;
    int shootCooldown;
    int color;
    int isBot;
    int aiState;
    int aiTimer;
    int aiTarget;
    int kills;
    int deaths;
    float aimAngle;
    struct Weapon weapon;
    int animFrame;
    int jetpackFuel;
    int maxJetpackFuel;
    int usingJetpack;
    int streak;
    int comboTimer;
    int score;
    int headColor;
    int bodyColor;
    char name[10];
};

struct Bullet
{
    float x, y;
    float vx, vy;
    int damage;
    int owner;
    int active;
    int color;
    int isRocket;
};

struct Grenade
{
    float x, y;
    float vx, vy;
    int timer;
    int owner;
    int active;
    int exploded;
};

struct Platform
{
    int x, y;
    int width, height;
    int active;
};

struct Effect
{
    float x, y;
    int type;
    int timer;
    int active;
    int color;
};

struct Particle
{
    float x, y;
    float vx, vy;
    int color;
    int life;
    int size;
    int active;
};

struct Cloud
{
    float x;
    int y;
    int width;
    float speed;
    int active;
};

struct Pickup
{
    float x, y;
    int type;
    int active;
    int respawnTimer;
    int floatOffset;
};

struct KillFeed
{
    int killer;
    int victim;
    int timer;
    int active;
};

/* =============== GLOBALS =============== */

struct Player players[MAX_PLAYERS];
struct Bullet bullets[MAX_BULLETS];
struct Grenade grenades[MAX_GRENADES];
struct Platform platforms[MAX_PLATFORMS];
struct Effect effects[MAX_EFFECTS];
struct Particle particles[MAX_PARTICLES];
struct Cloud clouds[MAX_CLOUDS];
struct Pickup pickups[MAX_PICKUPS];
struct KillFeed killfeed[MAX_KILLFEED];

int numPlayers = 4;
int playerIndex = 0;
int gameState = GS_SPLASH;
int difficulty = 1;
int activePage = 0;
int frameCount = 0;
int matchTime = 0;
int splashTimer = 0;
int roundNumber = 1;
int winnerIndex = -1;
int roundEndTimer = 0;
int showStats = 0;

float cameraX = 0;
float cameraY = 0;
float shakeX = 0;
float shakeY = 0;
int shakeTimer = 0;

float aiAccuracy[3] = {0.3, 0.5, 0.7};
int aiReactionTime[3] = {60, 40, 20};
int soundEnabled = 1;

char *weaponNames[5] = {"PISTOL", "RIFLE", "SHOTGUN", "SNIPER", "ROCKET"};

/* =============== SOUND EFFECTS =============== */

void playShoot(int wtype)
{
    if (!soundEnabled)
        return;
    if (wtype == WP_SHOTGUN)
    {
        sound(400);
        delay(20);
        sound(200);
        delay(20);
    }
    else if (wtype == WP_SNIPER)
    {
        sound(1200);
        delay(30);
        sound(600);
        delay(20);
    }
    else if (wtype == WP_ROCKET)
    {
        sound(200);
        delay(40);
        sound(100);
        delay(30);
    }
    else
    {
        sound(800);
        delay(15);
    }
    nosound();
}

void playExplosion(void)
{
    if (!soundEnabled)
        return;
    sound(150);
    delay(30);
    sound(100);
    delay(30);
    sound(50);
    delay(40);
    nosound();
}

void playHit(void)
{
    if (!soundEnabled)
        return;
    sound(300);
    delay(20);
    sound(200);
    delay(20);
    nosound();
}

void playDeath(void)
{
    if (!soundEnabled)
        return;
    sound(400);
    delay(50);
    sound(300);
    delay(50);
    sound(200);
    delay(50);
    sound(100);
    delay(100);
    nosound();
}

void playJump(void)
{
    if (!soundEnabled)
        return;
    sound(400);
    delay(10);
    sound(600);
    delay(10);
    sound(800);
    delay(10);
    nosound();
}

void playJetpack(void)
{
    if (!soundEnabled)
        return;
    sound(100 + rand() % 50);
    delay(5);
    nosound();
}

void playPickup(void)
{
    if (!soundEnabled)
        return;
    sound(800);
    delay(20);
    sound(1000);
    delay(20);
    sound(1200);
    delay(30);
    nosound();
}

void playStreak(int streak)
{
    if (!soundEnabled)
        return;
    int i;
    for (i = 0; i < streak && i < 5; i++)
    {
        sound(800 + i * 200);
        delay(50);
    }
    nosound();
}

void playMenu(void)
{
    if (!soundEnabled)
        return;
    sound(1000);
    delay(30);
    nosound();
}

void playVictory(void)
{
    if (!soundEnabled)
        return;
    int notes[6] = {523, 659, 784, 1047, 784, 1047};
    int i;
    for (i = 0; i < 6; i++)
    {
        sound(notes[i]);
        delay(100);
    }
    nosound();
}

void playGameOver(void)
{
    if (!soundEnabled)
        return;
    sound(400);
    delay(150);
    sound(300);
    delay(150);
    sound(200);
    delay(200);
    nosound();
}

/* =============== FORWARD DECLARATIONS =============== */

void initGame(void);
void initPlayers(void);
void initMap(void);
void initClouds(void);
void initPickups(void);
void mainLoop(void);
void handleInput(void);
void updatePhysics(void);
void updateBullets(void);
void updateGrenades(void);
void updateAI(void);
void updateEffects(void);
void updateParticles(void);
void updateClouds(void);
void updatePickups(void);
void updateKillfeed(void);
void updateShake(void);
void checkCollisions(void);
void render(void);
void renderMap(void);
void renderPlayers(void);
void renderBullets(void);
void renderGrenades(void);
void renderEffects(void);
void renderParticles(void);
void renderPickups(void);
void renderUI(void);
void renderMinimap(void);
void renderKillfeed(void);
void drawSplash(void);
void drawMenu(void);
void drawDifficulty(void);
void drawGameOver(void);
void drawVictory(void);
void drawPlayer(int idx, int sx, int sy);
void drawLogo(int x, int y, int size);
void fireBullet(int playerIdx);
void throwGrenade(int playerIdx);
void spawnEffect(float x, float y, int type, int color);
void spawnParticles(float x, float y, int count, int color, int spread);
void spawnExplosion(float x, float y);
void addKillfeed(int killer, int victim);
void triggerShake(int intensity);
void damagePlayer(int idx, int damage, int attacker);
void respawnPlayer(int idx);
int isOnPlatform(float x, float y, float *platformY);
void updateCamera(void);
void switchWeapon(int playerIdx, int weaponType);
void spawnPickup(float x, float y, int type);

/* =============== INITIALIZATION =============== */

void initGame(void)
{
    int gd = VGA, gm = VGAMED;
    initgraph(&gd, &gm, "C:\\TURBOC3\\BGI");

    if (graphresult() != grOk)
    {
        printf("Graphics error!\n");
        exit(1);
    }

    setvisualpage(0);
    setactivepage(1);
    activePage = 1;

    randomize();
    gameState = GS_SPLASH;
    splashTimer = 100;
}

void initClouds(void)
{
    int i;
    for (i = 0; i < MAX_CLOUDS; i++)
    {
        clouds[i].x = rand() % 1200;
        clouds[i].y = 30 + rand() % 80;
        clouds[i].width = 40 + rand() % 60;
        clouds[i].speed = 0.2 + (float)(rand() % 10) / 20.0;
        clouds[i].active = 1;
    }
}

void initPickups(void)
{
    int i;
    for (i = 0; i < MAX_PICKUPS; i++)
    {
        pickups[i].active = 0;
        pickups[i].respawnTimer = 0;
    }
    /* Spawn initial pickups */
    spawnPickup(200, 240, PK_HEALTH);
    spawnPickup(450, 190, PK_AMMO);
    spawnPickup(600, 90, PK_WEAPON);
    spawnPickup(800, 220, PK_JETPACK);
}

void initMap(void)
{
    int i;

    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        platforms[i].active = 0;
    }

    /* Ground */
    platforms[0].x = 0;
    platforms[0].y = GROUND_Y;
    platforms[0].width = 1200;
    platforms[0].height = 30;
    platforms[0].active = 1;

    /* Floating platforms */
    platforms[1].x = 100;
    platforms[1].y = 250;
    platforms[1].width = 120;
    platforms[1].height = 15;
    platforms[1].active = 1;

    platforms[2].x = 300;
    platforms[2].y = 200;
    platforms[2].width = 100;
    platforms[2].height = 15;
    platforms[2].active = 1;

    platforms[3].x = 500;
    platforms[3].y = 250;
    platforms[3].width = 120;
    platforms[3].height = 15;
    platforms[3].active = 1;

    platforms[4].x = 700;
    platforms[4].y = 180;
    platforms[4].width = 100;
    platforms[4].height = 15;
    platforms[4].active = 1;

    platforms[5].x = 900;
    platforms[5].y = 230;
    platforms[5].width = 120;
    platforms[5].height = 15;
    platforms[5].active = 1;

    platforms[6].x = 400;
    platforms[6].y = 130;
    platforms[6].width = 80;
    platforms[6].height = 15;
    platforms[6].active = 1;

    platforms[7].x = 600;
    platforms[7].y = 100;
    platforms[7].width = 100;
    platforms[7].height = 15;
    platforms[7].active = 1;

    platforms[8].x = 150;
    platforms[8].y = 160;
    platforms[8].width = 70;
    platforms[8].height = 15;
    platforms[8].active = 1;

    platforms[9].x = 850;
    platforms[9].y = 140;
    platforms[9].width = 80;
    platforms[9].height = 15;
    platforms[9].active = 1;
}

void switchWeapon(int playerIdx, int weaponType)
{
    struct Player *p = &players[playerIdx];

    p->weapon.type = weaponType;

    switch (weaponType)
    {
    case WP_PISTOL:
        p->weapon.damage = 12;
        p->weapon.fireRate = 15;
        p->weapon.maxAmmo = 20;
        p->weapon.pellets = 1;
        p->weapon.speed = 10;
        break;
    case WP_RIFLE:
        p->weapon.damage = 15;
        p->weapon.fireRate = 8;
        p->weapon.maxAmmo = 30;
        p->weapon.pellets = 1;
        p->weapon.speed = 12;
        break;
    case WP_SHOTGUN:
        p->weapon.damage = 8;
        p->weapon.fireRate = 25;
        p->weapon.maxAmmo = 16;
        p->weapon.pellets = 5;
        p->weapon.speed = 8;
        break;
    case WP_SNIPER:
        p->weapon.damage = 50;
        p->weapon.fireRate = 50;
        p->weapon.maxAmmo = 10;
        p->weapon.pellets = 1;
        p->weapon.speed = 18;
        break;
    case WP_ROCKET:
        p->weapon.damage = 40;
        p->weapon.fireRate = 60;
        p->weapon.maxAmmo = 5;
        p->weapon.pellets = 1;
        p->weapon.speed = 6;
        break;
    }
    p->weapon.ammo = p->weapon.maxAmmo;
    p->weapon.cooldown = 0;
}

void initPlayers(void)
{
    int i;
    int colors[6] = {LIGHTGREEN, RED, YELLOW, CYAN, MAGENTA, WHITE};
    float spawnX[6] = {100, 300, 500, 700, 900, 1000};

    numPlayers = 4;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        players[i].alive = 0;
    }

    for (i = 0; i < numPlayers; i++)
    {
        players[i].x = spawnX[i];
        players[i].y = GROUND_Y - 40;
        players[i].vx = 0;
        players[i].vy = 0;
        players[i].hp = 100;
        players[i].maxHp = 100;
        players[i].alive = 1;
        players[i].respawnTimer = 0;
        players[i].direction = 1;
        players[i].walking = 0;
        players[i].jumping = 0;
        players[i].shooting = 0;
        players[i].shootCooldown = 0;
        players[i].color = colors[i];
        players[i].headColor = colors[i];
        players[i].bodyColor = (i == 0) ? GREEN : (i == 1) ? LIGHTRED
                                              : (i == 2)   ? BROWN
                                                           : LIGHTBLUE;
        players[i].isBot = (i != playerIndex);
        players[i].aiState = AI_PATROL;
        players[i].aiTimer = 0;
        players[i].aiTarget = -1;
        players[i].kills = 0;
        players[i].deaths = 0;
        players[i].score = 0;
        players[i].aimAngle = 0;
        players[i].animFrame = 0;
        players[i].jetpackFuel = 100;
        players[i].maxJetpackFuel = 100;
        players[i].usingJetpack = 0;
        players[i].streak = 0;
        players[i].comboTimer = 0;

        /* Set player names */
        if (i == 0)
            strcpy(players[i].name, "HERO");
        else if (i == 1)
            strcpy(players[i].name, "BOSS");
        else if (i == 2)
            strcpy(players[i].name, "TANK");
        else
            strcpy(players[i].name, "NINJA");

        switchWeapon(i, WP_RIFLE);
    }

    for (i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = 0;
    for (i = 0; i < MAX_GRENADES; i++)
        grenades[i].active = 0;
    for (i = 0; i < MAX_EFFECTS; i++)
        effects[i].active = 0;
    for (i = 0; i < MAX_PARTICLES; i++)
        particles[i].active = 0;
    for (i = 0; i < MAX_KILLFEED; i++)
        killfeed[i].active = 0;

    cameraX = 0;
    matchTime = 0;
    shakeTimer = 0;
}

void startGame(void)
{
    initMap();
    initClouds();
    initPlayers();
    initPickups();
    gameState = GS_PLAYING;
}

/* =============== PLATFORM COLLISION =============== */

int isOnPlatform(float x, float y, float *platformY)
{
    int i;
    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        if (!platforms[i].active)
            continue;
        if (x >= platforms[i].x - 10 &&
            x <= platforms[i].x + platforms[i].width + 10)
        {
            if (y >= platforms[i].y - 5 && y <= platforms[i].y + 10)
            {
                *platformY = platforms[i].y;
                return 1;
            }
        }
    }
    return 0;
}

int checkPlatformBelow(float x, float y, float *platformY)
{
    int i;
    float closest = 9999;
    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        if (!platforms[i].active)
            continue;
        if (x >= platforms[i].x - 10 &&
            x <= platforms[i].x + platforms[i].width + 10)
        {
            if (platforms[i].y > y && platforms[i].y < closest)
            {
                closest = platforms[i].y;
                *platformY = platforms[i].y;
            }
        }
    }
    return (closest < 9999);
}

/* =============== EFFECTS & PARTICLES =============== */

void spawnEffect(float x, float y, int type, int color)
{
    int i;
    for (i = 0; i < MAX_EFFECTS; i++)
    {
        if (!effects[i].active)
        {
            effects[i].x = x;
            effects[i].y = y;
            effects[i].type = type;
            effects[i].color = color;
            effects[i].timer = (type == 2) ? 15 : 5;
            effects[i].active = 1;
            break;
        }
    }
}

void spawnParticles(float x, float y, int count, int color, int spread)
{
    int i, j;
    for (j = 0; j < count; j++)
    {
        for (i = 0; i < MAX_PARTICLES; i++)
        {
            if (!particles[i].active)
            {
                particles[i].x = x;
                particles[i].y = y;
                particles[i].vx = (float)(rand() % spread - spread / 2) / 5.0;
                particles[i].vy = (float)(rand() % spread - spread / 2) / 5.0 - 2;
                particles[i].color = color;
                particles[i].life = 20 + rand() % 20;
                particles[i].size = 1 + rand() % 2;
                particles[i].active = 1;
                break;
            }
        }
    }
}

void spawnExplosion(float x, float y)
{
    spawnEffect(x, y, 2, RED);
    spawnParticles(x, y, 15, YELLOW, 30);
    spawnParticles(x, y, 10, RED, 25);
    spawnParticles(x, y, 8, LIGHTGRAY, 20);
    triggerShake(8);
    playExplosion();
}

void triggerShake(int intensity)
{
    shakeTimer = intensity;
}

void updateShake(void)
{
    if (shakeTimer > 0)
    {
        shakeX = (float)(rand() % (shakeTimer * 2) - shakeTimer);
        shakeY = (float)(rand() % (shakeTimer * 2) - shakeTimer);
        shakeTimer--;
    }
    else
    {
        shakeX = 0;
        shakeY = 0;
    }
}

void updateEffects(void)
{
    int i;
    for (i = 0; i < MAX_EFFECTS; i++)
    {
        if (!effects[i].active)
            continue;
        effects[i].timer--;
        if (effects[i].timer <= 0)
        {
            effects[i].active = 0;
        }
    }
}

void updateParticles(void)
{
    int i;
    for (i = 0; i < MAX_PARTICLES; i++)
    {
        if (!particles[i].active)
            continue;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.15;
        particles[i].life--;
        if (particles[i].life <= 0 || particles[i].y > GROUND_Y)
        {
            particles[i].active = 0;
        }
    }
}

void updateClouds(void)
{
    int i;
    for (i = 0; i < MAX_CLOUDS; i++)
    {
        if (!clouds[i].active)
            continue;
        clouds[i].x += clouds[i].speed;
        if (clouds[i].x > 1250)
        {
            clouds[i].x = -clouds[i].width;
        }
    }
}

/* =============== KILLFEED =============== */

void addKillfeed(int killer, int victim)
{
    int i;
    /* Shift existing entries */
    for (i = MAX_KILLFEED - 1; i > 0; i--)
    {
        killfeed[i] = killfeed[i - 1];
    }
    killfeed[0].killer = killer;
    killfeed[0].victim = victim;
    killfeed[0].timer = 120;
    killfeed[0].active = 1;
}

void updateKillfeed(void)
{
    int i;
    for (i = 0; i < MAX_KILLFEED; i++)
    {
        if (!killfeed[i].active)
            continue;
        killfeed[i].timer--;
        if (killfeed[i].timer <= 0)
        {
            killfeed[i].active = 0;
        }
    }
}

/* =============== PICKUPS =============== */

void spawnPickup(float x, float y, int type)
{
    int i;
    for (i = 0; i < MAX_PICKUPS; i++)
    {
        if (!pickups[i].active)
        {
            pickups[i].x = x;
            pickups[i].y = y;
            pickups[i].type = type;
            pickups[i].active = 1;
            pickups[i].floatOffset = 0;
            pickups[i].respawnTimer = 0;
            break;
        }
    }
}

void updatePickups(void)
{
    int i, j;
    for (i = 0; i < MAX_PICKUPS; i++)
    {
        if (pickups[i].respawnTimer > 0)
        {
            pickups[i].respawnTimer--;
            if (pickups[i].respawnTimer == 0)
            {
                pickups[i].active = 1;
            }
            continue;
        }
        if (!pickups[i].active)
            continue;

        pickups[i].floatOffset = (int)(sin(frameCount * 0.1 + i) * 4);

        /* Check player collision */
        for (j = 0; j < numPlayers; j++)
        {
            if (!players[j].alive)
                continue;
            float dx = pickups[i].x - players[j].x;
            float dy = pickups[i].y - players[j].y;
            if (fabs(dx) < 25 && fabs(dy) < 30)
            {
                switch (pickups[i].type)
                {
                case PK_HEALTH:
                    players[j].hp += 30;
                    if (players[j].hp > players[j].maxHp)
                        players[j].hp = players[j].maxHp;
                    break;
                case PK_AMMO:
                    players[j].weapon.ammo += 15;
                    if (players[j].weapon.ammo > players[j].weapon.maxAmmo)
                        players[j].weapon.ammo = players[j].weapon.maxAmmo;
                    break;
                case PK_WEAPON:
                    switchWeapon(j, rand() % 5);
                    break;
                case PK_JETPACK:
                    players[j].jetpackFuel = players[j].maxJetpackFuel;
                    break;
                }
                if (j == playerIndex)
                    playPickup();
                pickups[i].active = 0;
                pickups[i].respawnTimer = 300;
                break;
            }
        }
    }
}

/* =============== PHYSICS =============== */

void updatePhysics(void)
{
    int i;
    float gravity = 0.5;
    float platformY;

    for (i = 0; i < numPlayers; i++)
    {
        if (!players[i].alive)
        {
            if (players[i].respawnTimer > 0)
            {
                players[i].respawnTimer--;
                if (players[i].respawnTimer == 0)
                    respawnPlayer(i);
            }
            continue;
        }

        if (players[i].usingJetpack && players[i].jetpackFuel > 0)
        {
            players[i].vy -= 0.8;
            players[i].jetpackFuel--;
            if (i == playerIndex && frameCount % 3 == 0)
                playJetpack();
            spawnParticles(players[i].x, players[i].y + 10, 1, YELLOW, 10);
        }

        players[i].vy += gravity;
        players[i].x += players[i].vx;
        players[i].y += players[i].vy;

        if (checkPlatformBelow(players[i].x, players[i].y - 30, &platformY))
        {
            if (players[i].y >= platformY)
            {
                players[i].y = platformY;
                players[i].vy = 0;
                players[i].jumping = 0;
                players[i].usingJetpack = 0;
                if (players[i].jetpackFuel < players[i].maxJetpackFuel)
                    players[i].jetpackFuel += 2;
            }
        }

        if (players[i].x < 20)
            players[i].x = 20;
        if (players[i].x > 1180)
            players[i].x = 1180;
        if (players[i].y > GROUND_Y)
        {
            players[i].y = GROUND_Y;
            players[i].vy = 0;
            players[i].jumping = 0;
        }
        if (players[i].y < 20)
            players[i].y = 20;

        players[i].vx *= 0.85;

        if (fabs(players[i].vx) > 0.5)
        {
            players[i].walking = 1;
            players[i].animFrame = (players[i].animFrame + 1) % 8;
        }
        else
            players[i].walking = 0;

        if (players[i].shootCooldown > 0)
            players[i].shootCooldown--;
        if (players[i].comboTimer > 0)
        {
            players[i].comboTimer--;
            if (players[i].comboTimer == 0)
                players[i].streak = 0;
        }
    }
}

/* =============== DAMAGE =============== */

void damagePlayer(int idx, int damage, int attacker)
{
    players[idx].hp -= damage;
    if (idx == playerIndex)
    {
        playHit();
        triggerShake(3);
    }
    spawnParticles(players[idx].x, players[idx].y - 15, 5, RED, 15);

    if (players[idx].hp <= 0)
    {
        players[idx].hp = 0;
        players[idx].alive = 0;
        players[idx].deaths++;
        players[idx].respawnTimer = 90;
        players[idx].streak = 0;
        spawnExplosion(players[idx].x, players[idx].y);

        if (attacker >= 0 && attacker != idx)
        {
            players[attacker].kills++;
            players[attacker].streak++;
            players[attacker].comboTimer = 150;
            addKillfeed(attacker, idx);
            if (players[attacker].streak == 3 && attacker == playerIndex)
                playStreak(3);
            if (players[attacker].streak == 5 && attacker == playerIndex)
                playStreak(5);
        }
        if (idx == playerIndex)
            playDeath();
    }
}

void respawnPlayer(int idx)
{
    float spawnX[6] = {100, 300, 500, 700, 900, 1000};
    players[idx].x = spawnX[idx] + rand() % 100 - 50;
    players[idx].y = GROUND_Y - 40;
    players[idx].vx = 0;
    players[idx].vy = 0;
    players[idx].hp = players[idx].maxHp;
    players[idx].alive = 1;
    players[idx].weapon.ammo = players[idx].weapon.maxAmmo;
    players[idx].jetpackFuel = players[idx].maxJetpackFuel;
}

/* =============== BULLETS =============== */

void fireBullet(int playerIdx)
{
    int i, p;
    struct Player *pl = &players[playerIdx];
    float angle, spread;

    if (pl->shootCooldown > 0 || pl->weapon.ammo <= 0)
        return;

    for (p = 0; p < pl->weapon.pellets; p++)
    {
        for (i = 0; i < MAX_BULLETS; i++)
        {
            if (!bullets[i].active)
            {
                angle = pl->aimAngle;
                if (pl->weapon.pellets > 1)
                {
                    spread = (float)(rand() % 40 - 20) / 100.0;
                    angle += spread;
                }
                bullets[i].x = pl->x + pl->direction * 15;
                bullets[i].y = pl->y - 20;
                bullets[i].vx = cos(angle) * pl->weapon.speed * pl->direction;
                bullets[i].vy = sin(angle) * pl->weapon.speed;
                bullets[i].damage = pl->weapon.damage;
                bullets[i].owner = playerIdx;
                bullets[i].active = 1;
                bullets[i].color = (pl->weapon.type == WP_ROCKET) ? RED : YELLOW;
                bullets[i].isRocket = (pl->weapon.type == WP_ROCKET);
                break;
            }
        }
    }

    pl->weapon.ammo--;
    pl->shootCooldown = pl->weapon.fireRate;
    pl->shooting = 1;
    spawnEffect(pl->x + pl->direction * 20, pl->y - 20, 0, YELLOW);
    if (playerIdx == playerIndex)
        playShoot(pl->weapon.type);
}

void updateBullets(void)
{
    int i, j, k;
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
            continue;
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        if (bullets[i].isRocket)
        {
            bullets[i].vy += 0.1;
            spawnParticles(bullets[i].x, bullets[i].y, 1, LIGHTGRAY, 5);
        }

        if (bullets[i].x < 0 || bullets[i].x > 1200 ||
            bullets[i].y < 0 || bullets[i].y > SCREEN_H)
        {
            if (bullets[i].isRocket)
                spawnExplosion(bullets[i].x, bullets[i].y);
            bullets[i].active = 0;
            continue;
        }

        for (j = 0; j < numPlayers; j++)
        {
            if (!players[j].alive || j == bullets[i].owner)
                continue;
            float dx = bullets[i].x - players[j].x;
            float dy = bullets[i].y - (players[j].y - 20);
            if (fabs(dx) < 20 && fabs(dy) < 30)
            {
                if (bullets[i].isRocket)
                {
                    for (k = 0; k < numPlayers; k++)
                    {
                        if (!players[k].alive)
                            continue;
                        float d = sqrt(pow(bullets[i].x - players[k].x, 2) +
                                       pow(bullets[i].y - players[k].y, 2));
                        if (d < 60)
                            damagePlayer(k, (int)(bullets[i].damage * (1 - d / 60)), bullets[i].owner);
                    }
                    spawnExplosion(bullets[i].x, bullets[i].y);
                }
                else
                {
                    damagePlayer(j, bullets[i].damage, bullets[i].owner);
                    spawnEffect(bullets[i].x, bullets[i].y, 1, RED);
                }
                bullets[i].active = 0;
                break;
            }
        }

        for (j = 0; j < MAX_PLATFORMS; j++)
        {
            if (!platforms[j].active)
                continue;
            if (bullets[i].x >= platforms[j].x && bullets[i].x <= platforms[j].x + platforms[j].width &&
                bullets[i].y >= platforms[j].y && bullets[i].y <= platforms[j].y + platforms[j].height)
            {
                if (bullets[i].isRocket)
                    spawnExplosion(bullets[i].x, bullets[i].y);
                else
                    spawnEffect(bullets[i].x, bullets[i].y, 1, LIGHTGRAY);
                bullets[i].active = 0;
                break;
            }
        }
    }
}

/* =============== GRENADES =============== */

void throwGrenade(int playerIdx)
{
    int i;
    struct Player *p = &players[playerIdx];
    for (i = 0; i < MAX_GRENADES; i++)
    {
        if (!grenades[i].active)
        {
            grenades[i].x = p->x;
            grenades[i].y = p->y - 20;
            grenades[i].vx = p->direction * 6;
            grenades[i].vy = -8;
            grenades[i].timer = 90;
            grenades[i].owner = playerIdx;
            grenades[i].active = 1;
            grenades[i].exploded = 0;
            break;
        }
    }
}

void updateGrenades(void)
{
    int i, j;
    for (i = 0; i < MAX_GRENADES; i++)
    {
        if (!grenades[i].active)
            continue;
        grenades[i].vy += 0.3;
        grenades[i].x += grenades[i].vx;
        grenades[i].y += grenades[i].vy;

        if (grenades[i].y > GROUND_Y - 5)
        {
            grenades[i].y = GROUND_Y - 5;
            grenades[i].vy = -grenades[i].vy * 0.5;
            grenades[i].vx *= 0.8;
        }

        grenades[i].timer--;
        if (grenades[i].timer <= 0 && !grenades[i].exploded)
        {
            grenades[i].exploded = 1;
            spawnExplosion(grenades[i].x, grenades[i].y);
            for (j = 0; j < numPlayers; j++)
            {
                if (!players[j].alive)
                    continue;
                float dx = grenades[i].x - players[j].x;
                float dy = grenades[i].y - players[j].y;
                float dist = sqrt(dx * dx + dy * dy);
                if (dist < 80)
                    damagePlayer(j, (int)(50 * (1 - dist / 80)), grenades[i].owner);
            }
            grenades[i].active = 0;
        }
    }
}

/* =============== AI =============== */

void updateAI(void)
{
    int i, j;
    struct Player *bot, *target;
    float nearestDist, dx, dy, dist;

    for (i = 0; i < numPlayers; i++)
    {
        if (!players[i].isBot || !players[i].alive)
            continue;
        bot = &players[i];
        bot->aiTimer++;

        nearestDist = 9999;
        bot->aiTarget = -1;
        for (j = 0; j < numPlayers; j++)
        {
            if (j == i || !players[j].alive)
                continue;
            dx = players[j].x - bot->x;
            dy = players[j].y - bot->y;
            dist = sqrt(dx * dx + dy * dy);
            if (dist < nearestDist)
            {
                nearestDist = dist;
                bot->aiTarget = j;
            }
        }

        switch (bot->aiState)
        {
        case AI_PATROL:
            if (bot->aiTimer % 60 == 0)
                bot->direction = (rand() % 2) ? 1 : -1;
            bot->vx += bot->direction * 0.3;
            if (nearestDist < 300)
            {
                bot->aiState = AI_CHASE;
                bot->aiTimer = 0;
            }
            break;
        case AI_CHASE:
            if (bot->aiTarget >= 0)
            {
                target = &players[bot->aiTarget];
                dx = target->x - bot->x;
                dy = target->y - bot->y;
                if (dx > 30)
                {
                    bot->vx += 0.4;
                    bot->direction = 1;
                }
                else if (dx < -30)
                {
                    bot->vx -= 0.4;
                    bot->direction = -1;
                }
                if (dy < -50 && !bot->jumping && bot->vy == 0)
                {
                    bot->vy = -10;
                    bot->jumping = 1;
                }
                if (nearestDist < 200)
                {
                    bot->aiState = AI_ATTACK;
                    bot->aiTimer = 0;
                }
            }
            if (nearestDist > 400)
            {
                bot->aiState = AI_PATROL;
                bot->aiTimer = 0;
            }
            break;
        case AI_ATTACK:
            if (bot->aiTarget >= 0)
            {
                target = &players[bot->aiTarget];
                dx = target->x - bot->x;
                dy = target->y - bot->y;
                bot->direction = (dx > 0) ? 1 : -1;
                bot->aimAngle = atan2(dy, fabs(dx));
                if (bot->aiTimer % aiReactionTime[difficulty] == 0)
                    if ((float)rand() / RAND_MAX < aiAccuracy[difficulty])
                        fireBullet(i);
                if (fabs(dx) > 150)
                    bot->vx += (dx > 0) ? 0.2 : -0.2;
            }
            if (nearestDist > 250)
                bot->aiState = AI_CHASE;
            if (bot->hp < 30)
            {
                bot->aiState = AI_FLEE;
                bot->aiTimer = 0;
            }
            break;
        case AI_FLEE:
            if (bot->aiTarget >= 0)
            {
                dx = players[bot->aiTarget].x - bot->x;
                if (dx > 0)
                {
                    bot->vx -= 0.5;
                    bot->direction = -1;
                }
                else
                {
                    bot->vx += 0.5;
                    bot->direction = 1;
                }
            }
            if (nearestDist > 350 || bot->hp > 50)
            {
                bot->aiState = AI_PATROL;
                bot->aiTimer = 0;
            }
            break;
        }
        if (bot->vx > 4)
            bot->vx = 4;
        if (bot->vx < -4)
            bot->vx = -4;
    }
}

/* =============== CAMERA =============== */

void updateCamera(void)
{
    float targetX = players[playerIndex].x - SCREEN_W / 2;
    if (targetX < 0)
        targetX = 0;
    if (targetX > 1200 - SCREEN_W)
        targetX = 1200 - SCREEN_W;
    cameraX += (targetX - cameraX) * 0.1;
}

/* =============== GAME LOGIC =============== */

void checkGameOver(void)
{
    int i, maxKills = 0, leader = -1;

    /* Check if anyone reached KILLS_TO_WIN */
    for (i = 0; i < numPlayers; i++)
    {
        if (players[i].kills > maxKills)
        {
            maxKills = players[i].kills;
            leader = i;
        }
    }

    /* Victory: First to 10 kills wins */
    if (maxKills >= KILLS_TO_WIN)
    {
        winnerIndex = leader;
        if (leader == playerIndex)
        {
            gameState = GS_VICTORY;
            playVictory();
        }
        else
        {
            gameState = GS_GAMEOVER;
            playGameOver();
        }
        return;
    }

    /* Time limit check */
    if (matchTime >= TIME_LIMIT)
    {
        winnerIndex = leader;
        if (leader == playerIndex)
        {
            gameState = GS_VICTORY;
            playVictory();
        }
        else
        {
            gameState = GS_GAMEOVER;
            playGameOver();
        }
        return;
    }

    /* Player died too many times */
    if (players[playerIndex].deaths >= 10)
    {
        winnerIndex = leader;
        gameState = GS_GAMEOVER;
        playGameOver();
    }
}

/* =============== INPUT =============== */

void handleInput(void)
{
    struct Player *p = &players[playerIndex];
    if (!kbhit())
        return;

    int key = getch();
    if (key == 0 || key == 224)
    {
        key = getch();
        if (gameState == GS_MENU && key == KEY_ENTER)
        {
            gameState = GS_DIFFICULTY;
            playMenu();
        }
        else if (gameState == GS_DIFFICULTY)
        {
            if (key == KEY_UP)
                difficulty = (difficulty - 1 + 3) % 3;
            else if (key == KEY_DOWN)
                difficulty = (difficulty + 1) % 3;
            else if (key == KEY_ENTER)
            {
                startGame();
                playMenu();
            }
        }
        else if (gameState == GS_PLAYING && p->alive)
        {
            if (key == KEY_LEFT)
            {
                p->vx -= 1.5;
                p->direction = -1;
            }
            if (key == KEY_RIGHT)
            {
                p->vx += 1.5;
                p->direction = 1;
            }
            if (key == KEY_UP)
            {
                if (p->jumping && p->jetpackFuel > 0)
                    p->usingJetpack = 1;
                else if (!p->jumping && p->vy >= 0)
                {
                    p->vy = -10;
                    p->jumping = 1;
                    playJump();
                }
            }
        }
        else if ((gameState == GS_GAMEOVER || gameState == GS_VICTORY) && key == KEY_ENTER)
            gameState = GS_MENU;
    }
    else
    {
        if (key == KEY_ESC)
        {
            if (gameState == GS_PLAYING)
                gameState = GS_MENU;
            else if (gameState == GS_SPLASH)
                gameState = GS_MENU;
            else if (gameState == GS_MENU)
            {
                closegraph();
                exit(0);
            }
        }
        if (key == KEY_SPACE && gameState == GS_PLAYING && p->alive)
            fireBullet(playerIndex);
        if (key == 'g' && gameState == GS_PLAYING && p->alive)
            throwGrenade(playerIndex);
        if (key == 's' && gameState == GS_PLAYING)
            soundEnabled = 1 - soundEnabled;
        if (key >= '1' && key <= '5' && gameState == GS_PLAYING)
            switchWeapon(playerIndex, key - '1');
        if (key == KEY_ENTER)
        {
            if (gameState == GS_SPLASH)
                gameState = GS_MENU;
            else if (gameState == GS_MENU)
            {
                gameState = GS_DIFFICULTY;
                playMenu();
            }
            else if (gameState == GS_DIFFICULTY)
            {
                startGame();
                playMenu();
            }
            else if (gameState == GS_GAMEOVER || gameState == GS_VICTORY)
                gameState = GS_MENU;
        }
    }
    p->usingJetpack = 0;
}

/* =============== RENDERING =============== */

void drawLogo(int x, int y, int size)
{
    setcolor(RED);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, size);
    outtextxy(x - 100, y, "WARZONE");
    setcolor(YELLOW);
    outtextxy(x - 50, y + 30, "ELITE");
}

void drawPlayer(int idx, int sx, int sy)
{
    struct Player *p = &players[idx];
    int legOffset = 0;
    int armOffset = 0;
    if (!p->alive)
        return;

    /* Animation */
    if (p->walking)
    {
        legOffset = (p->animFrame / 2) % 2 == 0 ? 4 : -4;
        armOffset = (p->animFrame / 2) % 2 == 0 ? 2 : -2;
    }

    /* Jetpack on back */
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(sx - 8, sy - 12, sx - 4, sy + 2);

    /* Legs with boots */
    setcolor(p->bodyColor);
    line(sx - 2, sy, sx - 6 + legOffset, sy + 16);
    line(sx + 2, sy, sx + 6 - legOffset, sy + 16);
    setfillstyle(SOLID_FILL, BROWN);
    fillellipse(sx - 6 + legOffset, sy + 18, 4, 3);
    fillellipse(sx + 6 - legOffset, sy + 18, 4, 3);

    /* Body/torso with armor */
    setfillstyle(SOLID_FILL, p->bodyColor);
    bar(sx - 5, sy - 14, sx + 5, sy);
    setcolor(LIGHTGRAY);
    line(sx - 5, sy - 14, sx - 5, sy);
    line(sx + 5, sy - 14, sx + 5, sy);
    /* Armor line */
    line(sx - 4, sy - 7, sx + 4, sy - 7);

    /* Shoulders */
    setfillstyle(SOLID_FILL, p->bodyColor);
    fillellipse(sx - 7, sy - 12, 3, 3);
    fillellipse(sx + 7, sy - 12, 3, 3);

    /* Arms */
    int armX = sx + p->direction * 14;
    int armY = sy - 10 + armOffset;
    setcolor(p->headColor);
    line(sx + p->direction * 7, sy - 12, armX, armY);

    /* Weapon in hand */
    setcolor(LIGHTGRAY);
    int wepLen = (p->weapon.type == WP_SNIPER) ? 20 : (p->weapon.type == WP_ROCKET) ? 18
                                                  : (p->weapon.type == WP_SHOTGUN)  ? 14
                                                                                    : 12;
    int wepX = armX + p->direction * wepLen;
    int wepY = armY + (int)(sin(p->aimAngle) * 8);
    line(armX, armY, wepX, wepY);
    if (p->weapon.type == WP_ROCKET)
    {
        setfillstyle(SOLID_FILL, DARKGRAY);
        fillellipse(wepX, wepY, 3, 2);
    }

    /* Head with helmet */
    setfillstyle(SOLID_FILL, p->headColor);
    fillellipse(sx, sy - 20, 8, 8);

    /* Helmet visor */
    setcolor(LIGHTBLUE);
    arc(sx, sy - 20, 0, 180, 6);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    fillellipse(sx + p->direction * 2, sy - 20, 4, 3);

    /* Eye */
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(sx + p->direction * 4, sy - 21, 2, 2);
    setfillstyle(SOLID_FILL, BLACK);
    fillellipse(sx + p->direction * 4, sy - 21, 1, 1);

    /* Jetpack flame when using */
    if (p->usingJetpack)
    {
        setfillstyle(SOLID_FILL, YELLOW);
        int flameLen = 8 + rand() % 6;
        fillellipse(sx - 6, sy + 5 + flameLen / 2, 3, flameLen / 2);
        setcolor(RED);
        line(sx - 6, sy + 5, sx - 6, sy + 5 + flameLen);
        setcolor(YELLOW);
        line(sx - 5, sy + 5, sx - 5, sy + 3 + flameLen - 2);
    }

    /* HP bar above head */
    int hpWidth = (int)((float)p->hp / p->maxHp * 30);
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(sx - 15, sy - 38, sx + 15, sy - 34);
    setfillstyle(SOLID_FILL, (p->hp > 50) ? GREEN : (p->hp > 25) ? YELLOW
                                                                 : RED);
    bar(sx - 15, sy - 38, sx - 15 + hpWidth, sy - 34);
    setcolor((p->hp > 50) ? GREEN : (p->hp > 25) ? YELLOW
                                                 : RED);
    rectangle(sx - 15, sy - 38, sx + 15, sy - 34);

    setcolor(p->color);
    outtextxy(sx - 12, sy - 48, p->name);

    if (p->streak >= 3)
    {
        setcolor(YELLOW);
        char streakBuf[5];
        sprintf(streakBuf, "x%d", p->streak);
        outtextxy(sx + 10, sy - 38, streakBuf);
    }
}

void renderPlayers(void)
{
    int i;
    for (i = 0; i < numPlayers; i++)
    {
        int sx = (int)(players[i].x - cameraX + shakeX);
        int sy = (int)(players[i].y + shakeY);
        if (sx < -50 || sx > SCREEN_W + 50)
            continue;
        if (players[i].alive)
            drawPlayer(i, sx, sy);
        else if (players[i].respawnTimer > 0)
        {
            char buf[20];
            setcolor(players[i].color);
            sprintf(buf, "RESPAWN %d", players[i].respawnTimer / 30 + 1);
            outtextxy(sx - 30, sy - 20, buf);
        }
    }
}

void renderMap(void)
{
    int i, j;

    /* Enhanced sky gradient - more layers */
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, SCREEN_W, 50);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 50, SCREEN_W, 100);
    setfillstyle(SOLID_FILL, CYAN);
    bar(0, 100, SCREEN_W, 180);
    setfillstyle(SOLID_FILL, LIGHTCYAN);
    bar(0, 180, SCREEN_W, GROUND_Y);

    /* Stars in upper sky */
    setcolor(WHITE);
    for (i = 0; i < 15; i++)
    {
        int sx = (i * 47 + frameCount / 3) % SCREEN_W;
        int sy = 10 + (i * 23) % 40;
        putpixel(sx, sy, WHITE);
        if (i % 3 == 0)
            putpixel(sx + 1, sy, LIGHTGRAY);
    }

    /* Sun with glow effect */
    int sunX = 80 - (int)(cameraX * 0.1);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(sunX, 45, 22, 22);
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(sunX - 5, 40, 8, 8);
    /* Sun rays */
    setcolor(YELLOW);
    for (i = 0; i < 8; i++)
    {
        int angle = i * 45 + frameCount % 45;
        int rx = sunX + (int)(cos(angle * PI / 180) * 30);
        int ry = 45 + (int)(sin(angle * PI / 180) * 30);
        line(sunX + (int)(cos(angle * PI / 180) * 24),
             45 + (int)(sin(angle * PI / 180) * 24), rx, ry);
    }

    /* Background mountains */
    setfillstyle(SOLID_FILL, LIGHTGRAY);
    int mOffset = (int)(cameraX * 0.2);
    /* Mountain 1 */
    int mx1[4] = {50 - mOffset, 180 - mOffset, 310 - mOffset, 50 - mOffset};
    int my1[4] = {GROUND_Y - 30, 100, GROUND_Y - 30, GROUND_Y - 30};
    fillpoly(3, (int *)mx1);
    /* Mountain 2 */
    int mx2[4] = {250 - mOffset, 400 - mOffset, 550 - mOffset, 250 - mOffset};
    int my2[4] = {GROUND_Y - 30, 80, GROUND_Y - 30, GROUND_Y - 30};
    fillpoly(3, (int *)mx2);
    /* Mountain 3 */
    int mx3[4] = {450 - mOffset, 620 - mOffset, 790 - mOffset, 450 - mOffset};
    int my3[4] = {GROUND_Y - 30, 110, GROUND_Y - 30, GROUND_Y - 30};
    fillpoly(3, (int *)mx3);
    /* Snow caps */
    setcolor(WHITE);
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(180 - mOffset, 105, 15, 8);
    fillellipse(400 - mOffset, 85, 18, 10);
    fillellipse(620 - mOffset, 115, 14, 7);

    /* Fluffy clouds with shadow */
    for (i = 0; i < MAX_CLOUDS; i++)
    {
        if (!clouds[i].active)
            continue;
        int cx = (int)(clouds[i].x - cameraX * 0.3);
        if (cx < -100 || cx > SCREEN_W + 100)
            continue;
        /* Cloud shadow */
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        fillellipse(cx + 3, clouds[i].y + 3, clouds[i].width / 2, 12);
        fillellipse(cx + 23, clouds[i].y - 2, clouds[i].width / 3, 10);
        /* Main cloud */
        setfillstyle(SOLID_FILL, WHITE);
        fillellipse(cx, clouds[i].y, clouds[i].width / 2, 12);
        fillellipse(cx + 20, clouds[i].y - 5, clouds[i].width / 3, 10);
        fillellipse(cx - 15, clouds[i].y + 3, clouds[i].width / 3, 8);
        fillellipse(cx + 35, clouds[i].y - 2, clouds[i].width / 4, 9);
    }

    /* Ground with grass texture */
    setfillstyle(SOLID_FILL, BROWN);
    bar(0, GROUND_Y, SCREEN_W, SCREEN_H);
    setfillstyle(SOLID_FILL, GREEN);
    bar(0, GROUND_Y, SCREEN_W, GROUND_Y + 5);
    /* Grass blades */
    setcolor(GREEN);
    for (i = 0; i < SCREEN_W; i += 8)
    {
        int gx = i - ((int)cameraX % 8);
        int gh = 3 + (i * 7) % 5;
        line(gx, GROUND_Y, gx, GROUND_Y - gh);
        line(gx + 3, GROUND_Y, gx + 2, GROUND_Y - gh + 2);
    }
    /* Dark grass line */
    setcolor(LIGHTGREEN);
    line(0, GROUND_Y + 2, SCREEN_W, GROUND_Y + 2);

    /* Platforms with 3D effect */
    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        if (!platforms[i].active || i == 0)
            continue;
        int px = (int)(platforms[i].x - cameraX + shakeX);
        int py = (int)(platforms[i].y + shakeY);
        if (px + platforms[i].width < 0 || px > SCREEN_W)
            continue;

        /* Platform shadow */
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(px + 4, py + 4, px + platforms[i].width + 4, py + platforms[i].height + 4);

        /* Platform body */
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        bar(px, py, px + platforms[i].width, py + platforms[i].height);

        /* 3D edges */
        setcolor(WHITE);
        line(px, py, px + platforms[i].width, py);
        line(px, py, px, py + platforms[i].height);
        setcolor(DARKGRAY);
        line(px, py + platforms[i].height, px + platforms[i].width, py + platforms[i].height);
        line(px + platforms[i].width, py, px + platforms[i].width, py + platforms[i].height);

        /* Platform texture */
        setcolor(DARKGRAY);
        for (j = px + 10; j < px + platforms[i].width - 5; j += 20)
        {
            line(j, py + 2, j, py + platforms[i].height - 2);
        }

        /* Grass on top of platform */
        setcolor(GREEN);
        for (j = px; j < px + platforms[i].width; j += 6)
        {
            line(j, py, j + 1, py - 3);
            line(j + 3, py, j + 2, py - 2);
        }
    }
}

void renderBullets(void)
{
    int i;
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
            continue;
        int bx = (int)(bullets[i].x - cameraX + shakeX);
        int by = (int)(bullets[i].y + shakeY);
        if (bx < 0 || bx > SCREEN_W)
            continue;

        if (bullets[i].isRocket)
        {
            /* Rocket with flame trail */
            setfillstyle(SOLID_FILL, DARKGRAY);
            fillellipse(bx, by, 6, 3);
            setfillstyle(SOLID_FILL, RED);
            fillellipse(bx, by, 4, 2);
            /* Rocket flame */
            int flameDir = (bullets[i].vx > 0) ? -1 : 1;
            setcolor(YELLOW);
            line(bx + flameDir * 6, by, bx + flameDir * (10 + rand() % 5), by);
            setcolor(RED);
            line(bx + flameDir * 6, by - 1, bx + flameDir * (8 + rand() % 4), by - 1);
            line(bx + flameDir * 6, by + 1, bx + flameDir * (8 + rand() % 4), by + 1);
            /* Smoke trail */
            setcolor(LIGHTGRAY);
            int tx = bx - (int)(bullets[i].vx * 3);
            int ty = by - (int)(bullets[i].vy * 3);
            putpixel(tx + rand() % 4 - 2, ty + rand() % 4 - 2, LIGHTGRAY);
            putpixel(tx + rand() % 6 - 3, ty + rand() % 6 - 3, DARKGRAY);
        }
        else
        {
            /* Normal bullet with glow */
            setcolor(WHITE);
            putpixel(bx, by - 1, WHITE);
            putpixel(bx, by + 1, WHITE);
            setfillstyle(SOLID_FILL, bullets[i].color);
            fillellipse(bx, by, 2, 2);
            /* Trail */
            setcolor(YELLOW);
            int tx1 = bx - (int)(bullets[i].vx * 1.5);
            int ty1 = by - (int)(bullets[i].vy * 1.5);
            line(bx, by, tx1, ty1);
            setcolor(LIGHTGRAY);
            int tx2 = bx - (int)(bullets[i].vx * 3);
            int ty2 = by - (int)(bullets[i].vy * 3);
            line(tx1, ty1, tx2, ty2);
        }
    }
}

void renderGrenades(void)
{
    int i;
    for (i = 0; i < MAX_GRENADES; i++)
    {
        if (!grenades[i].active || grenades[i].exploded)
            continue;
        int gx = (int)(grenades[i].x - cameraX + shakeX);
        int gy = (int)(grenades[i].y + shakeY);
        if (gx < 0 || gx > SCREEN_W)
            continue;

        /* Grenade shadow */
        setfillstyle(SOLID_FILL, DARKGRAY);
        fillellipse(gx + 2, gy + 3, 5, 3);

        /* Grenade body with 3D effect */
        setfillstyle(SOLID_FILL, DARKGRAY);
        fillellipse(gx, gy, 6, 6);
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        fillellipse(gx - 1, gy - 1, 4, 4);
        /* Highlight */
        setcolor(WHITE);
        putpixel(gx - 2, gy - 2, WHITE);
        putpixel(gx - 1, gy - 3, WHITE);

        /* Grenade top/handle */
        setfillstyle(SOLID_FILL, BROWN);
        bar(gx - 2, gy - 9, gx + 2, gy - 5);

        /* Fuse */
        setcolor(YELLOW);
        line(gx, gy - 9, gx, gy - 12);

        /* Danger indicator when about to explode */
        if (grenades[i].timer < 60)
        {
            int pulseSize = (grenades[i].timer < 30) ? (frameCount % 4 < 2 ? 3 : 0) : (frameCount % 8 < 4 ? 2 : 0);
            if (pulseSize > 0)
            {
                setcolor(RED);
                circle(gx, gy, 8 + pulseSize);
            }
            /* Sparking fuse */
            setcolor((frameCount % 4 < 2) ? YELLOW : RED);
            fillellipse(gx, gy - 12, 2 + rand() % 2, 2 + rand() % 2);
            /* Fuse sparks */
            setcolor(YELLOW);
            putpixel(gx + rand() % 5 - 2, gy - 13 - rand() % 3, YELLOW);
            putpixel(gx + rand() % 5 - 2, gy - 11 - rand() % 3, RED);
        }
    }
}

void renderEffects(void)
{
    int i, j;
    for (i = 0; i < MAX_EFFECTS; i++)
    {
        if (!effects[i].active)
            continue;
        int ex = (int)(effects[i].x - cameraX + shakeX);
        int ey = (int)(effects[i].y + shakeY);
        if (ex < 0 || ex > SCREEN_W)
            continue;

        switch (effects[i].type)
        {
        case 0: /* Muzzle flash - enhanced */
        {
            int size = 6 - effects[i].timer;
            if (size > 0)
            {
                setfillstyle(SOLID_FILL, WHITE);
                fillellipse(ex, ey, size, size / 2);
                setfillstyle(SOLID_FILL, YELLOW);
                fillellipse(ex, ey, size - 1, size / 3);
                setcolor(YELLOW);
                line(ex, ey, ex + (rand() % 8 - 4), ey + (rand() % 6 - 3));
            }
        }
        break;
        case 1: /* Impact sparks - enhanced */
            setcolor(effects[i].color);
            for (j = 0; j < 8; j++)
            {
                int sx = ex + rand() % 14 - 7;
                int sy = ey + rand() % 14 - 7;
                putpixel(sx, sy, effects[i].color);
                putpixel(sx + 1, sy, DARKGRAY);
            }
            /* Small debris */
            setcolor(BROWN);
            for (j = 0; j < 3; j++)
            {
                putpixel(ex + rand() % 10 - 5, ey + rand() % 10 - 5, BROWN);
            }
            break;
        case 2: /* Explosion - massively enhanced */
        {
            int t = effects[i].timer;
            int r = 20 - t;
            if (r > 0)
            {
                /* Outer shockwave ring */
                if (t > 5)
                {
                    setcolor(LIGHTGRAY);
                    circle(ex, ey, r + 8);
                    circle(ex, ey, r + 10);
                }

                /* Multiple fire layers */
                if (t > 8)
                {
                    setfillstyle(SOLID_FILL, RED);
                    fillellipse(ex, ey, r, r);
                }
                if (t > 5)
                {
                    setfillstyle(SOLID_FILL, LIGHTRED);
                    fillellipse(ex, ey, r - 3, r - 3);
                }
                if (t > 2)
                {
                    setfillstyle(SOLID_FILL, YELLOW);
                    fillellipse(ex, ey, r / 2, r / 2);
                }

                /* Central white flash */
                if (t > 12)
                {
                    setfillstyle(SOLID_FILL, WHITE);
                    fillellipse(ex, ey, r / 3, r / 3);
                }

                /* Outer spark rays */
                setcolor((t % 2) ? YELLOW : RED);
                for (j = 0; j < 6; j++)
                {
                    int angle = j * 60 + t * 5;
                    int len = r + 5 + rand() % 5;
                    int rx = ex + (int)(cos(angle * PI / 180) * len);
                    int ry = ey + (int)(sin(angle * PI / 180) * len);
                    line(ex + (int)(cos(angle * PI / 180) * (r / 2)),
                         ey + (int)(sin(angle * PI / 180) * (r / 2)), rx, ry);
                }

                /* Smoke puffs */
                if (t < 10)
                {
                    setfillstyle(SOLID_FILL, DARKGRAY);
                    fillellipse(ex + rand() % 10 - 5, ey - 5 - rand() % 5, 4, 3);
                    fillellipse(ex + rand() % 10 - 5, ey - 8 - rand() % 5, 3, 2);
                }
            }
        }
        break;
        }
    }
}

void renderParticles(void)
{
    int i;
    for (i = 0; i < MAX_PARTICLES; i++)
    {
        if (!particles[i].active)
            continue;
        int px = (int)(particles[i].x - cameraX + shakeX);
        int py = (int)(particles[i].y + shakeY);
        if (px < 0 || px > SCREEN_W)
            continue;
        setfillstyle(SOLID_FILL, particles[i].color);
        fillellipse(px, py, particles[i].size, particles[i].size);
    }
}

void renderPickups(void)
{
    int i, j;
    int colors[4] = {GREEN, YELLOW, MAGENTA, CYAN};
    int darkColors[4] = {2, 6, 5, 3}; /* Darker versions */
    char *labels[4] = {"+", "A", "W", "J"};

    for (i = 0; i < MAX_PICKUPS; i++)
    {
        if (!pickups[i].active)
            continue;
        int px = (int)(pickups[i].x - cameraX + shakeX);
        int py = (int)(pickups[i].y + pickups[i].floatOffset + shakeY);
        if (px < 0 || px > SCREEN_W)
            continue;

        /* Pulsing glow effect */
        int pulseSize = 2 + (int)(sin(frameCount * 0.15 + i) * 2);
        setcolor(colors[pickups[i].type]);
        circle(px, py, 12 + pulseSize);

        /* Shadow */
        setfillstyle(SOLID_FILL, DARKGRAY);
        fillellipse(px + 3, py + 5, 8, 4);

        /* Outer ring */
        setfillstyle(SOLID_FILL, darkColors[pickups[i].type]);
        fillellipse(px, py, 12, 12);

        /* Main pickup body */
        setfillstyle(SOLID_FILL, colors[pickups[i].type]);
        fillellipse(px, py, 10, 10);

        /* 3D highlight */
        setfillstyle(SOLID_FILL, WHITE);
        fillellipse(px - 3, py - 3, 3, 3);

        /* Rotating sparkles */
        setcolor(WHITE);
        for (j = 0; j < 4; j++)
        {
            int angle = (frameCount * 3 + j * 90 + i * 45) % 360;
            int sx = px + (int)(cos(angle * PI / 180) * 14);
            int sy = py + (int)(sin(angle * PI / 180) * 14);
            putpixel(sx, sy, WHITE);
        }

        /* Label with shadow */
        setcolor(BLACK);
        outtextxy(px - 3, py - 3, labels[pickups[i].type]);
        setcolor(WHITE);
        outtextxy(px - 4, py - 4, labels[pickups[i].type]);
    }
}

void renderMinimap(void)
{
    int i, mx = SCREEN_W - 115, my = 48;

    /* Minimap frame with 3D effect */
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(mx + 2, my + 2, mx + 107, my + 40);
    setfillstyle(SOLID_FILL, BLACK);
    bar(mx, my, mx + 105, my + 38);

    /* Gradient background */
    setcolor(1); /* Dark blue */
    for (i = 0; i < 36; i += 2)
    {
        line(mx + 1, my + 1 + i, mx + 104, my + 1 + i);
    }

    /* Border */
    setcolor(LIGHTGRAY);
    rectangle(mx, my, mx + 105, my + 38);
    setcolor(WHITE);
    line(mx, my, mx + 105, my);
    line(mx, my, mx, my + 38);

    /* Title */
    setcolor(CYAN);
    outtextxy(mx + 35, my + 2, "MAP");

    /* Simplified terrain */
    setcolor(GREEN);
    line(mx + 5, my + 30, mx + 100, my + 30);

    /* Players as colored dots */
    for (i = 0; i < numPlayers; i++)
    {
        if (!players[i].alive)
            continue;
        int px = mx + 5 + (int)(players[i].x / 13);
        int py = my + 20;

        /* Player dot with ring for current player */
        setfillstyle(SOLID_FILL, players[i].color);
        fillellipse(px, py, 3, 3);
        if (i == playerIndex)
        {
            setcolor(WHITE);
            circle(px, py, 5);
        }
    }

    /* Camera view indicator */
    setcolor(WHITE);
    int camStart = mx + 5 + (int)(cameraX / 13);
    int camEnd = camStart + (int)(SCREEN_W / 13);
    line(camStart, my + 35, camEnd, my + 35);
}

void renderKillfeed(void)
{
    int i, y = 52;

    /* Killfeed background if there are entries */
    int count = 0;
    for (i = 0; i < MAX_KILLFEED; i++)
        if (killfeed[i].active)
            count++;

    if (count > 0)
    {
        setfillstyle(SOLID_FILL, BLACK);
        bar(5, 48, 130, 52 + count * 14);
        setcolor(DARKGRAY);
        rectangle(5, 48, 130, 52 + count * 14);
    }

    for (i = 0; i < MAX_KILLFEED; i++)
    {
        if (!killfeed[i].active)
            continue;
        char buf[40];

        /* Killer name */
        setcolor(players[killfeed[i].killer].color);
        outtextxy(10, y, players[killfeed[i].killer].name);

        /* Arrow */
        setcolor(WHITE);
        outtextxy(50, y, "->");

        /* Victim name */
        setcolor(players[killfeed[i].victim].color);
        outtextxy(70, y, players[killfeed[i].victim].name);

        y += 14;
    }
}

void renderUI(void)
{
    struct Player *p = &players[playerIndex];
    char buf[60];
    int i;

    /* Premium HUD background with gradient */
    setfillstyle(SOLID_FILL, 8);
    bar(0, 0, SCREEN_W, 50);
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(0, 2, SCREEN_W, 48);
    setfillstyle(SOLID_FILL, BLACK);
    bar(0, 4, SCREEN_W, 46);

    /* Bottom border with animated glow */
    int glowCol = (frameCount / 5) % 2 ? BLUE : 9;
    setcolor(1);
    line(0, 47, SCREEN_W, 47);
    setcolor(glowCol);
    line(0, 48, SCREEN_W, 48);
    setcolor(LIGHTBLUE);
    line(0, 49, SCREEN_W, 49);

    /* === LEFT PANEL: Health & Jetpack === */
    /* HP Icon - animated heart */
    int heartPulse = (p->hp <= 25) ? (int)(sin(frameCount * 0.3) * 2) : 0;
    setfillstyle(SOLID_FILL, 4); /* Dark red */
    fillellipse(16, 14 + heartPulse, 7, 6);
    fillellipse(22, 14 + heartPulse, 7, 6);
    setfillstyle(SOLID_FILL, RED);
    fillellipse(16, 13 + heartPulse, 6, 5);
    fillellipse(22, 13 + heartPulse, 6, 5);
    /* Heart point */
    int hpx[] = {10, 19, 28};
    int hpy[] = {14 + heartPulse, 24 + heartPulse, 14 + heartPulse};
    setfillstyle(SOLID_FILL, RED);
    fillpoly(3, hpx);

    /* HP Bar with gradient */
    int hpWidth = (int)((float)p->hp / p->maxHp * 75);
    int hpColor = (p->hp > 60) ? GREEN : (p->hp > 30) ? YELLOW : RED;
    int hpLightColor = (p->hp > 60) ? LIGHTGREEN : (p->hp > 30) ? YELLOW : LIGHTRED;
    
    /* Bar background */
    setfillstyle(SOLID_FILL, 1);
    bar(32, 6, 110, 20);
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(33, 7, 109, 19);
    /* Bar fill */
    if (hpWidth > 0)
    {
        setfillstyle(SOLID_FILL, hpColor);
        bar(34, 8, 34 + hpWidth, 18);
        /* Highlight */
        setfillstyle(SOLID_FILL, hpLightColor);
        bar(34, 8, 34 + hpWidth, 11);
    }
    /* HP segments */
    setcolor(BLACK);
    for (i = 34; i < 108; i += 15)
        line(i, 8, i, 18);
    /* Border */
    setcolor(hpLightColor);
    rectangle(32, 6, 110, 20);
    /* HP number */
    setcolor(WHITE);
    sprintf(buf, "%d", p->hp);
    outtextxy(113, 9, buf);

    /* Jetpack Icon - fuel tank */
    setfillstyle(SOLID_FILL, 1);
    bar(11, 26, 27, 42);
    setfillstyle(SOLID_FILL, CYAN);
    int fuelHeight = (int)((float)p->jetpackFuel / p->maxJetpackFuel * 14);
    bar(12, 41 - fuelHeight, 26, 41);
    setcolor(LIGHTCYAN);
    rectangle(11, 26, 27, 42);
    /* Flame */
    if (p->usingJetpack)
    {
        setcolor(YELLOW);
        line(19, 42, 15, 48);
        line(19, 42, 23, 48);
    }

    /* Jetpack Bar */
    int jetWidth = (int)((float)p->jetpackFuel / p->maxJetpackFuel * 75);
    setfillstyle(SOLID_FILL, 1);
    bar(32, 26, 110, 40);
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(33, 27, 109, 39);
    if (jetWidth > 0)
    {
        setfillstyle(SOLID_FILL, 3);
        bar(34, 28, 34 + jetWidth, 38);
        setfillstyle(SOLID_FILL, CYAN);
        bar(34, 28, 34 + jetWidth, 32);
    }
    setcolor(LIGHTCYAN);
    rectangle(32, 26, 110, 40);

    /* === CENTER-LEFT PANEL: Weapon & Ammo === */
    /* Weapon box */
    setfillstyle(SOLID_FILL, 1);
    bar(125, 3, 225, 47);
    setfillstyle(SOLID_FILL, 8);
    bar(127, 5, 223, 45);
    setcolor(YELLOW);
    rectangle(125, 3, 225, 47);

    /* Weapon-specific icons */
    int wType = p->weapon.type;
    if (wType == WP_PISTOL)
    {
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        bar(133, 20, 155, 26);
        bar(140, 26, 148, 35);
    }
    else if (wType == WP_RIFLE)
    {
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        bar(130, 21, 160, 25);
        bar(145, 25, 153, 34);
        bar(133, 19, 140, 22);
    }
    else if (wType == WP_SHOTGUN)
    {
        setfillstyle(SOLID_FILL, BROWN);
        bar(130, 20, 158, 24);
        bar(130, 24, 158, 28);
        bar(145, 28, 152, 35);
    }
    else if (wType == WP_SNIPER)
    {
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(128, 22, 165, 25);
        setfillstyle(SOLID_FILL, CYAN);
        fillellipse(135, 18, 4, 4);
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(145, 25, 152, 34);
    }
    else /* ROCKET */
    {
        setfillstyle(SOLID_FILL, 2);
        bar(130, 18, 160, 28);
        setfillstyle(SOLID_FILL, RED);
        fillellipse(160, 23, 5, 5);
        setfillstyle(SOLID_FILL, 2);
        bar(145, 28, 152, 36);
    }

    /* Weapon name */
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(170, 8, weaponNames[p->weapon.type]);

    /* Ammo count with color coding */
    int ammoRatio = (p->weapon.ammo * 100) / p->weapon.maxAmmo;
    setcolor((ammoRatio > 50) ? YELLOW : (ammoRatio > 25) ? 6 : RED);
    sprintf(buf, "%d/%d", p->weapon.ammo, p->weapon.maxAmmo);
    outtextxy(170, 22, buf);
    
    /* Bullet icons */
    int bulletsToShow = (p->weapon.ammo > 5) ? 5 : p->weapon.ammo;
    for (i = 0; i < bulletsToShow; i++)
    {
        setfillstyle(SOLID_FILL, YELLOW);
        fillellipse(175 + i * 10, 38, 2, 5);
        setfillstyle(SOLID_FILL, 6);
        fillellipse(175 + i * 10, 35, 2, 2);
    }

    /* === CENTER PANEL: Score & Timer === */
    /* Kill score box */
    setfillstyle(SOLID_FILL, 1);
    bar(235, 3, 318, 46);
    setfillstyle(SOLID_FILL, 2);
    bar(237, 5, 316, 25);
    setcolor(LIGHTGREEN);
    rectangle(235, 3, 318, 46);
    
    /* Kills text */
    setcolor(WHITE);
    sprintf(buf, "KILLS %d/%d", p->kills, KILLS_TO_WIN);
    outtextxy(245, 9, buf);

    /* Progress bar to win */
    int killProgress = (int)((float)p->kills / KILLS_TO_WIN * 75);
    setfillstyle(SOLID_FILL, 1);
    bar(240, 28, 313, 38);
    if (killProgress > 0)
    {
        setfillstyle(SOLID_FILL, 2);
        bar(241, 29, 241 + killProgress, 37);
        setfillstyle(SOLID_FILL, LIGHTGREEN);
        bar(241, 29, 241 + killProgress, 33);
    }
    setcolor(WHITE);
    rectangle(240, 28, 313, 38);

    /* Deaths with skull */
    setcolor(LIGHTRED);
    sprintf(buf, "D:%d", p->deaths);
    outtextxy(245, 40, buf);

    /* Streak indicator with fire */
    if (p->streak >= 3)
    {
        /* Fire effect behind streak */
        int flicker = (frameCount / 3) % 3;
        setcolor((flicker == 0) ? RED : (flicker == 1) ? YELLOW : LIGHTRED);
        fillellipse(295, 40, 4 + flicker, 4);
        
        setcolor(YELLOW);
        sprintf(buf, "x%d!", p->streak);
        outtextxy(285, 40, buf);
    }

    /* === CENTER-RIGHT: Timer === */
    int timeLeft = (TIME_LIMIT - matchTime) / 30;
    int mins = timeLeft / 60;
    int secs = timeLeft % 60;
    int urgent = (timeLeft < 30);

    /* Timer box */
    setfillstyle(SOLID_FILL, 1);
    bar(325, 5, 395, 45);
    setfillstyle(SOLID_FILL, urgent ? 4 : 8);
    bar(327, 7, 393, 43);
    setcolor(urgent ? RED : LIGHTGRAY);
    rectangle(325, 5, 395, 45);

    /* Clock icon */
    setcolor(WHITE);
    circle(342, 25, 10);
    setfillstyle(SOLID_FILL, BLACK);
    fillellipse(342, 25, 8, 8);
    setcolor(WHITE);
    line(342, 25, 342, 18);
    line(342, 25, 348, 25);
    /* Clock center */
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(342, 25, 2, 2);

    /* Time display */
    setcolor(urgent ? ((frameCount / 8) % 2 ? RED : YELLOW) : WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    sprintf(buf, "%d:%02d", mins, secs);
    outtextxy(358, 14, buf);

    /* Urgency indicator */
    if (urgent)
    {
        setcolor(RED);
        outtextxy(358, 30, "HURRY!");
    }

    /* === RIGHT PANEL: Status === */
    /* Sound toggle */
    setfillstyle(SOLID_FILL, soundEnabled ? 2 : 4);
    bar(405, 6, 455, 24);
    setcolor(WHITE);
    rectangle(405, 6, 455, 24);
    setcolor(WHITE);
    outtextxy(412, 10, soundEnabled ? "SND ON" : "MUTED");

    /* Goal reminder */
    setcolor(YELLOW);
    outtextxy(405, 30, "Goal:10K");

    /* Controls - compact */
    setcolor(DARKGRAY);
    outtextxy(465, 10, "1-5:Wpn");
    outtextxy(465, 25, "G:Gren");
    outtextxy(465, 40, "S:Snd");

    renderMinimap();
    renderKillfeed();

    /* Enhanced respawn message */
    if (!p->alive && p->respawnTimer > 0)
    {
        /* Dark overlay with vignette */
        setfillstyle(SOLID_FILL, BLACK);
        bar(SCREEN_W / 2 - 95, SCREEN_H / 2 - 30, SCREEN_W / 2 + 95, SCREEN_H / 2 + 30);
        setfillstyle(SOLID_FILL, 4);
        bar(SCREEN_W / 2 - 90, SCREEN_H / 2 - 25, SCREEN_W / 2 + 90, SCREEN_H / 2 + 25);
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(SCREEN_W / 2 - 85, SCREEN_H / 2 - 20, SCREEN_W / 2 + 85, SCREEN_H / 2 + 20);
        
        /* Animated border */
        int pulseCol = (frameCount / 5) % 2 ? RED : LIGHTRED;
        setcolor(pulseCol);
        rectangle(SCREEN_W / 2 - 90, SCREEN_H / 2 - 25, SCREEN_W / 2 + 90, SCREEN_H / 2 + 25);

        /* Skull icon */
        setfillstyle(SOLID_FILL, WHITE);
        fillellipse(SCREEN_W / 2, SCREEN_H / 2 - 10, 10, 8);
        setfillstyle(SOLID_FILL, BLACK);
        fillellipse(SCREEN_W / 2 - 4, SCREEN_H / 2 - 12, 3, 3);
        fillellipse(SCREEN_W / 2 + 4, SCREEN_H / 2 - 12, 3, 3);

        setcolor(WHITE);
        outtextxy(SCREEN_W / 2 - 35, SCREEN_H / 2, "YOU DIED!");
        
        setcolor(YELLOW);
        int countdown = p->respawnTimer / 30 + 1;
        sprintf(buf, "Respawn: %d", countdown);
        outtextxy(SCREEN_W / 2 - 40, SCREEN_H / 2 + 12, buf);
    }
}

/* =============== DRAWING FUNCTIONS =============== */
void drawSplash(void)
{
    int i, j;

    /* Premium gradient background - multiple layers */
    for (i = 0; i < SCREEN_H; i += 5)
    {
        int shade = (i < 60) ? 1 : (i < 120) ? BLUE : (i < 200) ? 9 : BLACK;
        setfillstyle(SOLID_FILL, shade);
        bar(0, i, SCREEN_W, i + 5);
    }

    /* Nebula effect in background */
    for (i = 0; i < 25; i++)
    {
        int nx = (i * 31 + frameCount / 4) % SCREEN_W;
        int ny = 40 + (i * 17) % 100;
        setcolor((i % 2) ? 9 : 1);
        circle(nx, ny, 15 + (i % 10));
    }

    /* Animated twinkling stars - multiple layers */
    for (i = 0; i < 100; i++)
    {
        int sx = (i * 7 + frameCount / 3) % SCREEN_W;
        int sy = (i * 5) % 160;
        int twinkle = (i + frameCount / 2) % 5;
        int col = (twinkle < 2) ? WHITE : (twinkle < 4) ? LIGHTGRAY : CYAN;
        putpixel(sx, sy, col);
        if (i % 5 == 0 && twinkle == 0)
        {
            /* Star cross effect for brighter stars */
            putpixel(sx - 1, sy, LIGHTGRAY);
            putpixel(sx + 1, sy, LIGHTGRAY);
            putpixel(sx, sy - 1, LIGHTGRAY);
            putpixel(sx, sy + 1, LIGHTGRAY);
        }
    }

    /* Animated energy particles around logo area */
    for (i = 0; i < 20; i++)
    {
        int angle = (frameCount * 2 + i * 18) % 360;
        int radius = 100 + (int)(sin((frameCount + i * 10) * 0.05) * 20);
        int px = SCREEN_W / 2 + (int)(cos(angle * PI / 180) * radius);
        int py = 130 + (int)(sin(angle * PI / 180) * 40);
        int col = (i % 3 == 0) ? YELLOW : (i % 3 == 1) ? RED : LIGHTRED;
        setfillstyle(SOLID_FILL, col);
        fillellipse(px, py, 2, 2);
        /* Trail effect */
        int trailAngle = angle - 15;
        int tx = SCREEN_W / 2 + (int)(cos(trailAngle * PI / 180) * radius);
        int ty = 130 + (int)(sin(trailAngle * PI / 180) * 40);
        setcolor(DARKGRAY);
        line(px, py, tx, ty);
    }

    /* Animated fire wave at bottom */
    for (i = 0; i < SCREEN_W; i += 3)
    {
        int fh = 15 + (int)(sin((i + frameCount * 3) * 0.05) * 10) + rand() % 10;
        /* Outer flame */
        setcolor(4); /* Dark red */
        line(i, SCREEN_H, i, SCREEN_H - fh - 5);
        /* Middle flame */
        setcolor(RED);
        line(i, SCREEN_H, i, SCREEN_H - fh);
        /* Inner flame */
        setcolor(LIGHTRED);
        line(i + 1, SCREEN_H, i + 1, SCREEN_H - fh + 5);
        /* Core flame */
        if (i % 6 == 0)
        {
            setcolor(YELLOW);
            line(i, SCREEN_H - fh + 8, i, SCREEN_H - fh + 3);
        }
    }

    /* Rising fire particles */
    for (i = 0; i < 50; i++)
    {
        int fx = (i * 13 + frameCount * 3) % SCREEN_W;
        int fy = SCREEN_H - 40 - (frameCount * 2 + i * 7) % 80;
        int fc = (i + frameCount) % 4;
        int col = (fc == 0) ? RED : (fc == 1) ? YELLOW : (fc == 2) ? LIGHTRED : 6;
        setcolor(col);
        putpixel(fx, fy, col);
        putpixel(fx, fy - 1, (fc < 2) ? YELLOW : LIGHTRED);
    }

    /* Logo floating animation */
    int logoY = 75 + (int)(sin(frameCount * 0.04) * 6);

    /* Multiple shadow layers for 3D depth */
    setcolor(4); /* Dark red */
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 4);
    outtextxy(SCREEN_W / 2 - 92, logoY + 5, "WARZONE");
    
    setcolor(DARKGRAY);
    outtextxy(SCREEN_W / 2 - 94, logoY + 3, "WARZONE");

    /* Outer glow effect */
    setcolor(LIGHTRED);
    outtextxy(SCREEN_W / 2 - 98, logoY - 2, "WARZONE");
    outtextxy(SCREEN_W / 2 - 94, logoY - 2, "WARZONE");

    /* Main logo with highlight */
    setcolor(RED);
    outtextxy(SCREEN_W / 2 - 96, logoY, "WARZONE");
    
    /* Top highlight for 3D effect */
    setcolor(LIGHTRED);
    outtextxy(SCREEN_W / 2 - 96, logoY - 1, "WARZONE");

    /* ELITE with enhanced glow */
    setcolor(6); /* Brown/Orange */
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(SCREEN_W / 2 - 48, logoY + 44, "ELITE");
    setcolor(BROWN);
    outtextxy(SCREEN_W / 2 - 50, logoY + 42, "ELITE");
    setcolor(YELLOW);
    outtextxy(SCREEN_W / 2 - 49, logoY + 40, "ELITE");
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 49, logoY + 39, "ELITE");

    /* Decorative lines with gradient */
    setcolor(4);
    line(SCREEN_W / 2 - 130, logoY + 72, SCREEN_W / 2 + 130, logoY + 72);
    setcolor(RED);
    line(SCREEN_W / 2 - 120, logoY + 75, SCREEN_W / 2 + 120, logoY + 75);
    setcolor(LIGHTRED);
    line(SCREEN_W / 2 - 100, logoY + 78, SCREEN_W / 2 + 100, logoY + 78);
    setcolor(YELLOW);
    line(SCREEN_W / 2 - 80, logoY + 81, SCREEN_W / 2 + 80, logoY + 81);
    setcolor(WHITE);
    line(SCREEN_W / 2 - 50, logoY + 84, SCREEN_W / 2 + 50, logoY + 84);

    /* Diamond decorations on line */
    setfillstyle(SOLID_FILL, YELLOW);
    int dx1[] = {SCREEN_W/2 - 125, SCREEN_W/2 - 120, SCREEN_W/2 - 125, SCREEN_W/2 - 130};
    int dy1[] = {logoY + 72, logoY + 77, logoY + 82, logoY + 77};
    fillpoly(4, dx1);
    int dx2[] = {SCREEN_W/2 + 125, SCREEN_W/2 + 130, SCREEN_W/2 + 125, SCREEN_W/2 + 120};
    int dy2[] = {logoY + 72, logoY + 77, logoY + 82, logoY + 77};
    fillpoly(4, dx2);

    /* Subtitle with glow */
    setcolor(DARKGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(SCREEN_W / 2 - 91, logoY + 98, "THE ULTIMATE 2D BATTLE ARENA");
    setcolor(LIGHTGRAY);
    outtextxy(SCREEN_W / 2 - 90, logoY + 97, "THE ULTIMATE 2D BATTLE ARENA");
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 90, logoY + 96, "THE ULTIMATE 2D BATTLE ARENA");

    /* Animated loading bar */
    int loadWidth = (frameCount * 3) % 160;
    int loadY = 225;
    /* Bar background */
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(SCREEN_W / 2 - 82, loadY, SCREEN_W / 2 + 82, loadY + 12);
    /* Bar fill with gradient effect */
    if (loadWidth > 0)
    {
        setfillstyle(SOLID_FILL, 4);
        bar(SCREEN_W / 2 - 80, loadY + 2, SCREEN_W / 2 - 80 + loadWidth, loadY + 10);
        setfillstyle(SOLID_FILL, RED);
        bar(SCREEN_W / 2 - 80, loadY + 2, SCREEN_W / 2 - 80 + loadWidth - 5, loadY + 8);
        setfillstyle(SOLID_FILL, LIGHTRED);
        bar(SCREEN_W / 2 - 80, loadY + 3, SCREEN_W / 2 - 80 + loadWidth - 10, loadY + 6);
    }
    /* Bar border */
    setcolor(RED);
    rectangle(SCREEN_W / 2 - 82, loadY, SCREEN_W / 2 + 82, loadY + 12);
    setcolor(YELLOW);
    rectangle(SCREEN_W / 2 - 81, loadY + 1, SCREEN_W / 2 + 81, loadY + 11);

    /* Animated "Press Any Key" with pulse effect */
    int pulse = (int)(sin(frameCount * 0.15) * 2);
    if ((frameCount / 15) % 2 == 0)
    {
        setcolor(DARKGRAY);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(SCREEN_W / 2 - 78 + pulse, 252, "PRESS ANY KEY");
        setcolor(WHITE);
        outtextxy(SCREEN_W / 2 - 80 + pulse, 250, "PRESS ANY KEY");

        /* Animated arrows */
        setcolor(YELLOW);
        int arrowAnim = (frameCount / 3) % 15;
        outtextxy(SCREEN_W / 2 - 110 + arrowAnim, 250, ">>>");
        outtextxy(SCREEN_W / 2 + 90 - arrowAnim, 250, "<<<");
    }

    /* Version with style */
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(SCREEN_W / 2 - 45, SCREEN_H - 28, SCREEN_W / 2 + 45, SCREEN_H - 12);
    setcolor(LIGHTGRAY);
    rectangle(SCREEN_W / 2 - 45, SCREEN_H - 28, SCREEN_W / 2 + 45, SCREEN_H - 12);
    setcolor(CYAN);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(SCREEN_W / 2 - 35, SCREEN_H - 24, "Version 3.0");

    /* Golden corner decorations */
    /* Top-left */
    setcolor(YELLOW);
    line(0, 0, 60, 0);
    line(0, 0, 0, 60);
    line(0, 3, 50, 3);
    line(3, 0, 3, 50);
    setcolor(BROWN);
    line(0, 6, 40, 6);
    line(6, 0, 6, 40);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(8, 8, 4, 4);

    /* Top-right */
    setcolor(YELLOW);
    line(SCREEN_W - 60, 0, SCREEN_W, 0);
    line(SCREEN_W - 1, 0, SCREEN_W - 1, 60);
    line(SCREEN_W - 50, 3, SCREEN_W, 3);
    line(SCREEN_W - 4, 0, SCREEN_W - 4, 50);
    setcolor(BROWN);
    line(SCREEN_W - 40, 6, SCREEN_W, 6);
    line(SCREEN_W - 7, 0, SCREEN_W - 7, 40);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(SCREEN_W - 9, 8, 4, 4);

    /* Bottom-left */
    setcolor(YELLOW);
    line(0, SCREEN_H - 60, 0, SCREEN_H);
    line(0, SCREEN_H - 1, 60, SCREEN_H - 1);
    line(3, SCREEN_H - 50, 3, SCREEN_H);
    line(0, SCREEN_H - 4, 50, SCREEN_H - 4);
    setcolor(BROWN);
    line(6, SCREEN_H - 40, 6, SCREEN_H);
    line(0, SCREEN_H - 7, 40, SCREEN_H - 7);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(8, SCREEN_H - 9, 4, 4);

    /* Bottom-right */
    setcolor(YELLOW);
    line(SCREEN_W - 1, SCREEN_H - 60, SCREEN_W - 1, SCREEN_H);
    line(SCREEN_W - 60, SCREEN_H - 1, SCREEN_W, SCREEN_H - 1);
    line(SCREEN_W - 4, SCREEN_H - 50, SCREEN_W - 4, SCREEN_H);
    line(SCREEN_W - 50, SCREEN_H - 4, SCREEN_W, SCREEN_H - 4);
    setcolor(BROWN);
    line(SCREEN_W - 7, SCREEN_H - 40, SCREEN_W - 7, SCREEN_H);
    line(SCREEN_W - 40, SCREEN_H - 7, SCREEN_W, SCREEN_H - 7);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(SCREEN_W - 9, SCREEN_H - 9, 4, 4);
}

void drawMenu(void)
{
    int i, j;

    /* Dark space background with gradient */
    for (i = 0; i < SCREEN_H; i += 8)
    {
        int shade = (i < 80) ? 1 : (i < 160) ? BLUE : (i < 240) ? 9 : BLACK;
        setfillstyle(SOLID_FILL, shade);
        bar(0, i, SCREEN_W, i + 8);
    }

    /* Animated deep space starfield */
    for (i = 0; i < 120; i++)
    {
        int sx = (i * 7 + frameCount / 2) % SCREEN_W;
        int sy = (i * 3) % 150;
        int twinkle = (i + frameCount / 3) % 6;
        int col = (twinkle < 2) ? WHITE : (twinkle < 4) ? LIGHTGRAY : CYAN;
        putpixel(sx, sy, col);
        if (i % 8 == 0 && twinkle == 0)
        {
            putpixel(sx - 1, sy, LIGHTGRAY);
            putpixel(sx + 1, sy, LIGHTGRAY);
        }
    }

    /* Animated explosion effects in background */
    for (i = 0; i < 3; i++)
    {
        int expPhase = (frameCount / 40 + i * 13) % 30;
        if (expPhase < 15)
        {
            int expX = 100 + i * 200;
            int expY = 60 + (i * 31) % 60;
            int expR = expPhase;
            setcolor((expPhase % 3) ? YELLOW : RED);
            circle(expX, expY, expR);
            if (expPhase > 8)
            {
                setcolor(LIGHTRED);
                circle(expX, expY, expR - 4);
            }
        }
    }

    /* War-torn ground with debris and fire */
    setfillstyle(SOLID_FILL, 6); /* Dark brown */
    bar(0, 290, SCREEN_W, SCREEN_H);
    setfillstyle(SOLID_FILL, BROWN);
    bar(0, 290, SCREEN_W, 298);
    
    /* Grass tufts */
    setcolor(2); /* Dark green */
    for (i = 0; i < SCREEN_W; i += 12)
    {
        line(i, 290, i + 2, 284);
        line(i + 4, 290, i + 3, 286);
    }

    /* Fire effect with multiple layers */
    for (i = 0; i < SCREEN_W; i += 4)
    {
        int fh = 8 + (int)(sin((i + frameCount * 4) * 0.08) * 6) + rand() % 8;
        setcolor(4); /* Dark red */
        line(i, 290, i, 290 - fh - 3);
        setcolor(RED);
        line(i, 290, i, 290 - fh);
        setcolor(LIGHTRED);
        line(i + 1, 290, i + 1, 290 - fh + 4);
        if (i % 8 == 0)
        {
            setcolor(YELLOW);
            line(i, 290 - fh + 6, i, 290 - fh + 2);
        }
    }

    /* Flying embers */
    for (i = 0; i < 30; i++)
    {
        int ex = (i * 23 + frameCount * 2) % SCREEN_W;
        int ey = 280 - (frameCount + i * 9) % 100;
        int ec = (i + frameCount) % 3;
        setcolor(ec == 0 ? RED : (ec == 1) ? YELLOW : LIGHTRED);
        putpixel(ex, ey, ec == 0 ? RED : (ec == 1) ? YELLOW : LIGHTRED);
    }

    /* Logo with dramatic 3D effect */
    int logoY = 30 + (int)(sin(frameCount * 0.025) * 4);

    /* Deep shadow */
    setcolor(4);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 4);
    outtextxy(SCREEN_W / 2 - 91, logoY + 6, "WARZONE");
    
    /* Shadow */
    setcolor(DARKGRAY);
    outtextxy(SCREEN_W / 2 - 93, logoY + 4, "WARZONE");

    /* Outer glow */
    setcolor(LIGHTRED);
    outtextxy(SCREEN_W / 2 - 98, logoY - 2, "WARZONE");
    outtextxy(SCREEN_W / 2 - 94, logoY - 2, "WARZONE");

    /* Main text */
    setcolor(RED);
    outtextxy(SCREEN_W / 2 - 96, logoY, "WARZONE");

    /* Highlight */
    setcolor(LIGHTRED);
    outtextxy(SCREEN_W / 2 - 96, logoY - 1, "WARZONE");

    /* ELITE subtitle with glow */
    setcolor(6);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(SCREEN_W / 2 - 47, logoY + 38, "ELITE");
    setcolor(BROWN);
    outtextxy(SCREEN_W / 2 - 49, logoY + 36, "ELITE");
    setcolor(YELLOW);
    outtextxy(SCREEN_W / 2 - 48, logoY + 34, "ELITE");
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 48, logoY + 33, "ELITE");

    /* Animated soldiers - LEFT (GREEN TEAM) */
    int soldierBob = (int)(sin(frameCount * 0.2) * 2);
    int armSwing = (int)(sin(frameCount * 0.15) * 3);
    
    /* Shadow */
    setfillstyle(SOLID_FILL, DARKGRAY);
    fillellipse(105, 255, 20, 5);

    /* Boots */
    setfillstyle(SOLID_FILL, 6);
    fillellipse(88, 248, 6, 4);
    fillellipse(118, 248, 6, 4);
    
    /* Legs */
    setcolor(GREEN);
    line(95, 220, 90, 245);
    line(105, 220, 115, 245);
    
    /* Body with armor */
    setfillstyle(SOLID_FILL, 2); /* Dark green */
    bar(88, 185, 112, 220);
    setfillstyle(SOLID_FILL, GREEN);
    bar(90, 187, 110, 218);
    /* Armor details */
    setcolor(LIGHTGREEN);
    line(90, 195, 110, 195);
    line(90, 205, 110, 205);
    rectangle(94, 198, 106, 212);

    /* Head with helmet */
    setfillstyle(SOLID_FILL, LIGHTGREEN);
    fillellipse(100, 170 + soldierBob, 14, 14);
    /* Helmet */
    setfillstyle(SOLID_FILL, 2);
    arc(100, 170 + soldierBob, 20, 160, 13);
    /* Visor */
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    fillellipse(105, 170 + soldierBob, 6, 4);
    /* Eye */
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(107, 169 + soldierBob, 2, 2);
    setfillstyle(SOLID_FILL, BLACK);
    fillellipse(108, 169 + soldierBob, 1, 1);

    /* Arm with rifle */
    setcolor(LIGHTGREEN);
    line(112, 190, 130 + armSwing, 180);
    /* Rifle */
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(128 + armSwing, 177, 165, 183);
    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(130 + armSwing, 178, 162, 182);
    /* Scope */
    setfillstyle(SOLID_FILL, CYAN);
    fillellipse(145 + armSwing, 175, 3, 2);

    /* Muzzle flash animation */
    if (frameCount % 45 < 8)
    {
        setfillstyle(SOLID_FILL, WHITE);
        fillellipse(167, 180, 4, 4);
        setcolor(YELLOW);
        line(165, 180, 175, 177);
        line(165, 180, 175, 183);
        line(165, 180, 178, 180);
    }

    /* RIGHT SOLDIER (RED TEAM) */
    /* Shadow */
    setfillstyle(SOLID_FILL, DARKGRAY);
    fillellipse(535, 255, 20, 5);

    /* Boots */
    setfillstyle(SOLID_FILL, 6);
    fillellipse(522, 248, 6, 4);
    fillellipse(552, 248, 6, 4);
    
    /* Legs */
    setcolor(RED);
    line(530, 220, 525, 245);
    line(540, 220, 550, 245);
    
    /* Body with armor */
    setfillstyle(SOLID_FILL, 4); /* Dark red */
    bar(523, 185, 547, 220);
    setfillstyle(SOLID_FILL, RED);
    bar(525, 187, 545, 218);
    /* Armor details */
    setcolor(LIGHTRED);
    line(525, 195, 545, 195);
    line(525, 205, 545, 205);
    rectangle(529, 198, 541, 212);

    /* Head with helmet */
    setfillstyle(SOLID_FILL, LIGHTRED);
    fillellipse(535, 170 + soldierBob, 14, 14);
    /* Helmet */
    setfillstyle(SOLID_FILL, 4);
    arc(535, 170 + soldierBob, 20, 160, 13);
    /* Visor */
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    fillellipse(530, 170 + soldierBob, 6, 4);
    /* Eye */
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(528, 169 + soldierBob, 2, 2);
    setfillstyle(SOLID_FILL, BLACK);
    fillellipse(527, 169 + soldierBob, 1, 1);

    /* Arm with rifle */
    setcolor(LIGHTRED);
    line(523, 190, 505 - armSwing, 180);
    /* Rifle */
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(470, 177, 507 - armSwing, 183);
    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(473, 178, 505 - armSwing, 182);
    /* Scope */
    setfillstyle(SOLID_FILL, CYAN);
    fillellipse(490 - armSwing, 175, 3, 2);

    /* Muzzle flash animation */
    if ((frameCount + 22) % 45 < 8)
    {
        setfillstyle(SOLID_FILL, WHITE);
        fillellipse(468, 180, 4, 4);
        setcolor(YELLOW);
        line(470, 180, 460, 177);
        line(470, 180, 460, 183);
        line(470, 180, 457, 180);
    }

    /* Bullet tracers between soldiers */
    if (frameCount % 20 < 10)
    {
        int bx = 170 + (frameCount * 8) % 300;
        setcolor(YELLOW);
        line(bx, 180, bx + 15, 180);
        setcolor(WHITE);
        putpixel(bx + 15, 180, WHITE);
    }
    if ((frameCount + 10) % 20 < 10)
    {
        int bx = 470 - (frameCount * 8) % 300;
        setcolor(YELLOW);
        line(bx, 180, bx - 15, 180);
        setcolor(WHITE);
        putpixel(bx - 15, 180, WHITE);
    }

    /* Main PLAY button with premium glow effect */
    int btnY = 125;
    int btnPulse = (int)(sin(frameCount * 0.08) * 4);

    /* Outer glow rings */
    setcolor(9); /* Light blue */
    rectangle(SCREEN_W / 2 - 88 - btnPulse, btnY - 18 - btnPulse,
              SCREEN_W / 2 + 88 + btnPulse, btnY + 48 + btnPulse);
    setcolor(BLUE);
    rectangle(SCREEN_W / 2 - 85 - btnPulse/2, btnY - 15 - btnPulse/2,
              SCREEN_W / 2 + 85 + btnPulse/2, btnY + 45 + btnPulse/2);

    /* Button shadow */
    setfillstyle(SOLID_FILL, 1);
    bar(SCREEN_W / 2 - 78, btnY - 6, SCREEN_W / 2 + 82, btnY + 42);

    /* Button body with gradient effect */
    setfillstyle(SOLID_FILL, 1); /* Dark blue */
    bar(SCREEN_W / 2 - 80, btnY - 8, SCREEN_W / 2 + 80, btnY + 40);
    setfillstyle(SOLID_FILL, BLUE);
    bar(SCREEN_W / 2 - 78, btnY - 6, SCREEN_W / 2 + 78, btnY + 38);
    setfillstyle(SOLID_FILL, 9); /* Light blue top */
    bar(SCREEN_W / 2 - 78, btnY - 6, SCREEN_W / 2 + 78, btnY + 5);

    /* Button border */
    setcolor(LIGHTBLUE);
    rectangle(SCREEN_W / 2 - 80, btnY - 8, SCREEN_W / 2 + 80, btnY + 40);
    setcolor(WHITE);
    line(SCREEN_W / 2 - 79, btnY - 7, SCREEN_W / 2 + 79, btnY - 7);
    line(SCREEN_W / 2 - 79, btnY - 7, SCREEN_W / 2 - 79, btnY + 39);

    /* Button text */
    setcolor(DARKGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(SCREEN_W / 2 - 62, btnY + 2, "PRESS ENTER");
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 64, btnY, "PRESS ENTER");

    setcolor(6);
    outtextxy(SCREEN_W / 2 - 42, btnY + 22, "TO PLAY");
    setcolor(YELLOW);
    outtextxy(SCREEN_W / 2 - 44, btnY + 20, "TO PLAY");

    /* Feature icons panel */
    int panelY = 185;
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(SCREEN_W / 2 - 95, panelY, SCREEN_W / 2 + 95, panelY + 50);
    setcolor(LIGHTGRAY);
    rectangle(SCREEN_W / 2 - 95, panelY, SCREEN_W / 2 + 95, panelY + 50);

    /* Weapon icon */
    setfillstyle(SOLID_FILL, YELLOW);
    bar(SCREEN_W / 2 - 85, panelY + 8, SCREEN_W / 2 - 65, panelY + 14);
    bar(SCREEN_W / 2 - 80, panelY + 14, SCREEN_W / 2 - 72, panelY + 22);
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(SCREEN_W / 2 - 60, panelY + 8, "5 WEAPONS");

    /* Jetpack icon */
    setfillstyle(SOLID_FILL, CYAN);
    fillellipse(SCREEN_W / 2 - 75, panelY + 37, 6, 4);
    setcolor(YELLOW);
    line(SCREEN_W / 2 - 75, panelY + 41, SCREEN_W / 2 - 78, panelY + 48);
    line(SCREEN_W / 2 - 75, panelY + 41, SCREEN_W / 2 - 72, panelY + 48);
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 60, panelY + 32, "JETPACK");

    /* AI icon */
    setfillstyle(SOLID_FILL, RED);
    fillellipse(SCREEN_W / 2 + 20, panelY + 12, 8, 8);
    setfillstyle(SOLID_FILL, LIGHTRED);
    fillellipse(SCREEN_W / 2 + 35, panelY + 12, 6, 6);
    setfillstyle(SOLID_FILL, 4);
    fillellipse(SCREEN_W / 2 + 47, panelY + 12, 5, 5);
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 + 55, panelY + 8, "4 BOTS");

    /* Difficulty icon */
    setfillstyle(SOLID_FILL, GREEN);
    fillellipse(SCREEN_W / 2 + 20, panelY + 37, 5, 5);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(SCREEN_W / 2 + 33, panelY + 37, 5, 5);
    setfillstyle(SOLID_FILL, RED);
    fillellipse(SCREEN_W / 2 + 46, panelY + 37, 5, 5);
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 + 55, panelY + 32, "3 LVLS");

    /* Controls hint box */
    setfillstyle(SOLID_FILL, BLACK);
    bar(SCREEN_W / 2 - 55, 248, SCREEN_W / 2 + 55, 268);
    setcolor(RED);
    rectangle(SCREEN_W / 2 - 55, 248, SCREEN_W / 2 + 55, 268);
    setcolor(LIGHTGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(SCREEN_W / 2 - 45, 254, "ESC = EXIT");

    /* Version with style */
    setcolor(DARKGRAY);
    outtextxy(SCREEN_W / 2 - 38, 280, "v3.0 - 2025");

    /* Author credit */
    setcolor(8);
    outtextxy(5, SCREEN_H - 15, "By: Yaaqob Al-mahajeri");
}

void drawDifficulty(void)
{
    char *diffNames[3] = {"EASY", "MEDIUM", "HARD"};
    char *diffDesc[3] = {"Relaxed pace - AI 30% accuracy", "Balanced - AI 50% accuracy", "Intense! - AI 70% accuracy"};
    int aiReact[3] = {60, 40, 20};
    int colors[3] = {GREEN, YELLOW, RED};
    int darkColors[3] = {2, 6, 4};
    int i, j;

    /* Gradient background */
    for (i = 0; i < SCREEN_H; i += 8)
    {
        int shade = (i < 100) ? 1 : (i < 200) ? 9 : BLACK;
        setfillstyle(SOLID_FILL, shade);
        bar(0, i, SCREEN_W, i + 8);
    }

    /* Dynamic particles based on current selection */
    for (i = 0; i < 60; i++)
    {
        int px = (i * 13 + frameCount / 2) % SCREEN_W;
        int py = (i * 7 + frameCount / 3) % SCREEN_H;
        int bright = (i + frameCount / 4) % 3;
        int col = (bright == 0) ? colors[difficulty] : (bright == 1) ? WHITE : DARKGRAY;
        putpixel(px, py, col);
    }

    /* Title with 3D effect */
    setcolor(1);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(SCREEN_W / 2 - 115, 18, "SELECT DIFFICULTY");
    setcolor(DARKGRAY);
    outtextxy(SCREEN_W / 2 - 117, 16, "SELECT DIFFICULTY");
    setcolor(colors[difficulty]);
    outtextxy(SCREEN_W / 2 - 118, 15, "SELECT DIFFICULTY");
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 119, 14, "SELECT DIFFICULTY");

    /* Decorative lines with gradient */
    setcolor(DARKGRAY);
    line(SCREEN_W / 2 - 140, 48, SCREEN_W / 2 + 140, 48);
    setcolor(colors[difficulty]);
    line(SCREEN_W / 2 - 130, 50, SCREEN_W / 2 + 130, 50);
    setcolor(WHITE);
    line(SCREEN_W / 2 - 100, 52, SCREEN_W / 2 + 100, 52);

    /* Difficulty buttons with enhanced design */
    for (i = 0; i < 3; i++)
    {
        int y = 70 + i * 70;
        int isSelected = (i == difficulty);
        int pulse = isSelected ? (int)(sin(frameCount * 0.12) * 4) : 0;

        /* Outer glow for selected */
        if (isSelected)
        {
            setcolor(colors[i]);
            rectangle(SCREEN_W / 2 - 125 + pulse/2, y - 7, SCREEN_W / 2 + 125 - pulse/2, y + 52);
        }

        /* Button shadow */
        setfillstyle(SOLID_FILL, 1);
        bar(SCREEN_W / 2 - 116 + pulse, y + 4, SCREEN_W / 2 + 120 - pulse, y + 52);

        /* Button body */
        setfillstyle(SOLID_FILL, isSelected ? colors[i] : darkColors[i]);
        bar(SCREEN_W / 2 - 120 + pulse, y, SCREEN_W / 2 + 116 - pulse, y + 48);

        /* Button gradient highlight */
        if (isSelected)
        {
            setfillstyle(SOLID_FILL, (i == 0) ? LIGHTGREEN : (i == 1) ? YELLOW : LIGHTRED);
            bar(SCREEN_W / 2 - 118 + pulse, y + 2, SCREEN_W / 2 + 114 - pulse, y + 12);
        }

        /* Button border */
        setcolor(isSelected ? WHITE : LIGHTGRAY);
        rectangle(SCREEN_W / 2 - 120 + pulse, y, SCREEN_W / 2 + 116 - pulse, y + 48);

        /* Animated selection indicators */
        if (isSelected)
        {
            setcolor(WHITE);
            int arrowAnim = (frameCount / 4) % 8;
            outtextxy(SCREEN_W / 2 - 145 + arrowAnim, y + 15, ">>>");
            outtextxy(SCREEN_W / 2 + 125 - arrowAnim, y + 15, "<<<");
        }

        /* Difficulty icon */
        int iconX = SCREEN_W / 2 - 105 + pulse;
        for (j = 0; j <= i; j++)
        {
            setfillstyle(SOLID_FILL, isSelected ? WHITE : colors[i]);
            fillellipse(iconX + j * 12, y + 15, 4, 4);
        }

        /* Difficulty name */
        setcolor(isSelected ? BLACK : LIGHTGRAY);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        int textX = SCREEN_W / 2 - 20;
        if (i == 1) textX -= 15;
        outtextxy(textX, y + 8, diffNames[i]);

        /* Description with stats */
        setcolor(isSelected ? DARKGRAY : 8);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        outtextxy(SCREEN_W / 2 - 85, y + 32, diffDesc[i]);

        /* AI reaction time indicator */
        if (isSelected)
        {
            setcolor(WHITE);
            char reactBuf[20];
            sprintf(reactBuf, "React: %dms", aiReact[i] * 16);
            outtextxy(SCREEN_W / 2 + 50, y + 32, reactBuf);
        }
    }

    /* AI Stats Panel */
    int panelY = 285;
    setfillstyle(SOLID_FILL, 1);
    bar(SCREEN_W / 2 - 100, panelY, SCREEN_W / 2 + 100, panelY + 55);
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(SCREEN_W / 2 - 98, panelY + 2, SCREEN_W / 2 + 98, panelY + 53);
    setcolor(colors[difficulty]);
    rectangle(SCREEN_W / 2 - 100, panelY, SCREEN_W / 2 + 100, panelY + 55);

    /* Panel title */
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(SCREEN_W / 2 - 35, panelY + 5, "AI SETTINGS");

    /* AI accuracy bar */
    setcolor(LIGHTGRAY);
    outtextxy(SCREEN_W / 2 - 90, panelY + 20, "Accuracy:");
    setfillstyle(SOLID_FILL, BLACK);
    bar(SCREEN_W / 2 - 20, panelY + 18, SCREEN_W / 2 + 85, panelY + 28);
    int accWidth = (difficulty == 0) ? 20 : (difficulty == 1) ? 50 : 80;
    setfillstyle(SOLID_FILL, colors[difficulty]);
    bar(SCREEN_W / 2 - 18, panelY + 20, SCREEN_W / 2 - 18 + accWidth, panelY + 26);
    setcolor(WHITE);
    rectangle(SCREEN_W / 2 - 20, panelY + 18, SCREEN_W / 2 + 85, panelY + 28);

    /* AI speed bar */
    setcolor(LIGHTGRAY);
    outtextxy(SCREEN_W / 2 - 90, panelY + 37, "Speed:");
    setfillstyle(SOLID_FILL, BLACK);
    bar(SCREEN_W / 2 - 20, panelY + 35, SCREEN_W / 2 + 85, panelY + 45);
    int spdWidth = (difficulty == 0) ? 25 : (difficulty == 1) ? 55 : 85;
    setfillstyle(SOLID_FILL, colors[difficulty]);
    bar(SCREEN_W / 2 - 18, panelY + 37, SCREEN_W / 2 - 18 + spdWidth, panelY + 43);
    setcolor(WHITE);
    rectangle(SCREEN_W / 2 - 20, panelY + 35, SCREEN_W / 2 + 85, panelY + 45);

    /* Controls hint */
    setcolor(CYAN);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(10, SCREEN_H - 25, "UP/DOWN = Select");
    setcolor(YELLOW);
    outtextxy(SCREEN_W - 120, SCREEN_H - 25, "ENTER = Start!");
}

void drawGameOver(void)
{
    int i, j;
    char buf[60];

    /* Dark dramatic background with red vignette */
    setfillstyle(SOLID_FILL, BLACK);
    bar(0, 0, SCREEN_W, SCREEN_H);
    
    /* Red fog/vignette effect */
    for (i = 0; i < 30; i++)
    {
        setcolor((i < 15) ? 4 : DARKGRAY);
        circle(SCREEN_W / 2, SCREEN_H / 2, 200 - i * 5);
    }

    /* Falling blood/debris particles */
    for (i = 0; i < 40; i++)
    {
        int px = (i * 19) % SCREEN_W;
        int py = (frameCount + i * 11) % SCREEN_H;
        int col = (i % 3 == 0) ? RED : (i % 3 == 1) ? 4 : DARKGRAY;
        setcolor(col);
        putpixel(px, py, col);
        putpixel(px, py + 1, col);
    }

    /* Main panel with dark red theme */
    setfillstyle(SOLID_FILL, 4); /* Dark red */
    bar(SCREEN_W / 2 - 158, 55, SCREEN_W / 2 + 158, 298);
    setfillstyle(SOLID_FILL, BLACK);
    bar(SCREEN_W / 2 - 155, 58, SCREEN_W / 2 + 155, 295);

    /* Red border with glow effect */
    setcolor(4);
    rectangle(SCREEN_W / 2 - 160, 52, SCREEN_W / 2 + 160, 300);
    setcolor(RED);
    rectangle(SCREEN_W / 2 - 158, 55, SCREEN_W / 2 + 158, 298);
    rectangle(SCREEN_W / 2 - 155, 58, SCREEN_W / 2 + 155, 295);
    setcolor(LIGHTRED);
    rectangle(SCREEN_W / 2 - 153, 60, SCREEN_W / 2 + 153, 293);

    /* Skull icon */
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(SCREEN_W / 2, 85, 18, 15);
    /* Eye sockets */
    setfillstyle(SOLID_FILL, BLACK);
    fillellipse(SCREEN_W / 2 - 7, 82, 5, 6);
    fillellipse(SCREEN_W / 2 + 7, 82, 5, 6);
    /* Nose */
    setcolor(BLACK);
    line(SCREEN_W / 2, 88, SCREEN_W / 2 - 2, 93);
    line(SCREEN_W / 2, 88, SCREEN_W / 2 + 2, 93);
    /* Teeth */
    setfillstyle(SOLID_FILL, WHITE);
    bar(SCREEN_W / 2 - 10, 95, SCREEN_W / 2 + 10, 100);
    setcolor(BLACK);
    for (j = -8; j <= 8; j += 4)
    {
        line(SCREEN_W / 2 + j, 95, SCREEN_W / 2 + j, 100);
    }

    /* GAME OVER text with dramatic effect */
    int textBob = (int)(sin(frameCount * 0.05) * 2);
    setcolor(4);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(SCREEN_W / 2 - 68, 112 + textBob + 2, "GAME OVER");
    setcolor(DARKGRAY);
    outtextxy(SCREEN_W / 2 - 70, 112 + textBob + 1, "GAME OVER");
    setcolor(RED);
    outtextxy(SCREEN_W / 2 - 71, 112 + textBob, "GAME OVER");
    setcolor(LIGHTRED);
    outtextxy(SCREEN_W / 2 - 71, 111 + textBob, "GAME OVER");

    /* Winner announcement */
    if (winnerIndex >= 0)
    {
        setcolor(6);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        sprintf(buf, "%s WINS!", players[winnerIndex].name);
        outtextxy(SCREEN_W / 2 - 55, 142, buf);
        setcolor(YELLOW);
        outtextxy(SCREEN_W / 2 - 57, 140, buf);
    }

    /* Scoreboard header */
    setfillstyle(SOLID_FILL, 4);
    bar(SCREEN_W / 2 - 135, 163, SCREEN_W / 2 + 135, 177);
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(SCREEN_W / 2 - 40, 166, "FINAL SCORES");

    /* Player scores with bars */
    for (i = 0; i < numPlayers; i++)
    {
        int y = 182 + i * 23;
        
        /* Score bar background */
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(SCREEN_W / 2 - 130, y, SCREEN_W / 2 + 130, y + 18);
        
        /* Highlight winner */
        if (i == winnerIndex)
        {
            setfillstyle(SOLID_FILL, 6);
            bar(SCREEN_W / 2 - 128, y + 2, SCREEN_W / 2 + 128, y + 16);
        }

        /* Player color indicator */
        setfillstyle(SOLID_FILL, players[i].color);
        bar(SCREEN_W / 2 - 125, y + 4, SCREEN_W / 2 - 115, y + 14);

        /* Player info */
        setcolor(players[i].color);
        sprintf(buf, "%s", players[i].name);
        outtextxy(SCREEN_W / 2 - 110, y + 5, buf);

        /* Stats */
        setcolor(WHITE);
        sprintf(buf, "K:%d  D:%d", players[i].kills, players[i].deaths);
        outtextxy(SCREEN_W / 2 - 20, y + 5, buf);

        /* K/D ratio */
        float kd = (players[i].deaths > 0) ? (float)players[i].kills / players[i].deaths : players[i].kills;
        setcolor(kd >= 1.0 ? GREEN : RED);
        sprintf(buf, "%.1f", kd);
        outtextxy(SCREEN_W / 2 + 70, y + 5, buf);

        /* Winner badge */
        if (i == winnerIndex)
        {
            setcolor(YELLOW);
            outtextxy(SCREEN_W / 2 + 100, y + 5, "[WIN]");
        }
    }

    /* Retry button */
    int btnPulse = (int)(sin(frameCount * 0.1) * 2);
    setfillstyle(SOLID_FILL, 2);
    bar(SCREEN_W / 2 - 75 - btnPulse, 272, SCREEN_W / 2 + 75 + btnPulse, 290);
    setcolor(LIGHTGREEN);
    rectangle(SCREEN_W / 2 - 75 - btnPulse, 272, SCREEN_W / 2 + 75 + btnPulse, 290);
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 58, 278, "ENTER = Try Again!");
}

void drawVictory(void)
{
    int i, j;
    char buf[60];

    /* Golden gradient background */
    for (i = 0; i < SCREEN_H; i += 6)
    {
        int shade = (i < 80) ? 6 : (i < 160) ? BROWN : (i < 240) ? DARKGRAY : BLACK;
        setfillstyle(SOLID_FILL, shade);
        bar(0, i, SCREEN_W, i + 6);
    }

    /* Spotlight effect from top */
    for (i = 0; i < 15; i++)
    {
        setcolor((i < 8) ? YELLOW : (i < 12) ? 6 : BROWN);
        int topWidth = 30 + i * 4;
        int bottomWidth = 100 + i * 8;
        line(SCREEN_W / 2 - topWidth, 0, SCREEN_W / 2 - bottomWidth, SCREEN_H);
        line(SCREEN_W / 2 + topWidth, 0, SCREEN_W / 2 + bottomWidth, SCREEN_H);
    }

    /* Enhanced confetti with different shapes */
    for (i = 0; i < 80; i++)
    {
        int cx = (i * 11 + frameCount * 2) % SCREEN_W;
        int cy = (frameCount + i * 5) % SCREEN_H;
        int cc = i % 7;
        int confColors[7] = {RED, YELLOW, GREEN, CYAN, MAGENTA, WHITE, LIGHTRED};
        setcolor(confColors[cc]);
        if (i % 3 == 0)
        {
            /* Square confetti */
            bar(cx, cy, cx + 2, cy + 2);
        }
        else if (i % 3 == 1)
        {
            /* Line confetti */
            int angle = (frameCount + i * 10) % 180;
            line(cx, cy, cx + 3, cy + 2);
        }
        else
        {
            /* Dot confetti */
            putpixel(cx, cy, confColors[cc]);
            putpixel(cx + 1, cy, confColors[cc]);
        }
    }

    /* Enhanced firework bursts with more rays */
    for (i = 0; i < 5; i++)
    {
        int fx = 70 + i * 130;
        int fy = 50 + (i * 23) % 50;
        int burst = (frameCount / 15 + i * 4) % 20;
        if (burst < 12)
        {
            int fColors[5] = {YELLOW, RED, CYAN, MAGENTA, LIGHTGREEN};
            for (j = 0; j < 12; j++)
            {
                int angle = j * 30 + burst * 3;
                int len = burst * 2;
                int ex = fx + (int)(cos(angle * PI / 180) * len);
                int ey = fy + (int)(sin(angle * PI / 180) * len);
                setcolor(fColors[i]);
                line(fx, fy, ex, ey);
                setcolor(WHITE);
                putpixel(ex, ey, WHITE);
                if (burst > 6)
                {
                    /* Sparkle trails */
                    setcolor(YELLOW);
                    putpixel(ex + rand() % 3 - 1, ey + rand() % 3 - 1, YELLOW);
                }
            }
        }
    }

    /* Victory panel with golden frame */
    setfillstyle(SOLID_FILL, YELLOW);
    bar(SCREEN_W / 2 - 162, 80, SCREEN_W / 2 + 162, 302);
    setfillstyle(SOLID_FILL, 6); /* Brown */
    bar(SCREEN_W / 2 - 158, 84, SCREEN_W / 2 + 158, 298);
    setfillstyle(SOLID_FILL, BLACK);
    bar(SCREEN_W / 2 - 155, 87, SCREEN_W / 2 + 155, 295);

    /* Decorative golden corners */
    setcolor(YELLOW);
    line(SCREEN_W / 2 - 162, 80, SCREEN_W / 2 - 140, 80);
    line(SCREEN_W / 2 - 162, 80, SCREEN_W / 2 - 162, 100);
    line(SCREEN_W / 2 + 140, 80, SCREEN_W / 2 + 162, 80);
    line(SCREEN_W / 2 + 162, 80, SCREEN_W / 2 + 162, 100);
    line(SCREEN_W / 2 - 162, 280, SCREEN_W / 2 - 162, 302);
    line(SCREEN_W / 2 - 162, 302, SCREEN_W / 2 - 140, 302);
    line(SCREEN_W / 2 + 162, 280, SCREEN_W / 2 + 162, 302);
    line(SCREEN_W / 2 + 140, 302, SCREEN_W / 2 + 162, 302);

    /* Enhanced Trophy with shine animation */
    int trophyX = SCREEN_W / 2;
    int shineOffset = (frameCount / 3) % 20;

    /* Trophy cup */
    setfillstyle(SOLID_FILL, 6); /* Dark gold */
    fillellipse(trophyX, 110, 22, 14);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(trophyX, 108, 20, 12);
    /* Trophy shine */
    setfillstyle(SOLID_FILL, WHITE);
    fillellipse(trophyX - 8 + shineOffset / 2, 104, 4, 3);
    /* Trophy handles */
    setcolor(YELLOW);
    arc(trophyX - 22, 108, 90, 270, 8);
    arc(trophyX + 22, 108, 270, 90, 8);
    /* Trophy stem */
    setfillstyle(SOLID_FILL, YELLOW);
    bar(trophyX - 6, 118, trophyX + 6, 135);
    /* Trophy base */
    setfillstyle(SOLID_FILL, 6);
    bar(trophyX - 15, 135, trophyX + 15, 142);
    setfillstyle(SOLID_FILL, YELLOW);
    bar(trophyX - 13, 136, trophyX + 13, 140);

    /* Animated VICTORY text with rainbow effect */
    int textY = 146 + (int)(sin(frameCount * 0.08) * 4);
    setcolor(DARKGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(SCREEN_W / 2 - 57, textY + 3, "VICTORY!");
    setcolor(6);
    outtextxy(SCREEN_W / 2 - 58, textY + 2, "VICTORY!");
    setcolor(YELLOW);
    outtextxy(SCREEN_W / 2 - 59, textY + 1, "VICTORY!");
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 60, textY, "VICTORY!");

    /* Champion subtitle with stars */
    setcolor(YELLOW);
    putpixel(SCREEN_W / 2 - 65, 175, YELLOW);
    putpixel(SCREEN_W / 2 + 65, 175, YELLOW);
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(SCREEN_W / 2 - 50, 172, "** CHAMPION **");

    /* Scoreboard header */
    setfillstyle(SOLID_FILL, 6);
    bar(SCREEN_W / 2 - 135, 188, SCREEN_W / 2 + 135, 202);
    setcolor(WHITE);
    outtextxy(SCREEN_W / 2 - 45, 191, "FINAL STANDINGS");

    /* Sort players by kills */
    int sorted[4] = {0, 1, 2, 3};
    for (i = 0; i < numPlayers - 1; i++)
    {
        for (j = i + 1; j < numPlayers; j++)
        {
            if (players[sorted[j]].kills > players[sorted[i]].kills)
            {
                int temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    /* Player rankings with medal icons */
    for (i = 0; i < numPlayers; i++)
    {
        int idx = sorted[i];
        int y = 207 + i * 20;
        int medalColors[4] = {YELLOW, WHITE, 6, DARKGRAY};
        int medalBg[4] = {YELLOW, LIGHTGRAY, BROWN, DARKGRAY};

        /* Row background */
        setfillstyle(SOLID_FILL, (i % 2) ? DARKGRAY : 8);
        bar(SCREEN_W / 2 - 130, y, SCREEN_W / 2 + 130, y + 17);

        /* Medal icon */
        setfillstyle(SOLID_FILL, medalBg[i]);
        fillellipse(SCREEN_W / 2 - 118, y + 9, 7, 7);
        setcolor(medalColors[i]);
        circle(SCREEN_W / 2 - 118, y + 9, 7);
        /* Medal number */
        setcolor((i < 3) ? BLACK : WHITE);
        sprintf(buf, "%d", i + 1);
        outtextxy(SCREEN_W / 2 - 121, y + 5, buf);

        /* Player name */
        setcolor(players[idx].color);
        outtextxy(SCREEN_W / 2 - 100, y + 5, players[idx].name);

        /* Stats */
        setcolor(WHITE);
        sprintf(buf, "K:%d D:%d", players[idx].kills, players[idx].deaths);
        outtextxy(SCREEN_W / 2 - 20, y + 5, buf);

        /* K/D ratio */
        float kd = (players[idx].deaths > 0) ? (float)players[idx].kills / players[idx].deaths : players[idx].kills;
        setcolor(kd >= 1.0 ? LIGHTGREEN : LIGHTRED);
        sprintf(buf, "%.1f", kd);
        outtextxy(SCREEN_W / 2 + 60, y + 5, buf);

        /* YOU indicator */
        if (idx == playerIndex)
        {
            setcolor(CYAN);
            outtextxy(SCREEN_W / 2 + 95, y + 5, "<YOU");
        }
    }

    /* Play again button with premium design */
    int btnPulse = (int)(sin(frameCount * 0.12) * 3);
    setfillstyle(SOLID_FILL, 2); /* Dark green */
    bar(SCREEN_W / 2 - 78 - btnPulse, 278, SCREEN_W / 2 + 82 + btnPulse, 298);
    setfillstyle(SOLID_FILL, GREEN);
    bar(SCREEN_W / 2 - 75 - btnPulse, 280, SCREEN_W / 2 + 79 + btnPulse, 295);
    /* Button highlight */
    setfillstyle(SOLID_FILL, LIGHTGREEN);
    bar(SCREEN_W / 2 - 73 - btnPulse, 281, SCREEN_W / 2 + 77 + btnPulse, 285);
    setcolor(WHITE);
    rectangle(SCREEN_W / 2 - 75 - btnPulse, 280, SCREEN_W / 2 + 79 + btnPulse, 295);
    setcolor(BLACK);
    outtextxy(SCREEN_W / 2 - 55, 285, "ENTER = Play Again");
}

void render(void)
{
    setactivepage(activePage);
    setfillstyle(SOLID_FILL, BLACK);
    bar(0, 0, SCREEN_W, SCREEN_H);

    switch (gameState)
    {
    case GS_SPLASH:
        drawSplash();
        break;
    case GS_MENU:
        drawMenu();
        break;
    case GS_DIFFICULTY:
        drawDifficulty();
        break;
    case GS_PLAYING:
        renderMap();
        renderPickups();
        renderBullets();
        renderGrenades();
        renderPlayers();
        renderParticles();
        renderEffects();
        renderUI();
        break;
    case GS_GAMEOVER:
        renderMap();
        renderPlayers();
        renderUI();
        drawGameOver();
        break;
    case GS_VICTORY:
        renderMap();
        renderPlayers();
        renderUI();
        drawVictory();
        break;
    }

    setvisualpage(activePage);
    activePage = 1 - activePage;
}

/* =============== MAIN LOOP =============== */

void mainLoop(void)
{
    while (1)
    {
        handleInput();

        if (gameState == GS_SPLASH)
        {
            splashTimer--;
            if (splashTimer <= 0)
                gameState = GS_MENU;
        }

        if (gameState == GS_PLAYING)
        {
            updatePhysics();
            updateBullets();
            updateGrenades();
            updateAI();
            updateEffects();
            updateParticles();
            updateClouds();
            updatePickups();
            updateKillfeed();
            updateShake();
            updateCamera();
            checkGameOver();
            matchTime++;
        }

        render();
        frameCount++;
        delay(33);
    }
}

/* =============== MAIN =============== */

int main(void)
{
    initGame();
    mainLoop();
    closegraph();
    return 0;
}
