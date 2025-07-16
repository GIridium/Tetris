#include <iostream>
#include <time.h>
#include <cstdlib>
#include <graphics.h>
#include <conio.h>
#include <mmsystem.h>
#include <fstream>
#include <memory>

#pragma comment(lib, "winmm.lib")	// 加载多媒体静态库

#include "Tetris.h"
#include "Block.h"

#define MAX_LEVEL 5                // 游戏最高等级
#define RECORDER_FILE "recorder.txt"	// 用来永久存储最高分的文件名

const int SPEED_NORMAL = 500;	// 正常下落速度(毫秒)
const int SPEED_QUICK = 50;		// 快速下落速度(毫秒)

// 构造函数：初始化游戏区域参数
Tetris::Tetris(int rows, int cols, int left, int top, int blockSize)
{
    this->rows = rows;          // 游戏区域行数
    this->cols = cols;          // 游戏区域列数
    leftMargin = left;          // 网格左边界像素值
    topMargin = top;            // 网格上边界像素值
    this->blockSize = blockSize; // 每个方块的大小(像素)

    // 初始化游戏地图(二维数组)
    for (int i = 0; i < rows; i++) {
        vector<int>mapRow;
        for (int j = 0; j < cols; j++) {
            mapRow.push_back(0);  // 0表示该位置为空
        }
        map.push_back(mapRow);
    }
}

// 初始化游戏状态
void Tetris::init()
{
    // 背景音乐设置
    initMusic();  // 初始化音乐播放系统

    delay = SPEED_NORMAL;       // 设置初始下落速度

    srand(time(NULL));          // 设置随机数种子

    // 创建游戏窗口
    initgraph(948, 774);        // 初始化图形窗口
    setbkcolor(WHITE);          // 设置背景颜色为白色
    cleardevice();              // 清屏\

    // 初始化开始界面
    initStartScreen();

    // 显示开始界面循环
    while (true) {
        renderStartScreen();

        if (checkStartClick()) {
            break; // 点击开始按钮后进入游戏
        }

        Sleep(10); // 防止CPU占用过高
    }

    // 加载图片资源
    loadimage(&imgBg, "resource/background.png");  // 背景图片
    loadimage(&imgOver, "resource/gameOver.png");  // 游戏结束图片

    // 重置游戏地图
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            map[i][j] = 0;      // 清空地图
        }
    }

    score = 0;                  // 重置当前分数

    // 从文件读取最高分记录
    ifstream file(RECORDER_FILE);
    if (!file.is_open()) {
        std::cout << RECORDER_FILE << "打开失败" << std::endl;
        highestScore = 0;       // 文件打开失败则最高分为0
    }
    else {
        file >> highestScore;    // 从文件读取最高分
    }
    file.close();               // 关闭文件

    gameOver = false;           // 重置游戏状态
}

// 处理键盘输入
void Tetris::keyboard()
{
    unsigned char ch;
    bool rotateFlag = false;    // 旋转标志
    int dx = 0;                 // 水平移动方向

    if (_kbhit()) {             // 检测键盘输入
        ch = _getch();          // 获取按键(不需要回车)
        if (ch == 224) {        // 扩展键(方向键)
            ch = _getch();      // 获取扩展键码
            switch (ch) {
            case 72: rotateFlag = true; break;  // 上箭头 - 旋转
            case 80: delay = SPEED_QUICK; break; // 下箭头 - 加速下落
            case 75: dx = -1; break;            // 左箭头 - 左移
            case 77: dx = 1; break;             // 右箭头 - 右移
            default:break;
            }
        }
        if (ch == 27) {         // ESC键 - 退出游戏
            system("pause");
            exit(0);
        }
    }

    if (rotateFlag) {           // 处理旋转
        rotate();
        update = true;          // 需要更新画面
    }
    if (dx != 0) {              // 处理水平移动
        moveLeftRight(dx);
        update = true;          // 需要更新画面
    }
}

