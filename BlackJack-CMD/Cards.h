#pragma once
#include <string>

//����
class Cards
{
public:
	Cards();
	//����
	void ReSet();

	//��������
	void ComputeAttribute();
	
	//Ҫ�ƣ���������
	void AddCard(int num);

	//���ƣ�ͬʱ�������Ʒ�һ����
	void SplitCard(Cards* c,int n1,int n2);
	
	//��ׯ�ҽ��бȽ�
	void Compare(Cards* banker);

	//ׯ�ң���������Ƿ�ΪA
	bool IsFirA();

	//�ж��Ƿ�Ϊ�ڽܿ�����(A+10)
	bool IsBlackJack();

	//����ת����
	static std::string GetCardName(int num);
	//���ص�ǰ������ַ�����ʾ����������ʾ
	std::string GetCardsDisplayStr();
	std::string GetBankerCardsDisplayStr();
	std::string GetSumDisplayStr();
	std::string GetBankerSumDisplayStr();

public:
	//����
	//�������6��(1~6֮��Ϊ21)
	int cards[6]{0};
	//��ǰ����
	int num_cards;

	//����
	//A�ĵ���������1Ҳ������11
	int sum_min;
	int sum_max;
	//�������Ƚϣ�sum_min��sum_max����������Ҳ�����21���ֵ
	int sum;

	//״̬
	//����
	bool isSplitCards;
	//���ƣ�����21
	bool isOutRange;
	//����Ϊ21
	bool is21;
	//����
	bool isDouble;
	//��ɶԴ����ƵĲ���
	bool isFinish;
	//1��Ӯ��0��������Ϸ��-1����,2:ƽ��3���мҺڽܿ�ʤ
	int winState;

	//��ע
	int bet;
};

