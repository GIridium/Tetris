#include <iostream>
#include <time.h>
#include <cstdlib>
#include <graphics.h>
#include <conio.h>
#include <mmsystem.h>
#include <fstream>
#include <memory>

#pragma comment(lib, "winmm.lib")	// ���ض�ý�徲̬��

#include "Tetris.h"
#include "Block.h"

#define MAX_LEVEL 5                // ��Ϸ��ߵȼ�
#define RECORDER_FILE "recorder.txt"	// �������ô洢��߷ֵ��ļ���

const int SPEED_NORMAL = 500;	// ���������ٶ�(����)
const int SPEED_QUICK = 50;		// ���������ٶ�(����)

// ���캯������ʼ����Ϸ�������
Tetris::Tetris(int rows, int cols, int left, int top, int blockSize)
{
    this->rows = rows;          // ��Ϸ��������
    this->cols = cols;          // ��Ϸ��������
    leftMargin = left;          // ������߽�����ֵ
    topMargin = top;            // �����ϱ߽�����ֵ
    this->blockSize = blockSize; // ÿ������Ĵ�С(����)

    // ��ʼ����Ϸ��ͼ(��ά����)
    for (int i = 0; i < rows; i++) {
        vector<int>mapRow;
        for (int j = 0; j < cols; j++) {
            mapRow.push_back(0);  // 0��ʾ��λ��Ϊ��
        }
        map.push_back(mapRow);
    }
}

// ��ʼ����Ϸ״̬
void Tetris::init()
{
    // ������������
    initMusic();  // ��ʼ�����ֲ���ϵͳ

    delay = SPEED_NORMAL;       // ���ó�ʼ�����ٶ�

    srand(time(NULL));          // �������������

    // ������Ϸ����
    initgraph(948, 774);        // ��ʼ��ͼ�δ���
    setbkcolor(WHITE);          // ���ñ�����ɫΪ��ɫ
    cleardevice();              // ����\

    // ��ʼ����ʼ����
    initStartScreen();

    // ��ʾ��ʼ����ѭ��
    while (true) {
        renderStartScreen();

        if (checkStartClick()) {
            break; // �����ʼ��ť�������Ϸ
        }

        Sleep(10); // ��ֹCPUռ�ù���
    }

    // ����ͼƬ��Դ
    loadimage(&imgBg, "resource/background.png");  // ����ͼƬ
    loadimage(&imgOver, "resource/gameOver.png");  // ��Ϸ����ͼƬ

    // ������Ϸ��ͼ
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            map[i][j] = 0;      // ��յ�ͼ
        }
    }

    score = 0;                  // ���õ�ǰ����

    // ���ļ���ȡ��߷ּ�¼
    ifstream file(RECORDER_FILE);
    if (!file.is_open()) {
        std::cout << RECORDER_FILE << "��ʧ��" << std::endl;
        highestScore = 0;       // �ļ���ʧ������߷�Ϊ0
    }
    else {
        file >> highestScore;    // ���ļ���ȡ��߷�
    }
    file.close();               // �ر��ļ�

    gameOver = false;           // ������Ϸ״̬
}

// �����������
void Tetris::keyboard()
{
    unsigned char ch;
    bool rotateFlag = false;    // ��ת��־
    int dx = 0;                 // ˮƽ�ƶ�����

    if (_kbhit()) {             // ����������
        ch = _getch();          // ��ȡ����(����Ҫ�س�)
        if (ch == 224) {        // ��չ��(�����)
            ch = _getch();      // ��ȡ��չ����
            switch (ch) {
            case 72: rotateFlag = true; break;  // �ϼ�ͷ - ��ת
            case 80: delay = SPEED_QUICK; break; // �¼�ͷ - ��������
            case 75: dx = -1; break;            // ���ͷ - ����
            case 77: dx = 1; break;             // �Ҽ�ͷ - ����
            default:break;
            }
        }
        if (ch == 27) {         // ESC�� - �˳���Ϸ
            system("pause");
            exit(0);
        }
    }

    if (rotateFlag) {           // ������ת
        rotate();
        update = true;          // ��Ҫ���»���
    }
    if (dx != 0) {              // ����ˮƽ�ƶ�
        moveLeftRight(dx);
        update = true;          // ��Ҫ���»���
    }
}