// 渲染游戏画面
void Tetris::render()
{
    putimage(0, 0, &imgBg);     // 绘制背景图片

    IMAGE** imgs = Block::getImages();  // 获取方块图片资源
    BeginBatchDraw();           // 开始批量绘制
    // 绘制已固定的方块
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (map[i][j] == 0) continue;  // 跳过空位置
            int x = j * blockSize + leftMargin;  // 计算屏幕坐标
            int y = i * blockSize + topMargin;
            putimage(x, y, imgs[map[i][j] - 1]);  // 绘制方块
        }
    }

    // 绘制当前下落中的方块
    curBlock->draw(leftMargin, topMargin);
    // 绘制下一个方块预览
    nextBlock->draw(621, 96);

    drawScore();                // 绘制分数显示

    EndBatchDraw();             // 结束批量绘制
}

// 获取时间间隔(毫秒)
int Tetris::getDelay()
{
    static unsigned long long lastTime = 0;
    unsigned long long currentTime = GetTickCount();

    if (lastTime == 0) {        // 第一次调用
        lastTime = currentTime;
        return 0;
    }
    else {
        int ret = currentTime - lastTime;  // 计算时间差
        lastTime = currentTime;
        return ret;
    }
}

// 方块下落逻辑
void Tetris::drop()
{
    bakBlock = *curBlock;       // 备份当前方块状态
    curBlock->drop();           // 尝试下落

    if (!curBlock->blockInMap(map)) {  // 下落到底部或碰撞
        bakBlock.fixation(map);  // 将方块固定到地图中
        curBlock = std::move(nextBlock);  // 转移下一个方块所有权
        nextBlock = std::make_unique<Block>();  // 生成新方块
    }

    delay = SPEED_NORMAL;       // 重置下落速度
}

// 消除满行
void Tetris::clearLine()
{
    int lines = 0;              // 消除行数计数
    int k = rows - 1;           // 从底部开始扫描
    for (int i = rows - 1; i >= 0; i--) {
        int count = 0;          // 当前行方块计数
        for (int j = 0; j < cols; j++) {
            if (map[i][j]) {
                count++;        // 统计当前行方块数
            }
            map[k][j] = map[i][j];  // 重构地图
        }

        if (count < cols) {     // 不是满行
            k--;               // 保留该行
        }
        else {                  // 满行
            lines++;           // 增加消除行数
        }
    }

    if (lines > 0) {           // 计算得分并播放音效
        int addScore[4] = { 10,30,60,80 };  // 消除1-4行的得分
        score += addScore[lines - 1];      // 增加分数

        mciSendString("play resource/sound1.mp3", 0, 0, 0);  // 播放消除音效
        update = true;          // 需要更新画面
    }
}

// 主游戏循环
void Tetris::play()
{
    init();                     // 初始化游戏

    // 初始化方块
    nextBlock = std::make_unique<Block>();
    curBlock = std::move(nextBlock);  // 转移所有权
    nextBlock = std::make_unique<Block>();

    int timer = 0;              // 计时器
    while (1) {
        keyboard();             // 处理输入

        timer += getDelay();    // 累计时间
        if (timer > delay) {    // 到达下落间隔
            timer = 0;          // 重置计时器
            drop();             // 下落方块
            render();           // 渲染画面
            update = true;      // 标记需要更新
        }

        updateMusic();  // 在游戏循环中更新音乐状态

        if (update) {           // 需要更新时
            update = false;
            render();           // 渲染画面
            clearLine();        // 检查消除行
        }

        checkOver();            // 检查游戏结束

        if (gameOver) {         // 游戏结束处理
            saveScore();        // 保存最高分
            displayOver();      // 显示结束画面
            system("pause");
            init();             // 重新开始游戏
        }
    }
}

// 左右移动方块
void Tetris::moveLeftRight(int offset)
{
    bakBlock = *curBlock;       // 备份当前状态
    curBlock->moveLeftRight(offset);  // 尝试移动

    if (!curBlock->blockInMap(map)) {  // 移动不合法
        *curBlock = bakBlock;   // 恢复之前状态
    }
}

// 旋转方块
void Tetris::rotate()
{
    bakBlock = *curBlock;       // 备份当前状态
    curBlock->rotate();         // 尝试旋转

    if (!curBlock->blockInMap(map)) {  // 旋转不合法
        *curBlock = bakBlock;   // 恢复之前状态
    }
}

