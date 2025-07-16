#include <cstdlib>
#include "Block.h"

// ��̬��Ա��ʼ��
IMAGE* Block::imgs[7] = { NULL, };// �洢7�ַ���ͼƬ��ָ������
int Block::size = 35;			// ÿ��С��������سߴ�

Block::Block()
{
	// �״δ�������ʱ����ͼƬ��Դ
	if (imgs[0] == NULL) {
		IMAGE imgTmp;
		loadimage(&imgTmp, "resource/block.png");//��ͼƬ���ص��ڴ�

		// ���ز�ͼ���и��7�ַ���ͼƬ
		SetWorkingImage(&imgTmp);
		for (int i = 0; i < 7; i++) {
			imgs[i] = new IMAGE;
			getimage(imgs[i], i * size, 0, size, size);// ��˳���ȡÿ������
		}
		SetWorkingImage();//�ָ�������
	}

	// ����7�ַ���ĳ�ʼ��״�������ֱ����ʾ���λ�ã�
	int block[7][4] = {
		1,3,5,7,//ֱ��
		2,4,5,7,//��Z
		3,4,5,6,//Z
		3,4,5,7,//͹
		2,3,5,7,//L
		3,5,6,7,//��L
		2,3,4,5,//������
	};

	//�������һ�ַ��飨1-7��
	blockType = 1 + rand() % 7;

	// ��ʼ�������4��С��������
	for (int i = 0; i < 4; i++) {
		int value = block[blockType - 1][i]; // ��ȡ��ǰ���͵ı���ֵ
		smallBlock[i].row = value / 2;//����ÿ��С���������������block��ά������������ֳ���2����������
		smallBlock[i].col = (value % 2) + 4;//����ÿ��С���������������block��ά������������ֳ���2�����������ټ�4

	}

	img = imgs[blockType - 1];// ������Ӧ�ķ���ͼƬ
}

void Block::drop()
{
	for (int i = 0; i < 4; i++) {	//С�������� ����С����������+1
		smallBlock[i].row++;
	}
}

// ���������ƶ�������С����������ƫ��
void Block::moveLeftRight(int offset)
{
	for (int i = 0; i < 4; i++) {
		smallBlock[i].col += offset;
	}
}

// ���Ʒ��飺��ָ��ƫ��λ�û���4��С����
void Block::draw(int leftMargin, int topMargin)
{
	for (int i = 0; i < 4; i++) {
		int x = leftMargin + smallBlock[i].col * size;// ����ʵ����ĻX����
		int y = topMargin + smallBlock[i].row * size; // ����ʵ����ĻY����
		putimage(x, y, img); // ����С����ͼƬ
	}
}

// ��ȡ���з���ͼƬ��ָ������
IMAGE** Block::getImages()
{
	return imgs;
}

// ��ֵ��������أ������������
Block& Block::operator=(const Block& other)
{
	if (this == &other) {
		return *this;// �����Ը�ֵ
	}
	this->blockType = other.blockType;
	for (int i = 0; i < 4; i++) {
		this->smallBlock[i] = other.smallBlock[i];// ����ÿ��С��������
	}
	return *this;
}

// ��ⷽ���Ƿ��ںϷ���ͼ��Χ��
bool Block::blockInMap(const std::vector<std::vector<int>>& map)
{
	int rows = map.size();
	int cols = map[0].size();
	for (int i = 0; i < 4; i++) {// ����Ƿ�Խ��������з����ص�
		if (smallBlock[i].col < 0 || smallBlock[i].col >= cols ||
			smallBlock[i].row < 0 || smallBlock[i].row >= rows ||
			map[smallBlock[i].row][smallBlock[i].col]) {
			return false;
		}
	}
	return true;
}

// ������̶�����ͼ�У����Ϊ�����ƶ���
void Block::fixation(std::vector<std::vector<int>>& map)
{
	for (int i = 0; i < 4; i++) {	//�̶�����
		map[smallBlock[i].row][smallBlock[i].col] = blockType;// �÷������ͱ�ű�ǵ�ͼ
	}
}

// ������ת���������ĵ����90����ת�任
void Block::rotate()
{
	Point p = smallBlock[1];		//ʹ�õ�2��С������Ϊ��ת����

	for (int i = 0; i < 4; i++) {
		Point tmp = smallBlock[i];
		smallBlock[i].col = p.col - tmp.row + p.row;
		smallBlock[i].row = p.row + tmp.col - p.col;//��ʽ��Դ��bվup ����ԱRock
	}
}