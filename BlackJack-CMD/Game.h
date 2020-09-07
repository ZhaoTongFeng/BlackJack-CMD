#pragma once
#include <iomanip>
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <stdio.h>

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

	//是否需要更新游戏
	bool update = true;

	//累计毫秒数，
	long tickCount, currentTick;
};