// 绘制分数显示
void Tetris::drawScore()
{
    char scoreText[32];
    sprintf_s(scoreText, sizeof(scoreText), "%d", score);  // 格式化当前分数

    setcolor(RGB(0, 0, 139));   // 设置深蓝色文本

    // 设置文本样式
    LOGFONT f;
    gettextstyle(&f);           // 获取当前字体设置
    f.lfHeight = 35;            // 字体高度
    f.lfWidth = 15;             // 字体宽度
    strcpy_s(f.lfFaceName, sizeof(f.lfFaceName), _T("宋体"));  // 使用宋体
    settextstyle(&f);           // 应用字体设置

    // 绘制分数文本
    outtextxy(100, 130, "SCORE");      // 分数标签
    outtextxy(125, 170, scoreText);      // 分数值

    // 绘制最高分文本
    sprintf_s(scoreText, sizeof(scoreText), "%d", highestScore);  // 格式化最高分
    outtextxy(100, 230, "HIGHEST");    // 最高分标签
    outtextxy(125, 270, scoreText);    // 最高分值
}

// 检查游戏是否结束
void Tetris::checkOver()
{
    gameOver = (curBlock->blockInMap(map) == false);// 当前方块无法放置时游戏结束
}

// 保存最高分到文件
void Tetris::saveScore()
{
    if (score > highestScore) {  // 打破记录
        highestScore = score;

        ofstream file(RECORDER_FILE);
        file << highestScore;    // 写入新记录
        file.close();
    }
}

// 显示游戏结束画面
void Tetris::displayOver()
{
    // 停止两个音乐通道
    mciSendString("stop bgmusic1", 0, 0, 0);
    mciSendString("stop bgmusic2", 0, 0, 0);

    putimage(275, 215, &imgOver);  // 显示游戏结束图片

}

// 初始化开始界面
void Tetris::initStartScreen()
{
    //创建背景
    // 创建背景图像缓冲区
    imgStartBg = IMAGE(948, 774);
    // 获取背景图像像素缓冲区指针
    DWORD* pBuf = GetImageBuffer(&imgStartBg);
    // 生成粉色渐变背景
    for (int y = 0; y < 774; y++) {
        for (int x = 0; x < 948; x++) {
            // 计算当前行的粉色值（从上到下渐变）
            int pinkValue = 230 - y / 6;
            // 设置RGB颜色（R固定255，G/B根据位置变化）
            int r = 255;
            int g = pinkValue > 180 ? pinkValue : 180;  // 绿色分量最小值180
            int b = pinkValue > 200 ? pinkValue : 200;  // 蓝色分量最小值200
            // 设置像素颜色
            pBuf[y * 948 + x] = RGB(r, g, b);
        }
    }

    //创建普通状态按钮
    // 创建按钮图像
    imgBeginBtn = IMAGE(200, 80);
    // 设置当前绘图目标为按钮图像
    SetWorkingImage(&imgBeginBtn);
    setbkcolor(TRANSPARENT);  // 设置透明背景
    cleardevice();            // 清空图像

    // 绘制圆角矩形按钮
    setfillcolor(RGB(255, 182, 193)); // 浅粉色填充
    solidroundrect(0, 0, 200, 80, 20, 20); // 圆角半径20

    // 设置按钮文字样式
    LOGFONT f;
    gettextstyle(&f);          // 获取当前字体设置
    f.lfHeight = 40;           // 字体高度40像素
    f.lfWeight = FW_BOLD;      // 粗体
    _tcscpy_s(f.lfFaceName, _T("Comic Sans MS")); // 使用Comic Sans MS字体
    settextstyle(&f);          // 应用字体设置
    setbkmode(TRANSPARENT);    // 文字背景透明
    settextcolor(RGB(255, 105, 180)); // 深粉色文字
    outtextxy(35, 20, _T("BEGIN"));   // 绘制按钮文字

    //创建悬停状态按钮
    imgBeginBtnHover = IMAGE(200, 80);
    SetWorkingImage(&imgBeginBtnHover);
    setbkcolor(TRANSPARENT);
    cleardevice();

    // 悬停状态使用更亮的粉色
    setfillcolor(RGB(255, 215, 225));
    solidroundrect(0, 0, 200, 80, 20, 20);

    // 保持相同文字设置，但使用更深的文字颜色
    settextstyle(&f);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 20, 147)); // 更深的粉色文字
    outtextxy(50, 20, _T("BEGIN"));

    // 恢复默认绘图目标
    SetWorkingImage(NULL);

    //初始化状态变量
    isHovering = false;  // 初始不悬停
    bgOffset = 0;        // 背景偏移量初始0
    bgDirection = 1;     // 背景移动方向初始向下
}

