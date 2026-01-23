#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <string>

#define TILE_TEXTURE_PATH "assets/tile.png"
#define PLAYER_TEXTURE_FRONT  "assets/idle_front.png"
#define PLAYER_TEXTURE_BACK  "assets/idle_back.png"
#define PLAYER_TEXTURE_RIGHT "assets/idle_right.png"
#define PLAYER_TEXTURE_LEFT  "assets/idle_left.png"
#define FIREBALL_TEXTURE_PATH "assets/fireball.png"
#define GREEN_BEAD_TEXTURE_PATH "assets/green-bead.png"

#define GAME_FONT_PATH "assets/fonts/Roboto/static/Roboto-Medium.ttf"

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

const int 
	TILE_SIZE = 50, 
	TILE_MAP_SIZE = 25;
const int SCREEN_SIZE[2] = {800, 600};

short unsigned int score, fireballs_bypassed; 

bool 
	isDone = false,
	isPaused = false,
	isGameOver = false;

//Vectors
vector<Tile> tile_map;
vector<Sprite> fireballs;

//Advertisment
int randInt(int min, int max);
bool Init();
void GameOver();
void renderText(const char* msg, SDL_Rect dst);
void renderMenu();

SDL_Window *win = NULL;
SDL_Renderer *ren = NULL;

SDL_Texture *tileTexture = NULL;

SDL_Texture *playerIdleFront = NULL;
SDL_Texture *playerIdleBack = NULL;
SDL_Texture *playerIdleLeft = NULL;
SDL_Texture *playerIdleRight = NULL;

SDL_Texture *fireballTexture = NULL;
SDL_Texture *goalTexture = NULL;

SDL_Rect escape_cuadro = {0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1]};

TTF_Font *gameFont = NULL;
SDL_Color whiteText = {255, 255, 255, 255};

int main (void) {

	if (!Init()) {
		cout << "Find some Errors\n";
		return 1;
	}

	SDL_Event event;
	int speed = 8;

	Sprite player;
	player.posSize = { (SCREEN_SIZE[0]-66) / 2, (SCREEN_SIZE[1]-100) / 2, 66, 100};
	player.texture = playerIdleFront;
	player.type = PLAYER;

	Sprite goal;
	goal.posSize = { randInt(0, SCREEN_SIZE[0]-25) , randInt(0, SCREEN_SIZE[1]-25), 25, 25};
	goal.texture = goalTexture;
	goal.type = GOAL;

	while (!isDone) {
		while(SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				isDone = true;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE && !(isGameOver)) {
					isPaused = !(isPaused);
				}
			}
		}
	
		if (isGameOver && !isPaused) {
			isPaused = true;
		}

		const Uint8* keys = SDL_GetKeyboardState(NULL);	
		
		if (keys[SDL_SCANCODE_W] && !(isPaused)) {
			player.texture = playerIdleBack;
			if ( player.posSize.y-speed > 0 )
			player.posSize.y -= speed;
		} 
		
		if (keys[SDL_SCANCODE_S] && !(isPaused)) {
		 	player.texture = playerIdleFront;
			if (player.posSize.y+speed < 600-player.posSize.h)
			player.posSize.y += speed;
		}		

		if (keys[SDL_SCANCODE_A] && !(isPaused) ) { 
			player.texture = playerIdleLeft;
			if (player.posSize.x-speed > 0)
			player.posSize.x -= speed;
		}

		if (keys[SDL_SCANCODE_D] && !(isPaused) ) {
			player.texture = playerIdleRight;
 			if (player.posSize.x+speed < 800-player.posSize.w)
			player.posSize.x += speed;
		}

		for (auto& t : tile_map) {
    		SDL_RenderCopy(ren, tileTexture, &t.src, &t.dst);
		}	
	
		SDL_RenderCopy(ren, player.texture, NULL, &player.posSize);	
		
		for (auto& fireball : fireballs) {
			if (SDL_HasIntersection(&fireball.posSize, &player.posSize)) {
				isGameOver = true;
			}
			
			if (!isPaused) {
				fireball.posSize.x += speed + (rand() % 5);
				if (fireball.posSize.x > 800+fireball.posSize.w) {
					fireball.posSize = {-50, randInt(0, SCREEN_SIZE[1]-50), 50, 50};
					fireballs_bypassed++;
				}
			}
			SDL_RenderCopy(ren, fireball.texture, NULL, &fireball.posSize);
		}
		
		if (SDL_HasIntersection(&goal.posSize, &player.posSize)) {
			score++;
			goal.posSize.x = randInt(0, SCREEN_SIZE[0]-goal.posSize.w);
			goal.posSize.y = randInt(0, SCREEN_SIZE[1]-goal.posSize.h);
		}
		SDL_RenderCopy(ren, goal.texture, NULL, &goal.posSize);
		
		if (isPaused) renderMenu();

		SDL_RenderPresent(ren);
		SDL_RenderClear(ren);
		SDL_Delay(50);
	}	
	GameOver();

	TTF_CloseFont(gameFont);	
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