// ��Ⱦ��Ϸ����
void Tetris::render()
{
    putimage(0, 0, &imgBg);     // ���Ʊ���ͼƬ

    IMAGE** imgs = Block::getImages();  // ��ȡ����ͼƬ��Դ
    BeginBatchDraw();           // ��ʼ��������
    // �����ѹ̶��ķ���
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (map[i][j] == 0) continue;  // ������λ��
            int x = j * blockSize + leftMargin;  // ������Ļ����
            int y = i * blockSize + topMargin;
            putimage(x, y, imgs[map[i][j] - 1]);  // ���Ʒ���
        }
    }

    // ���Ƶ�ǰ�����еķ���
    curBlock->draw(leftMargin, topMargin);
    // ������һ������Ԥ��
    nextBlock->draw(621, 96);

    drawScore();                // ���Ʒ�����ʾ

    EndBatchDraw();             // ������������
}

// ��ȡʱ����(����)
int Tetris::getDelay()
{
    static unsigned long long lastTime = 0;
    unsigned long long currentTime = GetTickCount();

    if (lastTime == 0) {        // ��һ�ε���
        lastTime = currentTime;
        return 0;
    }
    else {
        int ret = currentTime - lastTime;  // ����ʱ���
        lastTime = currentTime;
        return ret;
    }
}

// ���������߼�
void Tetris::drop()
{
    bakBlock = *curBlock;       // ���ݵ�ǰ����״̬
    curBlock->drop();           // ��������

    if (!curBlock->blockInMap(map)) {  // ���䵽�ײ�����ײ
        bakBlock.fixation(map);  // ������̶�����ͼ��
        curBlock = std::move(nextBlock);  // ת����һ����������Ȩ
        nextBlock = std::make_unique<Block>();  // �����·���
    }

    delay = SPEED_NORMAL;       // ���������ٶ�
}

// ��������
void Tetris::clearLine()
{
    int lines = 0;              // ������������
    int k = rows - 1;           // �ӵײ���ʼɨ��
    for (int i = rows - 1; i >= 0; i--) {
        int count = 0;          // ��ǰ�з������
        for (int j = 0; j < cols; j++) {
            if (map[i][j]) {
                count++;        // ͳ�Ƶ�ǰ�з�����
            }
            map[k][j] = map[i][j];  // �ع���ͼ
        }

        if (count < cols) {     // ��������
            k--;               // ��������
        }
        else {                  // ����
            lines++;           // ������������
        }
    }

    if (lines > 0) {           // ����÷ֲ�������Ч
        int addScore[4] = { 10,30,60,80 };  // ����1-4�еĵ÷�
        score += addScore[lines - 1];      // ���ӷ���

        mciSendString("play resource/sound1.mp3", 0, 0, 0);  // ����������Ч
        update = true;          // ��Ҫ���»���
    }
}

// ����Ϸѭ��
void Tetris::play()
{
    init();                     // ��ʼ����Ϸ

    // ��ʼ������
    nextBlock = std::make_unique<Block>();
    curBlock = std::move(nextBlock);  // ת������Ȩ
    nextBlock = std::make_unique<Block>();

    int timer = 0;              // ��ʱ��
    while (1) {
        keyboard();             // ��������

        timer += getDelay();    // �ۼ�ʱ��
        if (timer > delay) {    // ����������
            timer = 0;          // ���ü�ʱ��
            drop();             // ���䷽��
            render();           // ��Ⱦ����
            update = true;      // �����Ҫ����
        }

        updateMusic();  // ����Ϸѭ���и�������״̬

        if (update) {           // ��Ҫ����ʱ
            update = false;
            render();           // ��Ⱦ����
            clearLine();        // ���������
        }

        checkOver();            // �����Ϸ����

        if (gameOver) {         // ��Ϸ��������
            saveScore();        // ������߷�
            displayOver();      // ��ʾ��������
            system("pause");
            init();             // ���¿�ʼ��Ϸ
        }
    }
}

