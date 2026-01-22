#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <ctime>

#define TILE_TEXTURE_NAME "assets/tile.png"
#define FIREBALLS_COUNT 3

using namespace std;

enum Type {
	PLAYER,
	ENEMY,
	GOAL
};

struct Sprite {
	enum Type type;
	SDL_Texture *texture;
	SDL_Rect posSize;
};

struct Tile {
	SDL_Rect src;
	SDL_Rect dst;
};

SDL_Window *win = NULL;
SDL_Renderer *ren = NULL;

SDL_Texture *tileTexture = NULL;

SDL_Texture *playerIdleFront = NULL;
SDL_Texture *playerIdleBack = NULL;
SDL_Texture *playerIdleLeft = NULL;
SDL_Texture *playerIdleRight = NULL;

SDL_Texture *fireballTexture = NULL;
SDL_Texture *goalTexture = NULL;

const int 
	TILE_SIZE = 50, 
	TILE_MAP_SIZE = 25;
const int SCREEN_SIZE[2] = {800, 600};

vector<Tile> tile_map;
short unsigned int score, fireballs_bypassed; 

void GameOver() {
	cout << "\n !GAME OVER! \n" << "COLLECTED GOALS: " << score << endl << "FIREBALLS BYPASSED: " << fireballs_bypassed << endl;
}

bool Init() {
	srand(time(NULL));

	bool isAllRight = true;

	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);

	win = SDL_CreateWindow("Events", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_SIZE[0], SCREEN_SIZE[1], SDL_WINDOW_SHOWN);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	tileTexture = IMG_LoadTexture(ren, TILE_TEXTURE_NAME);	
	
	playerIdleFront = IMG_LoadTexture(ren, "assets/idle_front.png");
	playerIdleBack = IMG_LoadTexture(ren, "assets/idle_back.png");
	playerIdleLeft = IMG_LoadTexture(ren, "assets/idle_left.png");
	playerIdleRight = IMG_LoadTexture(ren, "assets/idle_right.png");

	fireballTexture = IMG_LoadTexture(ren, "assets/fireball.png");
	goalTexture = IMG_LoadTexture(ren, "assets/green-bead.png");

	for (int y = 0; y < TILE_MAP_SIZE; y++) {
    	for (int x = 0; x < TILE_MAP_SIZE; x++) {
        	Tile t;

        	t.src = {0, 0, TILE_SIZE, TILE_SIZE};
        	t.dst = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

        	tile_map.push_back(t);
    	}
	}
	

	return isAllRight;
}

/*void DrawCircle(SDL_Renderer *ren, int cx, int cy, int r) {
	for (int x = -r; x <= r; ++x) {
		for (int y = -r; y <= r; ++y) {
			if (x*x + y*y <= r*r) {
				SDL_RenderDrawPoint(ren, cx + x, cy + y);
			}
		}
	}
}*/

int randInt(int min, int max) {
	return ( min + rand() % max - min + 1 );
}

int main (void) {
	Init();

	bool isDone = false;
	SDL_Event event;
	int speed = 8;

	Sprite player;
	player.posSize = { (SCREEN_SIZE[0]-66) / 2, (SCREEN_SIZE[1]-100) / 2, 66, 100};
	player.texture = playerIdleFront;
	player.type = PLAYER;

	Sprite fireballs[FIREBALLS_COUNT];
	
	for (int i = 0; i < FIREBALLS_COUNT; ++i) {
		fireballs[i].posSize = {-50, randInt(0, SCREEN_SIZE[1]-50), 50, 50 };
		fireballs[i].texture = fireballTexture;
		fireballs[i].type = ENEMY;
	}

	Sprite goal;
	goal.posSize = { randInt(0, SCREEN_SIZE[0]-25) , randInt(0, SCREEN_SIZE[1]-25), 25, 25};
	goal.texture = goalTexture;
	goal.type = GOAL;

	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

	while (!isDone) {
		while(SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				isDone = true;
			}
		}
	
		const Uint8* keys = SDL_GetKeyboardState(NULL);
		
		if (keys[SDL_SCANCODE_W]) {
			player.texture = playerIdleBack;
			if ( player.posSize.y-speed > 0 )
			player.posSize.y -= speed;
		} 
		
		if (keys[SDL_SCANCODE_S]) {
		 	player.texture = playerIdleFront;
			if (player.posSize.y+speed < 600-player.posSize.h)
			player.posSize.y += speed;
		}		

		if (keys[SDL_SCANCODE_A]) { 
			player.texture = playerIdleLeft;
			if (player.posSize.x-speed > 0)
			player.posSize.x -= speed;
		}

		if (keys[SDL_SCANCODE_D]) {
			player.texture = playerIdleRight;
 			if (player.posSize.x+speed < 800-player.posSize.w)
			player.posSize.x += speed;
		}
		
		for (auto& t : tile_map) {
    		SDL_RenderCopy(ren, tileTexture, &t.src, &t.dst);
		}	
	
		SDL_RenderCopy(ren, player.texture, NULL, &player.posSize);	
		
		for (int i = 0; i < FIREBALLS_COUNT; ++i) {	
			fireballs[i].posSize.x += speed + (rand() % 5);
			SDL_RenderCopy(ren, fireballs[i].texture, NULL, &fireballs[i].posSize);
			if (fireballs[i].posSize.x > 800+fireballs[i].posSize.w) {
				fireballs[i].posSize = {-50, randInt(0, SCREEN_SIZE[1]-50), 50, 50};
				fireballs_bypassed++;
			}
		}

		for (int i = 0; i < FIREBALLS_COUNT; ++i) {
			if (SDL_HasIntersection(&fireballs[i].posSize, &player.posSize)) {
				isDone = true;
			}
		}
		
		if (SDL_HasIntersection(&goal.posSize, &player.posSize)) {
			cout << "You recive goal\n";
			score++;
			goal.posSize.x = randInt(0, SCREEN_SIZE[0]-goal.posSize.w);
			goal.posSize.y = randInt(0, SCREEN_SIZE[1]-goal.posSize.h);
		}
		SDL_RenderCopy(ren, goal.texture, NULL, &goal.posSize);
		
		//SDL_RenderFillRect(ren, &player.posSize);
		SDL_RenderPresent(ren);
	
		SDL_Delay(50);
	}	
	GameOver();
	
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	
	IMG_Quit();
	SDL_Quit();
	return 0;
}