// 渲染开始界面
void Tetris::renderStartScreen()
{
    //初始化字体
    static LOGFONT f;
    static bool isFontInitialized = false;

    if (!isFontInitialized) {
        gettextstyle(&f);  // 获取当前字体
        f.lfHeight = 40;
        f.lfWeight = FW_BOLD;
        _tcscpy_s(f.lfFaceName, _T("Comic Sans MS"));
        isFontInitialized = true;
    }

    //准备设备上下文
    HDC hdc = GetImageHDC(NULL);  // 获取窗口HDC
    HDC hdcDimmed = NULL;        // 呼吸效果HDC
    HDC hdcBtn = NULL;           // 按钮HDC

    // 开始批量绘制
    BeginBatchDraw();

    //绘制动态背景
    // 获取背景图像像素缓冲区
    DWORD* pBuf = GetImageBuffer(&imgStartBg);
    // 更新背景渐变效果（带偏移量）
    for (int y = 0; y < 774; y++) {
        int adjustedY = (y + bgOffset) % 774;  // 计算偏移后的Y坐标
        for (int x = 0; x < 948; x++) {
            int pinkValue = 230 - adjustedY / 6;
            int r = 255;
            int g = pinkValue > 180 ? pinkValue : 180;
            int b = pinkValue > 200 ? pinkValue : 200;
            pBuf[y * 948 + x] = RGB(r, g, b);
        }
    }
    // 绘制背景到窗口
    putimage(0, 0, &imgStartBg);

    // 更新背景动画位置
    bgOffset += bgDirection * 2;
    if (bgOffset > 100 || bgOffset < 0) {
        bgDirection *= -1;  // 到达边界时反向移动
    }

    //绘制标题
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 80;      // 大号字体
    titleFont.lfWeight = FW_BOLD; // 粗体
    titleFont.lfItalic = true;    // 斜体
    _tcscpy_s(titleFont.lfFaceName, _T("Comic Sans MS"));
    settextstyle(&titleFont);

    // 绘制标题文字
    settextcolor(RGB(255, 105, 180));
    outtextxy(360, 150, _T(" Tetirs "));  // 居中显示标题

    //绘制按钮
    if (isHovering) {
        // 悬停状态 - 按钮放大效果
        static float scale = 1.0f;
        scale = min(scale + 0.02f, 1.05f);  // 限制最大放大倍数

        // 创建放大后的按钮图像
        IMAGE scaledBtn(int(200 * scale), int(80 * scale));
        HDC hdcSrc = GetImageHDC(&imgBeginBtnHover);
        HDC hdcDst = GetImageHDC(&scaledBtn);

        // 高质量缩放
        SetStretchBltMode(hdcDst, HALFTONE);
        StretchBlt(hdcDst, 0, 0, 200 * scale, 80 * scale,
            hdcSrc, 0, 0, 200, 80, SRCCOPY);

        // 居中显示缩放后的按钮
        putimage(374 - (200 * scale - 200) / 2,
            400 - (80 * scale - 80) / 2,
            &scaledBtn);
    }
    else {
        // 普通状态 - 呼吸效果
        static float alpha = 0;
        alpha += 0.03f;  // 呼吸动画进度
        float brightness = 0.9f + 0.1f * sin(alpha);  // 亮度变化

        // 创建临时按钮图像
        IMAGE tempBtn(200, 80);
        SetWorkingImage(&tempBtn);

        // 根据呼吸效果调整按钮颜色
        int r = 255 * brightness;
        int g = (182 + 20 * sin(alpha)) * brightness;
        int b = (193 + 10 * sin(alpha)) * brightness;
        setfillcolor(RGB(r, g, b));
        solidroundrect(0, 0, 200, 80, 20, 20);

        // 绘制按钮文字
        settextstyle(&f);
        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 105, 180));
        outtextxy(50, 20, _T("BEGIN"));

        SetWorkingImage(NULL);
        // 绘制按钮到窗口
        putimage(374, 400, &tempBtn);
    }

    //绘制装饰元素
    static int hearts[20][3]; // [x坐标, y坐标, 类型]
    static bool initialized = false;
    if (!initialized) {
        // 初始化装饰元素位置
        for (int i = 0; i < 20; i++) {
            hearts[i][0] = rand() % 948;  // 随机x坐标
            hearts[i][1] = rand() % 774; // 随机y坐标
            hearts[i][2] = rand() % 3;   // 随机类型(0=圆形,1=方形,2=菱形)
        }
        initialized = true;
    }

    // 更新并绘制所有装饰元素
    for (int i = 0; i < 20; i++) {
        // 更新位置（下落效果）
        hearts[i][1] += 1 + rand() % 3; // 随机下落速度
        if (hearts[i][1] > 774) {       // 超出底部时重置到顶部
            hearts[i][0] = rand() % 948;
            hearts[i][1] = 0;
        }

        // 设置随机粉色系颜色
        setfillcolor(RGB(255, 150 + rand() % 50, 180 + rand() % 40));

        // 根据类型绘制不同形状
        switch (hearts[i][2]) {
        case 0:
            fillcircle(hearts[i][0], hearts[i][1], 5); // 圆形
            break;
        case 1:
            fillrectangle(hearts[i][0] - 4, hearts[i][1] - 4,
                hearts[i][0] + 4, hearts[i][1] + 4); // 方形
            break;
        case 2:
            // 菱形
            POINT pts[] = {
                {hearts[i][0], hearts[i][1] - 5},
                {hearts[i][0] + 5, hearts[i][1]},
                {hearts[i][0], hearts[i][1] + 5},
                {hearts[i][0] - 5, hearts[i][1]}
            };
            fillpolygon(pts, 4);
            break;
        }
    }

    // 结束批量绘制
    EndBatchDraw();
}