// �����ƶ�����
void Tetris::moveLeftRight(int offset)
{
    bakBlock = *curBlock;       // ���ݵ�ǰ״̬
    curBlock->moveLeftRight(offset);  // �����ƶ�

    if (!curBlock->blockInMap(map)) {  // �ƶ����Ϸ�
        *curBlock = bakBlock;   // �ָ�֮ǰ״̬
    }
}

// ��ת����
void Tetris::rotate()
{
    bakBlock = *curBlock;       // ���ݵ�ǰ״̬
    curBlock->rotate();         // ������ת

    if (!curBlock->blockInMap(map)) {  // ��ת���Ϸ�
        *curBlock = bakBlock;   // �ָ�֮ǰ״̬
    }
}

// ���Ʒ�����ʾ
void Tetris::drawScore()
{
    char scoreText[32];
    sprintf_s(scoreText, sizeof(scoreText), "%d", score);  // ��ʽ����ǰ����

    setcolor(RGB(0, 0, 139));   // ��������ɫ�ı�

    // �����ı���ʽ
    LOGFONT f;
    gettextstyle(&f);           // ��ȡ��ǰ��������
    f.lfHeight = 35;            // ����߶�
    f.lfWidth = 15;             // ������
    strcpy_s(f.lfFaceName, sizeof(f.lfFaceName), _T("����"));  // ʹ������
    settextstyle(&f);           // Ӧ����������

    // ���Ʒ����ı�
    outtextxy(100, 130, "SCORE");      // ������ǩ
    outtextxy(125, 170, scoreText);      // ����ֵ

    // ������߷��ı�
    sprintf_s(scoreText, sizeof(scoreText), "%d", highestScore);  // ��ʽ����߷�
    outtextxy(100, 230, "HIGHEST");    // ��߷ֱ�ǩ
    outtextxy(125, 270, scoreText);    // ��߷�ֵ
}

// �����Ϸ�Ƿ����
void Tetris::checkOver()
{
    gameOver = (curBlock->blockInMap(map) == false);// ��ǰ�����޷�����ʱ��Ϸ����
}

// ������߷ֵ��ļ�
void Tetris::saveScore()
{
    if (score > highestScore) {  // ���Ƽ�¼
        highestScore = score;

        ofstream file(RECORDER_FILE);
        file << highestScore;    // д���¼�¼
        file.close();
    }
}

// ��ʾ��Ϸ��������
void Tetris::displayOver()
{
    // ֹͣ��������ͨ��
    mciSendString("stop bgmusic1", 0, 0, 0);
    mciSendString("stop bgmusic2", 0, 0, 0);

    putimage(275, 215, &imgOver);  // ��ʾ��Ϸ����ͼƬ

}

