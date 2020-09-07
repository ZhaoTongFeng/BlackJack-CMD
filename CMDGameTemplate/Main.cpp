#include "Game.h"
int main() {
	Game game = Game();
	if (game.Initialize())
		game.Loop();
	game.Shutdown();
}