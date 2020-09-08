#pragma once
#include <string>



class Cards
{
public:
	Cards();
	//重置
	void ReSet();

	//计算属性
	void ComputeAttribute();
	
	//要牌，更新属性
	void AddCard(int num);

	//分牌，同时给两副牌发一张牌
	void SplitCard(Cards* c,int n1,int n2);
	
	//和庄家进行比较
	void Compare(Cards* banker);

	//庄家：检查明牌是否为A
	bool IsFirA();

	//判断是否为黑杰克牌型(A+10)
	bool IsBlackJack();

	//返回当前牌面的字符串显示，仅用于显示
	std::string GetCardsDisplayStr();
	std::string GetBankerCardsDisplayStr();
	std::string GetSumDisplayStr();
	std::string GetBankerSumDisplayStr();

	static std::string GetCardName(int num);


	//牌数最多6张(1~6之和为21)
	int cards[6]{0};

	//当前牌数
	int num_cards;

	//A的点数可以是1也可以是11
	int sum_min;
	int sum_max;

	//用于最后比较，sum_min和sum_max两个中最大且不超过21点的值
	int sum;
	//是否进行了分牌
	bool isSplitCards;
	//爆牌，超出21
	bool isOutRange;
	//点数为21
	bool is21;
	//对子
	bool isDouble;

	//下注
	int bet;

	//完成对此手牌的操作
	bool isFinish;
	
	//1：赢，0：正在游戏，-1：输,2:平
	int isWin;
};