// ��ʼ����ʼ����
void Tetris::initStartScreen()
{
    //��������
    // ��������ͼ�񻺳���
    imgStartBg = IMAGE(948, 774);
    // ��ȡ����ͼ�����ػ�����ָ��
    DWORD* pBuf = GetImageBuffer(&imgStartBg);
    // ���ɷ�ɫ���䱳��
    for (int y = 0; y < 774; y++) {
        for (int x = 0; x < 948; x++) {
            // ���㵱ǰ�еķ�ɫֵ�����ϵ��½��䣩
            int pinkValue = 230 - y / 6;
            // ����RGB��ɫ��R�̶�255��G/B����λ�ñ仯��
            int r = 255;
            int g = pinkValue > 180 ? pinkValue : 180;  // ��ɫ������Сֵ180
            int b = pinkValue > 200 ? pinkValue : 200;  // ��ɫ������Сֵ200
            // ����������ɫ
            pBuf[y * 948 + x] = RGB(r, g, b);
        }
    }

    //������ͨ״̬��ť
    // ������ťͼ��
    imgBeginBtn = IMAGE(200, 80);
    // ���õ�ǰ��ͼĿ��Ϊ��ťͼ��
    SetWorkingImage(&imgBeginBtn);
    setbkcolor(TRANSPARENT);  // ����͸������
    cleardevice();            // ���ͼ��

    // ����Բ�Ǿ��ΰ�ť
    setfillcolor(RGB(255, 182, 193)); // ǳ��ɫ���
    solidroundrect(0, 0, 200, 80, 20, 20); // Բ�ǰ뾶20

    // ���ð�ť������ʽ
    LOGFONT f;
    gettextstyle(&f);          // ��ȡ��ǰ��������
    f.lfHeight = 40;           // ����߶�40����
    f.lfWeight = FW_BOLD;      // ����
    _tcscpy_s(f.lfFaceName, _T("Comic Sans MS")); // ʹ��Comic Sans MS����
    settextstyle(&f);          // Ӧ����������
    setbkmode(TRANSPARENT);    // ���ֱ���͸��
    settextcolor(RGB(255, 105, 180)); // ���ɫ����
    outtextxy(35, 20, _T("BEGIN"));   // ���ư�ť����

    //������ͣ״̬��ť
    imgBeginBtnHover = IMAGE(200, 80);
    SetWorkingImage(&imgBeginBtnHover);
    setbkcolor(TRANSPARENT);
    cleardevice();

    // ��ͣ״̬ʹ�ø����ķ�ɫ
    setfillcolor(RGB(255, 215, 225));
    solidroundrect(0, 0, 200, 80, 20, 20);

    // ������ͬ�������ã���ʹ�ø����������ɫ
    settextstyle(&f);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 20, 147)); // ����ķ�ɫ����
    outtextxy(50, 20, _T("BEGIN"));

    // �ָ�Ĭ�ϻ�ͼĿ��
    SetWorkingImage(NULL);

    //��ʼ��״̬����
    isHovering = false;  // ��ʼ����ͣ
    bgOffset = 0;        // ����ƫ������ʼ0
    bgDirection = 1;     // �����ƶ������ʼ����
}

