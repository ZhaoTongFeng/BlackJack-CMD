#include "Cards.h"

Cards::Cards()
{
    ReSet();
}

void Cards::ReSet()
{
    num_cards = 0;
    sum_min = 0;
    sum_max = 0;
    sum = 0;
    isSplitCards = false;
    isOutRange = false;
    is21 = false;
    isDouble = false;
    bet = 0;
    isFinish = false;
    isWin = 0;
}

void Cards::ComputeAttribute()
{
    sum_min = 0;
    sum_max = 0;
    sum = 0;
    for (int i = 0; i < num_cards; i++) {
        if (cards[i] % 13 == 1) {
            //A
            sum_min += 1;
            sum_max += 11;
        }
        else {
            if (cards[i] % 13 == 0) {
                sum_min += 13;
                sum_max += 13;
            }
            else {
                sum_min += cards[i] % 13;
                sum_max += cards[i] % 13;
            }
        }
    }
    sum = sum_max <= 21 ? sum_max : sum_min;

    is21 = sum == 21;

    isOutRange = sum > 21;
    isDouble = false;
    if (num_cards == 2) {
        if (cards[0] % 13 == cards[1] % 13) {
            isDouble = true;
        }
    }
}

void Cards::AddCard(int num)
{
    cards[num_cards++] = num;

    ComputeAttribute();
}

void Cards::SplitCard(Cards* c, int n1, int n2)
{
    isSplitCards = true;
    c->isSplitCards = true;
    c->AddCard(cards[--num_cards]);
    c->AddCard(n2);
    AddCard(n1);
    ComputeAttribute();
}

void Cards::Compare(Cards* banker)
{
    //玩家手牌有没有爆牌
    if (isOutRange) {
        isWin = -1;
    }
    //庄家是否爆牌
    else if (banker->isOutRange) {
        isWin = 1;
    }
    //再看是不是平局
    else if (banker->sum == sum) {
        isWin = 2;
    }
    //庄家是不是黑杰克
    else if (banker->IsBlackJack()) {
        isWin = -1;
    }
    //玩家是不是黑杰克
    else if (IsBlackJack() && !isSplitCards) {
        isWin = 1;
        //获得1.5倍下注
        bet = static_cast<int>(floor(bet * 1.5f));
    }
    //都不是就比较分数
    else if (banker->sum > sum) {
        isWin = -1;
    }
    else if (banker->sum < sum) {
        isWin = 1;
    }
}

bool Cards::IsFirA()
{
    return cards[0] % 13 == 1;
}

bool Cards::IsBlackJack()
{
    if (cards[0] % 13 == 1 && (cards[1] % 13 >= 10 || cards[1] % 13 >= 0)) {
        return true;
    }
    if (cards[1] % 13 == 1 && (cards[0] % 13 >= 10 || cards[0] % 13 >= 0)) {
        return true;
    }
    return false;
}

std::string Cards::GetCardsDisplayStr()
{
    std::string str = "";
    for (int i = 0; i < num_cards; i++) {
        str += GetCardName(cards[i]);
        if (i != num_cards - 1) {
            str += ",";
        }
    }
    return str;
}

std::string Cards::GetBankerCardsDisplayStr()
{
    return GetCardName(cards[0]);
}

std::string Cards::GetSumDisplayStr()
{
    return sum_min == sum_max ? std::to_string(sum) : std::to_string(sum_min) + "/" + std::to_string(sum_max);
}

std::string Cards::GetBankerSumDisplayStr()
{
    int temp = cards[0] % 13;
    std::string str = "";
    if (temp == 0) {
        str = "13";
    }
    else if (temp == 1) {
        str = "11";
    }
    else {
        str = std::to_string(temp);
    }
    return str;
}

std::string Cards::GetCardName(int num)
{
    std::string str;
    int temp = num % 13;

    if (temp == 11) {
        str = "J";
    }
    else if (temp == 12) {
        str = "Q";
    }
    else if (temp == 0) {
        str = "K";
    }
    else if (temp == 1) {
        str = "A";
    }
    else {
        str = std::to_string(temp);
    }
    return str;
}

