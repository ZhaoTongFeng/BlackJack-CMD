#include "Game.h"
#include <string>
#include "Cards.h"

bool Game::Initialize()
{
    //52张牌全部添加到牌堆
    cards = new int[52];
    for (int i = 0; i < 52; i++) {
        cards[i] = i + 1;
    }

    cards_banker = new Cards();
    cards_player_1 = new Cards();
    cards_player_2 = new Cards();

    mSession = 0;

    mCoin = 10000;
    mBet = 0;
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

        if(dir_base == 1){
            if (mBase < 10000) {
                mBase *= 10;
            }
        }
        else if (dir_base == -1) {
            if (mBase > 1) {
                mBase /= 10;
            }
        }
        if (StartGame) {
            if (mBet == 0) {
                return;
            }
            //默认只检测第一幅手牌
            cards_player_1->isFinish = false;
            cards_player_2->isFinish = true;

            //默认为手牌一
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
            if (cards_current->isOutRange) {
                SplitCards();
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
            //保险
            if (dir_insurance == 1) {
                mCoin -= static_cast<int>(floor(mBet / 2.0f));

            }
            if (dir_insurance != 0) {
                //庄家检查另外一张底牌，如果是10点直接结束游戏
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
                }

            }
        }
        else if (mSession & 2) {
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
                //对当前的手牌进行加倍，加倍之后停牌
                if (!cards_player_1->isSplitCards) {
                    mCoin -= mBet;
                    cards_current->bet += mBet;
                    HandCard(cards_current);
                    cards_current->isFinish = true;
                }
                break;
            case 3:
                //没有进行过分牌并且手牌一为对子
                if (!cards_player_1->isSplitCards && cards_player_1->isDouble) {
                    SplitCards();
                }
                break;
            default:break;
            }
            //爆牌或者点数等于21点,结束此手牌操作，如果进行了分牌再进行第二幅牌的操作
            if (cards_current->is21 || cards_current->isOutRange) {
                cards_current->isFinish = true;
                cards_current->isWin = -1;
                if (cards_player_1->isSplitCards) {
                    cards_current = cards_player_2;
                }
            }
            //都结束之后,如果庄家的点数小于17点则拿牌，直到超过17点
            if (cards_player_2->isFinish && cards_player_1->isFinish) {
                while (cards_banker->sum < 17) {
                    HandCard(cards_banker);
                }
                //最后对两幅手牌分别进行比较,然后分发奖励
                cards_player_1->Compare(cards_banker);
                GivePrize(cards_player_1);

                if (cards_player_1->isSplitCards) {
                    cards_player_2->Compare(cards_banker);
                    GivePrize(cards_player_2);
                }

                //最后跳转到结算环节
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
    if (c->isWin == 1) {
        //赢了发双倍下注
        mCoin += c->bet * 2;
        mWinCoin += c->bet;
    }
    else if (c->isWin == 2) {
        //平局把下注退还
        mCoin += c->bet;
        mWinCoin = 0;
    }
    else if (c->isWin == -1) {
        mWinCoin -= c->bet;
    }
}

void Game::SplitCards()
{
    if (mCoin >= mBet) {
        int n1 = cards[--cards_ptr];
        int n2 = cards[--cards_ptr];
        cards_player_1->SplitCard(cards_player_2, n1, n2);
        //给手牌二下注
        mCoin -= mBet;
        cards_player_2->bet += mBet;
        cards_player_2->isFinish = false;
    }
}





void Game::GenerateOutput() {
    
    //输出图像
    system("cls");//清屏
    std::cout << "*****BlackJack/21点控制台版*****\n";
    std::cout << "显示规则：R\n";
    if (isShowRule) {
        std::cout << "游戏规则：\n";
        std::cout << "闲家和庄家单独比较点数，黑杰克（A+10+）最大\n";
        std::cout << "要牌：\n";
        std::cout << "条件：当前手牌点数不超过21点\n";
        std::cout << "花费：无\n";
        std::cout << "效果：获得一张牌，和现有的牌点数相加，如果超过21点则爆牌，等于21点则停牌，小于21点可以继续要牌、停牌、加倍\n";
        std::cout << "\n";
        std::cout << "停牌：结束此回合，等待结果\n";
        std::cout << "条件：无\n";
        std::cout << "花费：无\n";
        std::cout << "\n";
        std::cout << "加倍：\n";
        std::cout << "条件：没有分牌\n";
        std::cout << "花费：下注筹码数\n";
        std::cout << "效果：获得一张牌，不管是否超过21点，结束此手牌操作，如果最后胜利，可获得双倍奖励\n";
        std::cout << "\n";
        std::cout << "分牌：\n";
        std::cout << "条件：闲家手牌为对子，或者开局点数超过21点\n";
        std::cout << "花费：下注筹码数\n";
        std::cout << "效果：将两张牌分成两份，由同一玩家进行操作，分牌后不能加倍，拿到BlackJack牌型也算普通21点\n";
        std::cout << "\n";
        std::cout << "保险：\n";
        std::cout << "条件：庄家第一张牌为A，玩家需要选择是否购买保险。\n";
        std::cout << "花费：一半下注筹码数\n";
        std::cout << "购买：庄家是黑杰克（A+10)，庄家只赢得保险金，否则收走保险金继续后面的步骤\n";
        std::cout << "不购买：庄家为黑杰克则没收全部下注\n";
        std::cout << "*****************************************************************\n";
        std::cout << "\n";
    }


    if (mSession == 0) {
        //1.下注环节Q A D Z C
        std::cout << "名称\t" << "数量\t" << "增加\t" << "减少\n";
        std::cout << "筹码\t"<< std::to_string(mCoin) <<"\tQ\tE\n";
        std::cout << "下注\t"<< std::to_string(mBet)<<"\tA\tD\n";
        std::cout << "基数\t"<< std::to_string(static_cast<int>(mBase))<<"\tZ\tC\n\n";

        std::cout << "开始游戏：H\n";
    }
    else if (mSession & 7) {
        //2.操作环节 A S D F
        std::cout << "筹码\t下注\n";
        std::cout << std::to_string(mCoin)<< "\t"<<std::to_string(mBet)<<"\n\n";

        std::cout << "牌面\n";

        //没有结束前庄家只显示一张牌
        if (mSession & 4) {
            std::cout << "庄家：" << cards_banker->GetCardsDisplayStr() << "\t点数：" << cards_banker->GetSumDisplayStr() << "\n";
        }
        else {
            std::cout << "庄家：" << cards_banker->GetBankerCardsDisplayStr() << "\t点数：" << cards_banker->GetBankerSumDisplayStr() << "\n";
        }
        
        std::cout << "闲家：\n";
        std::cout << "手牌：" << cards_player_1->GetCardsDisplayStr() << "\t点数：" << cards_player_1->GetSumDisplayStr();
        if (cards_player_1->isWin == 1) {
            std::cout << "\t胜利";
        }
        else if (cards_player_1->isWin == -1) {
            std::cout << "\t失败";
        }
        std::cout << "\n";

        if (cards_player_1->isSplitCards) {
            std::cout << "手牌：" << cards_player_2->GetCardsDisplayStr() << "\t点数：" << cards_player_2->GetSumDisplayStr();
            if (cards_player_2->isWin == 1) {
                std::cout << "\t胜利";
            }
            else if (cards_player_2->isWin == -1) {
                std::cout << "\t失败";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        
        if (isDebug) {
            std::cout << "牌堆：\n";
            for (int i = 0; i < 52; i++) {
                std::cout << Cards::GetCardName(cards[i]) << " ";
            }
            std::cout << "\n";
            
            std::cout << "当前牌堆：\n";
            for (int i = 0; i < cards_ptr; i++) {
                std::cout << Cards::GetCardName(cards[i]) << " ";
            }
            std::cout << "\n";

            std::cout << "庄家手牌：\n";
            for (int i = 0; i < cards_banker->num_cards; i++) {
                std::cout << Cards::GetCardName(cards_banker->cards[i]) << " ";
            }
            std::cout << "\n";

            std::cout << "闲家手牌：\n";
            for (int i = 0; i < cards_player_1->num_cards; i++) {
                std::cout << Cards::GetCardName(cards_player_1->cards[i]) << " ";
            }
            std::cout << "\n";
            std::cout << "\n";
        }

        if (mSession & 1) {
            std::cout << "庄家明牌为A，是否购买保险？\n";
            std::cout << "A：购买保险\n";
            std::cout << "D：不购买保险\n";
        }
        else if(mSession & 2){
            if (cards_current == cards_player_1) {
                std::cout << "正在操作手牌一\n";
            }else if (cards_current == cards_player_2) {
                std::cout << "正在操作手牌二\n";
            }
            std::cout << "A：要牌\n";
            std::cout << "S：停牌\n";

            if (!cards_player_1->isSplitCards) {
                std::cout << "D：加倍\n";
            }
            if (!cards_player_1->isSplitCards&& cards_player_1->isDouble) {
                std::cout << "F：分牌\n";
            }
        }
        else if (mSession & 4) {
            if (mWinCoin == 0) {
                std::cout << "平局\n";
            }
            else if (mWinCoin > 0) {
                std::cout << "你赢得了" << abs(mWinCoin) << "筹码\n";
            }
            else if (mWinCoin < 0) {
                std::cout << "你损失了" << abs(mWinCoin) << "筹码\n";
            }
            std::cout << "回到下注页面：H\n";
        }
    }
}

