#include "Tetris.h"

int main() {
	Tetris game(20, 10, 296, 35, 35);//20行，10列，左边起始位置296，上边起始位置35，方块边长35
	game.play();
	return 0;
}