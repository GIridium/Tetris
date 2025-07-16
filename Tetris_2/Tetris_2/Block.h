#pragma once
#include <graphics.h>
#include <vector>

struct Point {
	int row;
	int col;
};

class Block		//��ʾ����
{
public:
	Block();
	void drop();		//�½�
	void moveLeftRight(int offset);		//��������
	void rotate();		//��ת
	void draw(int leftMargin, int topMargin);		//�������λ��
	static IMAGE** getImages();
	Block& operator=(const Block& other);
	bool blockInMap(const std::vector<std::vector<int>>& map);//�жϷ����Ƿ�����������
	void fixation(std::vector<std::vector<int>>& map);		//�̶�����

private:
	int blockType;//��������
	Point smallBlock[4];//��ɸ��ַ������С��Ԫ
	IMAGE* img;

	static IMAGE* imgs[7];//�߸�ͼ�ε�ͼƬ
	static int size;//ͼƬ��С
};