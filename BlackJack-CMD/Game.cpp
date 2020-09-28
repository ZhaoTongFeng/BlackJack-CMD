#include "Game.h"
#include <string>
#include "Cards.h"

bool Game::Initialize()
{
    std::ios::sync_with_stdio(false);

    cards = new int[52];
    for (int i = 0; i < 52; i++) {
        cards[i] = i + 1;
    }

    cards_banker = new Cards();
    cards_player_1 = new Cards();
    cards_player_2 = new Cards();

    mSession = 0;
    mCoin = 10000;
    mBet = 50;
    mBase = 10.0f;
    mWinCoin = 0;
    isDebug = true;
    isShowRule = false;
    return true;
}

void Game::Loop()
{
    while (isRunning) {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::Shutdown()
{
    delete cards;
    delete cards_banker;
    delete cards_player_1;
    delete cards_player_2;
    cards_current = nullptr;
}

void Game::ProcessInput()
{
    //处理输入
    dir_coin = 0;
    dir_bet = 0;
    dir_base = 0;
    StartGame = false;
    dir_insurance = 0;

    option = -1;

    if (_kbhit()){
        int key = _getch();
        if (key == 'r') {
            isShowRule = !isShowRule;
        }
        if (key == 'p') {
            isRunning = false;
        }
        if (mSession == 0) {
            //1.下注环节

            switch (key){
                //携带筹码
            case 'q':dir_coin = 1; break;
            case 'e':dir_coin = -1; break;
                //下注
            case 'a':dir_bet = 1; break;
            case 'd':dir_bet = -1; break;
                //下注基数
            case 'z':dir_base = 1; break;
            case 'c':dir_base = -1; break;
                //开始游戏
            case 'h':StartGame = true; break;
            default:break;
            }
        }
        else if (mSession & 7) {
            //2.操作环节 A S D F
            if (mSession & 1) {
                //保险
                
                if (key == 'a') {
                    dir_insurance = 1;
                }
                else if (key == 'd') {
                    dir_insurance = -1;
                }
            }
            else if (mSession & 2) {
                switch (key)
                {
                case 'a':option = 0; break;
                case 's':option = 1; break;
                case 'd':option = 2; break;
                case 'f':option = 3; break;
                default:break;
                }
            }
            else if (mSession & 4) {
                if (key == 'h') {
                    //清理之前的数据，并回到下注环节
                    cards_banker->ReSet();
                    cards_player_1->ReSet();
                    cards_player_2->ReSet();
                    
                    mBet = mCoin > mBet ? mBet : mCoin;
                    mWinCoin = 0;
                    mSession = 0;
                }
            }
        }
    }
}

void Game::UpdateGame()
{
    if (mSession == 0) {
        //1.下注环节
        int newcoin = mCoin + dir_coin * 10000;
        if (newcoin >= 0) {
            mCoin = newcoin;
        }

        int newbet = mBet + dir_bet * static_cast<int>(mBase);
        if (newbet >= 0 && newbet <= mCoin) {
            mBet = newbet;
        }

        if(dir_base == 1&& mBase < 10000){
            mBase *= 10;
        }
        else if (dir_base == -1&& mBase > 1) {
            mBase /= 10;
        }

        if (StartGame) {
            if (mBet == 0) {
                return;
            }
            //默认只检测第一幅手牌
            cards_player_1->isFinish = false;
            cards_player_2->isFinish = true;

            cards_current = cards_player_1;

            //从筹码扣除下注
            mCoin -= mBet;
            cards_current->bet += mBet;
            //洗牌
            Shuffle();

            //发牌
            for (int i = 0; i < 2; i++) {
                HandCard(cards_banker);
                HandCard(cards_current);
            }

            
            //检查庄家明牌是否为A，如果为A则跳转1，购买保险，否则跳转2，玩家进行操作
            if (cards_banker->IsFirA()) {
                mSession = 1;
            }
            else {
                mSession = 2;
            }
        }
    }
    else if (mSession & 7) {
        //2.操作环节 A S D F
        if (mSession & 1) {
            //2.1.保险
            if (dir_insurance == 1) {
                mCoin -= static_cast<int>(floor(mBet / 2.0f));
            }
            if (dir_insurance != 0) {
                //如果玩家是黑杰克则判断是否购买了保险并结算，结束本回合
                //否则继续手牌操作
                if (cards_banker->IsBlackJack()) {

                    if (dir_insurance == 1) {
                        //购买了保险，退还下注
                        mCoin += mBet;
                        mWinCoin = -static_cast<int>(floor(mBet / 2.0f));
                    }
                    else {
                        //没有购买保险，没收所有下注
                        mWinCoin = -mBet;
                    }
                    mSession = 4;
                }
                else {
                    mSession = 2;
                    //如果闲家手牌直接超过21点,但却不是黑杰克，强制分牌，如果资金不足会导致分牌失败，直接爆牌
                    if (cards_current->isOutRange && !cards_current->IsBlackJack()) {
                        SplitCards();
                    }
                }
            }
        }
        else if (mSession & 2) {
            //2.2.手牌操作
            switch (option)
            {
            case 0:
                //要牌
                HandCard(cards_current);
                break;
            case 1:
                //停牌
                cards_current->isFinish = true;

                break;
            case 2:
                //加倍
                if (!cards_player_1->isSplitCards&& mCoin>= mBet) {
                    mCoin -= mBet;
                    cards_current->bet += mBet;
                    HandCard(cards_current);
                    cards_current->isFinish = true;
                }
                break;
            case 3:
                //分牌
                if (!cards_player_1->isSplitCards && cards_player_1->isDouble) {
                    SplitCards();
                }
                break;
            default:break;
            }

            //爆牌或者点数等于21点,强制结束此手牌操作，如果进行了分牌再进行第二幅牌的操作
            if (cards_current->is21 || cards_current->isOutRange) {
                cards_current->isFinish = true;
            }

            //切换当前手牌到第二手牌
            if (!cards_player_2->isFinish&&cards_player_1->isFinish&& cards_player_1->isSplitCards) {
                cards_current = cards_player_2;
            }

            //玩家操作结束
            if (cards_player_2->isFinish && cards_player_1->isFinish) {
                //如果庄家的点数小于17点则拿牌，直到超过17点
                while (cards_banker->sum < 17) {
                    HandCard(cards_banker);
                }
                //最后用玩家两幅手牌分别和庄家进行比较,并分发奖励
                cards_player_1->Compare(cards_banker);
                GivePrize(cards_player_1);

                if (cards_player_1->isSplitCards) {
                    cards_player_2->Compare(cards_banker);
                    GivePrize(cards_player_2);
                }

                //跳转到结算页面
                mSession = 4;
            }
        }
    }
}

void Game::Shuffle()
{
    //通过交换牌的位置打乱牌堆的顺序
    int indexA, indexB, temp;
    srand((int)time(0));
    for (int i = 0; i < 520; i++) {
        indexA = rand() % 51;
        indexB = rand() % 51;
        temp = cards[indexA];
        cards[indexA] = cards[indexB];
        cards[indexB] = temp;
    }
    //恢复牌堆栈顶指针
    cards_ptr = 52;
}

void Game::HandCard(Cards* c)
{
    c->AddCard(cards[--cards_ptr]);
}

void Game::GivePrize(Cards* c)
{
    if (c->winState == 1) {
        //赢了发双倍下注
        mCoin += static_cast<int>(floor(c->bet * 2.0f));
        mWinCoin += c->bet;
    }
    else if (c->winState == 3) {
        //玩家黑杰克获胜
        mCoin += static_cast<int>(floor(c->bet * 2.5f));
        mWinCoin += static_cast<int>(floor(c->bet * 1.5f));
    }
    else if (c->winState == 2) {
        //平局把下注退还
        mCoin += c->bet;
        mWinCoin = 0;
    }
    else if (c->winState == -1) {
        mWinCoin -= c->bet;
    }
}

void Game::SplitCards()
{
    if (mCoin >= mBet) {
        int n1 = cards[--cards_ptr];
        int n2 = cards[--cards_ptr];
        cards_player_1->SplitCard(cards_player_2, n1, n2);
        mCoin -= mBet;
        cards_player_2->bet += mBet;
        cards_player_2->isFinish = false;
    }
}

void Game::GenerateOutput() {
    
    //输出图像
    system("cls");//清屏

    std::string disStr = "";

    disStr+= "*****BlackJack/21点控制台版*****\n";
    disStr+= "显示规则：R\n";

    if (isShowRule) {
        disStr+= "游戏规则：\n";
        disStr+= "闲家和庄家单独比较点数，黑杰克（A+10+）最大\n";
        disStr+= "要牌：\n";
        disStr+= "条件：当前手牌点数不超过21点\n";
        disStr+= "花费：无\n";
        disStr+= "效果：获得一张牌，和现有的牌点数相加，如果超过21点则爆牌，等于21点则停牌，小于21点可以继续要牌、停牌、加倍\n";
        disStr+= "\n";
        disStr+= "停牌：结束此回合，等待结果\n";
        disStr+= "条件：无\n";
        disStr+= "花费：无\n";
        disStr+= "\n";
        disStr+= "加倍：\n";
        disStr+= "条件：没有分牌\n";
        disStr+= "花费：下注筹码数\n";
        disStr+= "效果：获得一张牌，不管是否超过21点，结束此手牌操作，如果最后胜利，可获得双倍奖励\n";
        disStr+= "\n";
        disStr+= "分牌：\n";
        disStr+= "条件：闲家手牌为对子，或者开局点数超过21点\n";
        disStr+= "花费：下注筹码数\n";
        disStr+= "效果：将两张牌分成两份，由同一玩家进行操作，分牌后不能加倍，拿到BlackJack牌型也算普通21点\n";
        disStr+= "\n";
        disStr+= "保险：\n";
        disStr+= "条件：庄家第一张牌为A，玩家需要选择是否购买保险。\n";
        disStr+= "花费：一半下注筹码数\n";
        disStr+= "购买：庄家是黑杰克（A+10)，庄家只赢得保险金，否则收走保险金继续后面的步骤\n";
        disStr+= "不购买：庄家为黑杰克则没收全部下注\n";
        disStr+= "*****************************************************************\n";
        disStr+= "\n";
    }

    if (mSession == 0) {
        disStr += "名称\t数量\t增加\t减少\n";
        disStr += "筹码\t" + std::to_string(mCoin) + "\tQ\tE\n";
        disStr += "下注\t" + std::to_string(mBet) + "\tA\tD\n";
        disStr += "基数\t" + std::to_string(static_cast<int>(mBase)) + "\tZ\tC\n\n";
        disStr += "开始游戏：H\n";
    }
    else if (mSession & 7) {
        disStr += "筹码\t下注\n";
        disStr += std::to_string(mCoin) + "\t" + std::to_string(mBet) + "\n\n";
        disStr += "牌面\n";

        //没有结束前庄家只显示一张牌
        if (mSession & 4) {
            disStr += "庄家：" + cards_banker->GetCardsDisplayStr() + "\t点数：" + cards_banker->GetSumDisplayStr() + "\n";
        }
        else {
            disStr += "庄家：" + cards_banker->GetBankerCardsDisplayStr() + "\t点数：" + cards_banker->GetBankerSumDisplayStr() + "\n";
        }
        disStr += "闲家：\n";
        disStr += "手牌：" + cards_player_1->GetCardsDisplayStr() + "\t点数：" + cards_player_1->GetSumDisplayStr();
        

        if (cards_player_1->winState == 1) {
            disStr += "\t胜利";
        }
        else if (cards_player_1->winState == -1) {
            disStr += "\t失败";
        }
        disStr += "\n";
        if (cards_player_1->isSplitCards) {
            disStr += "手牌：" + cards_player_2->GetCardsDisplayStr() + "\t点数：" + cards_player_2->GetSumDisplayStr();
            if (cards_player_2->winState == 1) {
                disStr += "\t胜利";
            }
            else if (cards_player_2->winState == -1) {
                disStr += "\t失败";
            }
            disStr += "\n";
        }
        disStr += "\n"; 
        if (isDebug) {
            disStr += "牌堆：\n";
            for (int i = 0; i < 52; i++) {
                disStr += Cards::GetCardName(cards[i]) + " ";
            }
            disStr += "\n";
            disStr += "当前牌堆：\n";
            for (int i = 0; i < cards_ptr; i++) {
                disStr += Cards::GetCardName(cards[i]) + " ";
            }
            disStr += "\n";
            disStr += "庄家手牌：\n";
            for (int i = 0; i < cards_banker->num_cards; i++) {
                disStr += Cards::GetCardName(cards_banker->cards[i]) + " ";
            }
            disStr += "\n";
            disStr += "闲家手牌：\n";
            for (int i = 0; i < cards_player_1->num_cards; i++) {
                disStr += Cards::GetCardName(cards_player_1->cards[i]) + " ";
            }
            disStr += "\n";
            disStr += "\n";
        }

        if (mSession & 1) {
            disStr += "庄家明牌为A，是否购买保险？\n";
            disStr += "A：购买保险\n";
            disStr += "D：不购买保险\n";
        }
        else if(mSession & 2){
            if (cards_current == cards_player_1) {
                disStr += "正在操作手牌一\n";
            }else if (cards_current == cards_player_2) {
                disStr += "正在操作手牌二\n";
            }
            disStr += "A：要牌\n";
            disStr += "S：停牌\n";
            if (!cards_player_1->isSplitCards && mCoin >= mBet) {
                disStr += "D：加倍\n";
            }
            if (!cards_player_1->isSplitCards&& cards_player_1->isDouble) {
                disStr += "F：分牌\n";
            }
        }
        else if (mSession & 4) {
            if (mWinCoin == 0) {
                disStr += "平局\n";
            }
            else if (mWinCoin > 0) {
                disStr += "你赢得了" + abs(mWinCoin);
                disStr += "筹码\n";
            }
            else if (mWinCoin < 0) {
                disStr += "你损失了" + abs(mWinCoin);
                disStr += "筹码\n";
            }
            disStr += "回到下注页面：H\n";
        }
    }
    std::cout << disStr;
}

