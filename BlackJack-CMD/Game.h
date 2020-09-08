#pragma once

#include <time.h>
#include <conio.h>
#include <iostream>


class Game {
public:
	//初始化
	bool Initialize();
	
	//游戏循环
	void Loop();
	
	//游戏结束
	void Shutdown();

private:
	//按键输入
	void ProcessInput();
	
	//更新游戏
	void UpdateGame();
	
	//输出图像
	void GenerateOutput();

	//游戏是否结束
	bool isRunning = true;

	///////////////////////////////////////////////////
	//洗牌
	void Shuffle();
	
	//发牌
	void HandCard(class Cards* c);

	//分牌
	void SplitCards();

	//分发奖励
	void GivePrize(class Cards* c);

	//牌堆
	int* cards;
	//牌堆栈顶指针
	int cards_ptr;
	
	//庄家手牌
	class Cards* cards_banker;
	//玩家手牌一
	class Cards* cards_player_1;
	//玩家手牌二
	class Cards* cards_player_2;
	//玩家当前正在操作的手牌
	class Cards* cards_current;

	//当前阶段
	int mSession;

	//输入状态：0无操作，负数减少，正数增加
	int dir_base;
	int dir_bet;
	int dir_coin;
	int dir_insurance;
	int option;
	bool StartGame;

	//下注基数
	float mBase;

	//当前下注
	int mBet;

	//玩家筹码
	int mCoin;

	//此回合获胜筹码数
	int mWinCoin;

	//作弊模式
	bool isDebug;

	//显示规则
	bool isShowRule;
};