#pragma once
#include <graphics.h>
#include <vector>
#include <memory>

#include "Block.h"
using namespace std;

class Tetris		//表示俄罗斯方块游戏
{
public:
	//构造函数
	Tetris(int rows, int cols, int left, int top, int blockSize);		//left是游戏区域左侧到x=0的距离,top是游戏区域上侧到y=0的距离,blockSize是小方块的尺寸
	void init(); //初始化
	void play();//开始游戏

private:
	//0：空白
	//5：第五种方块
	vector<vector<int>>map;		//二维数组 在这里设定网格尺寸，便于维护
	int rows;				//网格行数
	int cols;				//网格列数
	int leftMargin;			//网格范围距离背景左侧的距离
	int topMargin;			//网格范围距离背景上侧的距离
	int blockSize;			//方块大小
	IMAGE imgBg;			//背景图片
	int delay;				//渲染一次的时间间隔，放在这里方便改动(在初始化的时候赋值)
	bool update;			//判断是否渲染 1渲染 0不渲染
	std::unique_ptr<Block> curBlock;   // 当前方块
	std::unique_ptr<Block> nextBlock;  // 下一个方块
	Block bakBlock;		//替代正在下降的方块的 不动的方块
	int score;			//当前分数
	int highestScore;		//历史最高分
	bool gameOver;		//游戏结束
	IMAGE imgOver;		//游戏结束图片
	IMAGE imgStartBg;       // 开始背景图
	IMAGE imgBeginBtn;      // 开始按钮图
	IMAGE imgBeginBtnHover; // 按钮悬停效果图
	bool isHovering;        // 鼠标是否悬停在按钮上
	int bgOffset;           // 背景动画偏移量
	int bgDirection;        // 背景移动方向(1或-1)
	bool isFirstMusicPlaying;  // 用于音乐交替播放的标志

	void initMusic();          // 初始化音乐播放
	void updateMusic();        // 更新音乐播放状态
	void keyboard();		//接收用户输入
	void render();			//渲染游戏界面
	int getDelay();			//时间间隔
	void drop();			//方块下落
	void clearLine();		//消除一行方块
	void moveLeftRight(int offset);	//左右移动
	void rotate();			//旋转
	void drawScore();		//绘制分数
	void checkOver();		//检查游戏是否结束
	void saveScore();		//保存最高分
	void displayOver();		//更新游戏画面
	void initStartScreen();  // 初始化开始界面
	void renderStartScreen();// 渲染开始界面
	bool checkStartClick();  // 检查开始点击
};

