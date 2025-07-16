#pragma once
#include <graphics.h>
#include <vector>

struct Point {
	int row;
	int col;
};

class Block		//表示方块
{
public:
	Block();
	void drop();		//下降
	void moveLeftRight(int offset);		//左移右移
	void rotate();		//旋转
	void draw(int leftMargin, int topMargin);		//画方块的位置
	static IMAGE** getImages();
	Block& operator=(const Block& other);
	bool blockInMap(const std::vector<std::vector<int>>& map);//判断方块是否在网格里面
	void fixation(std::vector<std::vector<int>>& map);		//固定方块

private:
	int blockType;//方块类型
	Point smallBlock[4];//组成各种方块的最小单元
	IMAGE* img;

	static IMAGE* imgs[7];//七个图形的图片
	static int size;//图片大小
};