void renderMenu() {
		string score_text = to_string(score);

		SDL_RenderCopy(ren, NULL, NULL, &escape_cuadro);
		SDL_RenderFillRect(ren, &escape_cuadro);
		
		string collected_goals_text = "COLLECTED GOALS: " + score_text; 
		string fireballs_bypassed_text = "FIREBALLS BYPASSED: " + to_string(fireballs_bypassed);
		string victory_coeficient_text = "VICTORY COEFFICIENT: "; 

		if (score != 0) victory_coeficient_text += to_string(round(((float)fireballs_bypassed / (float)score)));
		else victory_coeficient_text += "0";
			
		renderText(collected_goals_text.c_str(), 	{25, 100, 0, 0}); 
		renderText(fireballs_bypassed_text.c_str(), {25, 150, 0, 0});
		renderText(victory_coeficient_text.c_str(), {25, 200, 0, 0});
		
}

void GameOver() {
	cout << "\n !GAME OVER! \n" 
		 << "COLLECTED GOALS: " << score << endl 
		 << "FIREBALLS BYPASSED: " << fireballs_bypassed << endl; 
		 
	if (score > 0) cout<< "VICTORY COEFFICIENT: " << round(((float)fireballs_bypassed / (float)score)) << endl;
	else cout << "VICTORY COEFFICIENT: 0\n";
}

bool Init() {
	srand(time(NULL));

	bool isAllRight = true;

	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		cout << "Error with Init video: " << SDL_GetError() << endl;
		isAllRight = false;
	}
	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
		cout << "Error with Init IMG: " << IMG_GetError() << endl;
		isAllRight = false;
	}
	if (TTF_Init() != 0) {
		cout << "Error with Init ttf: " << TTF_GetError() << endl;
		isAllRight = false;
	}

	win = SDL_CreateWindow("Events", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_SIZE[0], SCREEN_SIZE[1], SDL_WINDOW_SHOWN);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	gameFont = TTF_OpenFont(GAME_FONT_PATH, 30);	
	
	if (win == NULL) {
		isAllRight = false;
		cout << "Error with create window\n";
	}
	if (ren == NULL) {
		isAllRight = false;
		cout << "Error with create render\n";
	}

	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 128);
	
	tileTexture = IMG_LoadTexture(ren, TILE_TEXTURE_PATH);	
	
	playerIdleFront = IMG_LoadTexture(ren, PLAYER_TEXTURE_FRONT);
	playerIdleBack = IMG_LoadTexture(ren,  PLAYER_TEXTURE_BACK);
	playerIdleLeft = IMG_LoadTexture(ren,  PLAYER_TEXTURE_LEFT);
	playerIdleRight = IMG_LoadTexture(ren, PLAYER_TEXTURE_RIGHT);

	fireballTexture = IMG_LoadTexture(ren, FIREBALL_TEXTURE_PATH);
	goalTexture = IMG_LoadTexture(ren, GREEN_BEAD_TEXTURE_PATH);

	for (int y = 0; y < TILE_MAP_SIZE; y++) {
    	for (int x = 0; x < TILE_MAP_SIZE; x++) {
        	Tile t;

        	t.src = {0, 0, TILE_SIZE, TILE_SIZE};
        	t.dst = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

        	tile_map.push_back(t);
    	}
	}

	for (int i = 0; i < FIREBALLS_COUNT; ++i) {
		Sprite fireball;
		
		fireball.posSize = {-50, randInt(0, SCREEN_SIZE[1]-50), 50, 50 };
		fireball.texture = fireballTexture;
		fireball.type = ENEMY;
		fireballs.push_back(fireball);
	}
	return isAllRight;
}

int randInt(int min, int max) {
	return ( min + rand() % max - min + 1 );
}

void renderText(const char* msg, SDL_Rect dst) {
	SDL_Surface *temp_surf = TTF_RenderUTF8_Blended(gameFont, msg, whiteText);
	SDL_Texture *temp_texture = SDL_CreateTextureFromSurface(ren, temp_surf);
	
	SDL_QueryTexture(temp_texture, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, temp_texture, NULL, &dst);
	
	SDL_FreeSurface(temp_surf);	
	SDL_DestroyTexture(temp_texture);
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