// 检查开始按钮点击
bool Tetris::checkStartClick()
{
    MOUSEMSG msg;
    if (MouseHit()) {  // 检测鼠标事件
        msg = GetMouseMsg();

        // 检查鼠标是否悬停在按钮上
        isHovering = (msg.x >= 374 && msg.x <= 574 &&   // x范围检查
            msg.y >= 400 && msg.y <= 480);    // y范围检查

        // 检查是否在悬停状态下点击了左键
        if (msg.uMsg == WM_LBUTTONDOWN && isHovering) {
            return true;  // 返回点击确认
        }
    }
    return false;  // 未点击
}
void Tetris::initMusic()
{
    mciSendString("open resource/background.mp3 alias bgmusic1", 0, 0, 0);
    mciSendString("open resource/background.mp3 alias bgmusic2", 0, 0, 0);
    mciSendString("set bgmusic1 time format milliseconds", 0, 0, 0);
    mciSendString("set bgmusic2 time format milliseconds", 0, 0, 0);
    mciSendString("play bgmusic1 repeat", 0, 0, 0);
}

void Tetris::updateMusic()
{
    static bool firstPlaying = true;
    static DWORD lastCheck = GetTickCount();

    if (GetTickCount() - lastCheck > 1000) { // 每秒检查一次
        lastCheck = GetTickCount();

        char status[32];
        mciSendString("status bgmusic1 mode", status, sizeof(status), 0);

        if (strcmp(status, "playing") != 0) {
            if (firstPlaying) {
                mciSendString("play bgmusic2 from 0 repeat", 0, 0, 0);
            }
            else {
                mciSendString("play bgmusic1 from 0 repeat", 0, 0, 0);
            }
            firstPlaying = !firstPlaying;
        }
    }
}
