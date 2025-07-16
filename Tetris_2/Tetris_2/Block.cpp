#include <cstdlib>
#include "Block.h"

// 静态成员初始化
IMAGE* Block::imgs[7] = { NULL, };// 存储7种方块图片的指针数组
int Block::size = 35;			// 每个小方块的像素尺寸

Block::Block()
{
	// 首次创建方块时加载图片资源
	if (imgs[0] == NULL) {
		IMAGE imgTmp;
		loadimage(&imgTmp, "resource/block.png");//把图片加载到内存

		// 从素材图中切割出7种方块图片
		SetWorkingImage(&imgTmp);
		for (int i = 0; i < 7; i++) {
			imgs[i] = new IMAGE;
			getimage(imgs[i], i * size, 0, size, size);// 按顺序截取每个方块
		}
		SetWorkingImage();//恢复工作区
	}

	// 定义7种方块的初始形状（用数字编码表示相对位置）
	int block[7][4] = {
		1,3,5,7,//直线
		2,4,5,7,//反Z
		3,4,5,6,//Z
		3,4,5,7,//凸
		2,3,5,7,//L
		3,5,6,7,//反L
		2,3,4,5,//正方形
	};

	//随机生成一种方块（1-7）
	blockType = 1 + rand() % 7;

	// 初始化方块的4个小方块坐标
	for (int i = 0; i < 4; i++) {
		int value = block[blockType - 1][i]; // 获取当前类型的编码值
		smallBlock[i].row = value / 2;//定义每个小方块的行数是它在block二维数组里面的数字除以2的整数部分
		smallBlock[i].col = (value % 2) + 4;//定义每个小方块的行数是它在block二维数组里面的数字除以2的余数部分再加4

	}

	img = imgs[blockType - 1];// 关联对应的方块图片
}

void Block::drop()
{
	for (int i = 0; i < 4; i++) {	//小方块下落 所有小方块行坐标+1
		smallBlock[i].row++;
	}
}

// 方块左右移动：所有小方块列坐标偏移
void Block::moveLeftRight(int offset)
{
	for (int i = 0; i < 4; i++) {
		smallBlock[i].col += offset;
	}
}

// 绘制方块：在指定偏移位置绘制4个小方块
void Block::draw(int leftMargin, int topMargin)
{
	for (int i = 0; i < 4; i++) {
		int x = leftMargin + smallBlock[i].col * size;// 计算实际屏幕X坐标
		int y = topMargin + smallBlock[i].row * size; // 计算实际屏幕Y坐标
		putimage(x, y, img); // 绘制小方块图片
	}
}

// 获取所有方块图片的指针数组
IMAGE** Block::getImages()
{
	return imgs;
}

// 赋值运算符重载：深拷贝方块数据
Block& Block::operator=(const Block& other)
{
	if (this == &other) {
		return *this;// 处理自赋值
	}
	this->blockType = other.blockType;
	for (int i = 0; i < 4; i++) {
		this->smallBlock[i] = other.smallBlock[i];// 复制每个小方块坐标
	}
	return *this;
}

// 检测方块是否在合法地图范围内
bool Block::blockInMap(const std::vector<std::vector<int>>& map)
{
	int rows = map.size();
	int cols = map[0].size();
	for (int i = 0; i < 4; i++) {// 检查是否越界或与已有方块重叠
		if (smallBlock[i].col < 0 || smallBlock[i].col >= cols ||
			smallBlock[i].row < 0 || smallBlock[i].row >= rows ||
			map[smallBlock[i].row][smallBlock[i].col]) {
			return false;
		}
	}
	return true;
}

// 将方块固定到地图中（标记为不可移动）
void Block::fixation(std::vector<std::vector<int>>& map)
{
	for (int i = 0; i < 4; i++) {	//固定方块
		map[smallBlock[i].row][smallBlock[i].col] = blockType;// 用方块类型编号标记地图
	}
}

// 方块旋转：基于中心点进行90度旋转变换
void Block::rotate()
{
	Point p = smallBlock[1];		//使用第2个小方块作为旋转中心

	for (int i = 0; i < 4; i++) {
		Point tmp = smallBlock[i];
		smallBlock[i].col = p.col - tmp.row + p.row;
		smallBlock[i].row = p.row + tmp.col - p.col;//公式来源：b站up 程序员Rock
	}
}