// ��Ⱦ��ʼ����
void Tetris::renderStartScreen()
{
    //��ʼ������
    static LOGFONT f;
    static bool isFontInitialized = false;

    if (!isFontInitialized) {
        gettextstyle(&f);  // ��ȡ��ǰ����
        f.lfHeight = 40;
        f.lfWeight = FW_BOLD;
        _tcscpy_s(f.lfFaceName, _T("Comic Sans MS"));
        isFontInitialized = true;
    }

    //׼���豸������
    HDC hdc = GetImageHDC(NULL);  // ��ȡ����HDC
    HDC hdcDimmed = NULL;        // ����Ч��HDC
    HDC hdcBtn = NULL;           // ��ťHDC

    // ��ʼ��������
    BeginBatchDraw();

    //���ƶ�̬����
    // ��ȡ����ͼ�����ػ�����
    DWORD* pBuf = GetImageBuffer(&imgStartBg);
    // ���±�������Ч������ƫ������
    for (int y = 0; y < 774; y++) {
        int adjustedY = (y + bgOffset) % 774;  // ����ƫ�ƺ��Y����
        for (int x = 0; x < 948; x++) {
            int pinkValue = 230 - adjustedY / 6;
            int r = 255;
            int g = pinkValue > 180 ? pinkValue : 180;
            int b = pinkValue > 200 ? pinkValue : 200;
            pBuf[y * 948 + x] = RGB(r, g, b);
        }
    }
    // ���Ʊ���������
    putimage(0, 0, &imgStartBg);

    // ���±�������λ��
    bgOffset += bgDirection * 2;
    if (bgOffset > 100 || bgOffset < 0) {
        bgDirection *= -1;  // ����߽�ʱ�����ƶ�
    }

    //���Ʊ���
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 80;      // �������
    titleFont.lfWeight = FW_BOLD; // ����
    titleFont.lfItalic = true;    // б��
    _tcscpy_s(titleFont.lfFaceName, _T("Comic Sans MS"));
    settextstyle(&titleFont);

    // ���Ʊ�������
    settextcolor(RGB(255, 105, 180));
    outtextxy(360, 150, _T(" Tetirs "));  // ������ʾ����

    //���ư�ť
    if (isHovering) {
        // ��ͣ״̬ - ��ť�Ŵ�Ч��
        static float scale = 1.0f;
        scale = min(scale + 0.02f, 1.05f);  // �������Ŵ���

        // �����Ŵ��İ�ťͼ��
        IMAGE scaledBtn(int(200 * scale), int(80 * scale));
        HDC hdcSrc = GetImageHDC(&imgBeginBtnHover);
        HDC hdcDst = GetImageHDC(&scaledBtn);

        // ����������
        SetStretchBltMode(hdcDst, HALFTONE);
        StretchBlt(hdcDst, 0, 0, 200 * scale, 80 * scale,
            hdcSrc, 0, 0, 200, 80, SRCCOPY);

        // ������ʾ���ź�İ�ť
        putimage(374 - (200 * scale - 200) / 2,
            400 - (80 * scale - 80) / 2,
            &scaledBtn);
    }
    else {
        // ��ͨ״̬ - ����Ч��
        static float alpha = 0;
        alpha += 0.03f;  // ������������
        float brightness = 0.9f + 0.1f * sin(alpha);  // ���ȱ仯

        // ������ʱ��ťͼ��
        IMAGE tempBtn(200, 80);
        SetWorkingImage(&tempBtn);

        // ���ݺ���Ч��������ť��ɫ
        int r = 255 * brightness;
        int g = (182 + 20 * sin(alpha)) * brightness;
        int b = (193 + 10 * sin(alpha)) * brightness;
        setfillcolor(RGB(r, g, b));
        solidroundrect(0, 0, 200, 80, 20, 20);

        // ���ư�ť����
        settextstyle(&f);
        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 105, 180));
        outtextxy(50, 20, _T("BEGIN"));

        SetWorkingImage(NULL);
        // ���ư�ť������
        putimage(374, 400, &tempBtn);
    }

    //����װ��Ԫ��
    static int hearts[20][3]; // [x����, y����, ����]
    static bool initialized = false;
    if (!initialized) {
        // ��ʼ��װ��Ԫ��λ��
        for (int i = 0; i < 20; i++) {
            hearts[i][0] = rand() % 948;  // ���x����
            hearts[i][1] = rand() % 774; // ���y����
            hearts[i][2] = rand() % 3;   // �������(0=Բ��,1=����,2=����)
        }
        initialized = true;
    }

    // ���²���������װ��Ԫ��
    for (int i = 0; i < 20; i++) {
        // ����λ�ã�����Ч����
        hearts[i][1] += 1 + rand() % 3; // ��������ٶ�
        if (hearts[i][1] > 774) {       // �����ײ�ʱ���õ�����
            hearts[i][0] = rand() % 948;
            hearts[i][1] = 0;
        }

        // ���������ɫϵ��ɫ
        setfillcolor(RGB(255, 150 + rand() % 50, 180 + rand() % 40));

        // �������ͻ��Ʋ�ͬ��״
        switch (hearts[i][2]) {
        case 0:
            fillcircle(hearts[i][0], hearts[i][1], 5); // Բ��
            break;
        case 1:
            fillrectangle(hearts[i][0] - 4, hearts[i][1] - 4,
                hearts[i][0] + 4, hearts[i][1] + 4); // ����
            break;
        case 2:
            // ����
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

    // ������������
    EndBatchDraw();
}

// ��鿪ʼ��ť���
bool Tetris::checkStartClick()
{
    MOUSEMSG msg;
    if (MouseHit()) {  // �������¼�
        msg = GetMouseMsg();

        // �������Ƿ���ͣ�ڰ�ť��
        isHovering = (msg.x >= 374 && msg.x <= 574 &&   // x��Χ���
            msg.y >= 400 && msg.y <= 480);    // y��Χ���

        // ����Ƿ�����ͣ״̬�µ�������
        if (msg.uMsg == WM_LBUTTONDOWN && isHovering) {
            return true;  // ���ص��ȷ��
        }
    }
    return false;  // δ���
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

    if (GetTickCount() - lastCheck > 1000) { // ÿ����һ��
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
