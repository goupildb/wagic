#include "PrecompiledHeader.h"

#include "ManaCost.h"
#include "ManaCostHybrid.h"
#include "ExtraCost.h"
#include "TargetChooser.h"
#include "Targetable.h"
#include "Player.h"
#include "WEvent.h"
#include "MTGAbility.h"

ManaCost * ManaCost::parseManaCost(string s, ManaCost * _manaCost, MTGCardInstance * c)
{
    ManaCost * manaCost;
    if (_manaCost)
    {
        manaCost = _manaCost;
    }
    else
    {
        manaCost = NEW ManaCost();
    }
    int state = 0;
    size_t start = 0;
    size_t end = 0;
    while (!s.empty() && state != -1)
    {
        switch (state)
        {
        case 0:
            start = s.find_first_of("{");
            if (start == string::npos)
            {
                return manaCost;
            }
            else
            {
                state = 1;
            }
            break;
        case 1:
            end = s.find_first_of("}");
            if (end == string::npos)
            {
                state = -1;
            }
            else
            {
                string value = s.substr(start + 1, end - 1 - start);

                if (value == "u")
                {
                    manaCost->add(Constants::MTG_COLOR_BLUE, 1);
                }
                else if (value == "b")
                {
                    manaCost->add(Constants::MTG_COLOR_BLACK, 1);
                }
                else if (value == "w")
                {
                    manaCost->add(Constants::MTG_COLOR_WHITE, 1);
                }
                else if (value == "g")
                {
                    manaCost->add(Constants::MTG_COLOR_GREEN, 1);
                }
                else if (value == "r")
                {
                    manaCost->add(Constants::MTG_COLOR_RED, 1);

                }
                else
                {

                    //Parse target for extraCosts
                    TargetChooserFactory tcf;
                    TargetChooser * tc = NULL;
                    size_t target_start = value.find("(");
                    size_t target_end = value.find(")");
                    if (target_start != string::npos && target_end != string::npos)
                    {
                        string target = value.substr(target_start + 1, target_end - 1 - target_start);
                        tc = tcf.createTargetChooser(target, c);
                    }

                    //switch on the first letter. If two costs share their first letter, add an "if" within the switch
                    switch (value[0])
                    {
                    case 'x':
                        manaCost->x();
                        break;
                    case 't': //Tap
                        if (value == "t")
                        {
                            //default Tap is handled outside of Manacost
                        }
                        else
                        {
                            manaCost->addExtraCost(NEW TapTargetCost(tc));
                        }
                        break;
                    case 's':
                        if (value == "s2l")
                        { //Send To Library Cost (move from anywhere to Library)
                            manaCost->addExtraCost(NEW ToLibraryCost(tc));
                        }
                        else
                        { //Sacrifice
                            manaCost->addExtraCost(NEW SacrificeCost(tc));
                        }
                        break;
                    case 'e': //Exile
                        manaCost->addExtraCost(NEW ExileTargetCost(tc));
                        break;
                    case 'h': //bounce (move to Hand)
                        manaCost->addExtraCost(NEW BounceTargetCost(tc));
                        break;
                    case 'l':
                        if (value == "l2e")
                        { //Mill to exile yourself as a cost (Library 2 Exile)
                            manaCost->addExtraCost(NEW MillExileCost(tc));
                        }
                        else
                        { //Life cost
                            manaCost->addExtraCost(NEW LifeCost(tc));
                        }
                        break;
                    case 'd': //DiscardRandom cost
                        if (value == "d")
                        {
                            manaCost->addExtraCost(NEW DiscardRandomCost(tc));
                        }
                        else
                        {
                            manaCost->addExtraCost(NEW DiscardCost(tc));
                        }
                        break;
                    case 'm': //Mill yourself as a cost
                        manaCost->addExtraCost(NEW MillCost(tc));
                        break;
                    case 'n': //return unblocked attacker cost
                        TargetChooserFactory tcf;
                        tc = tcf.createTargetChooser("creature|myBattlefield", c);
                        manaCost->addExtraCost(NEW Ninja(tc));
                        break;
                    case 'c': //Counters
                    {
                        size_t counter_start = value.find("(");
                        size_t counter_end = value.find(")", counter_start);
                        AbilityFactory abf;
                        string counterString = value.substr(counter_start + 1, counter_end - counter_start - 1);
                        Counter * counter = abf.parseCounter(counterString, c);
                        size_t separator = value.find(",", counter_start);
                        size_t separator2 = string::npos;
                        if (separator != string::npos)
                        {
                            separator2 = value.find(",", separator + 1);
                        }
                        SAFE_DELETE(tc);
                        size_t target_start = string::npos;
                        if (separator2 != string::npos)
                        {
                            target_start = value.find(",", separator2 + 1);
                        }
                        size_t target_end = counter_end;
                        if (target_start != string::npos && target_end != string::npos)
                        {
                            string target = value.substr(target_start + 1, target_end - 1 - target_start);
                            tc = tcf.createTargetChooser(target, c);
                        }
                        manaCost->addExtraCost(NEW CounterCost(counter, tc));
                        break;
                    }
                    default: //uncolored cost and hybrid costs
                    {
                        int intvalue = atoi(value.c_str());
                        int colors[2];
                        int values[2];
                        if (intvalue < 10 && value.size() > 1)
                        {
                            for (int i = 0; i < 2; i++)
                            {
                                char c = value[i];
                                if (c >= '0' && c <= '9')
                                {
                                    colors[i] = Constants::MTG_COLOR_ARTIFACT;
                                    values[i] = c - '0';
                                }
                                else
                                {
                                    for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
                                    {
                                        if (c == Constants::MTGColorChars[j])
                                        {
                                            colors[i] = j;
                                            values[i] = 1;
                                        }
                                    }
                                }
                            }
                            manaCost->addHybrid(colors[0], values[0], colors[1], values[1]);
                        }
                        else
                        {
                            manaCost->add(Constants::MTG_COLOR_ARTIFACT, intvalue);
                        }
                        break;
                    }
                    }
                }
                s = s.substr(end + 1);
                state = 0;
            }
            break;
        default:
            break;
        }
    }
    return manaCost;
}

ManaCost::ManaCost()
{
    init();
}

ManaCost::ManaCost(int _cost[], int nb_elems)
{
    init();
    for (int i = 0; i < nb_elems; i++)
    {
        cost[_cost[i * 2]] = _cost[i * 2 + 1];
    }

}

ManaCost::ManaCost(ManaCost * _manaCost)
{
    init();
    for (int i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = _manaCost->getCost(i);
    }
}

ManaCost::~ManaCost()
{
    for (unsigned int i = 0; i < nbhybrids; i++)
    {
        SAFE_DELETE(hybrids[i]);
    }
    SAFE_DELETE(extraCosts);
    SAFE_DELETE(kicker);
    SAFE_DELETE(alternative);
    SAFE_DELETE(BuyBack);
    SAFE_DELETE(FlashBack);
    SAFE_DELETE(Retrace);
    SAFE_DELETE(morph);
}

void ManaCost::x()
{
    cost[Constants::MTG_NB_COLORS] = 1;
}

int ManaCost::hasX()
{
    return cost[Constants::MTG_NB_COLORS];
}

void ManaCost::init()
{
    int i;
    for (i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = 0;
    }
    nbhybrids = 0;
    extraCosts = NULL;
    extraCostsIsCopy = 0;
    kicker = NULL;
    alternative = NULL;
    BuyBack = NULL;
    FlashBack = NULL;
    Retrace = NULL;
    morph = NULL;
    
    // why is hybrids hardcoded to 10?
    for (i = 0; i < 10; i++)
        hybrids[i] = NULL;
}

void ManaCost::copy(ManaCost * _manaCost)
{
    if (!_manaCost)
        return;
    for (unsigned int i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = _manaCost->getCost(i);
    }
    for (unsigned int i = 0; i < nbhybrids; i++)
    {
        SAFE_DELETE(hybrids[i]);
    }
    for (unsigned int i = 0; i < _manaCost->nbhybrids; i++)
    {
        hybrids[i] = NEW ManaCostHybrid((*_manaCost->hybrids[i]));
    }
    nbhybrids = _manaCost->nbhybrids;

    SAFE_DELETE(extraCosts);
    if (_manaCost->extraCosts)
    {
        extraCosts = _manaCost->extraCosts->clone();
    }

    SAFE_DELETE(kicker);
    if (_manaCost->kicker)
    {
        kicker = NEW ManaCost();
        kicker->copy(_manaCost->kicker);
    }
    SAFE_DELETE(alternative);
    if (_manaCost->alternative)
    {
        alternative = NEW ManaCost();
        alternative->copy(_manaCost->alternative);
    }
    SAFE_DELETE(BuyBack);
    if (_manaCost->BuyBack)
    {
        BuyBack = NEW ManaCost();
        BuyBack->copy(_manaCost->BuyBack);
    }
    SAFE_DELETE(FlashBack);
    if (_manaCost->FlashBack)
    {
        FlashBack = NEW ManaCost();
        FlashBack->copy(_manaCost->FlashBack);
    }
    SAFE_DELETE(Retrace);
    if (_manaCost->Retrace)
    {
        Retrace = NEW ManaCost();
        Retrace->copy(_manaCost->Retrace);
    }
    SAFE_DELETE(morph);
    if (_manaCost->morph)
    {
        morph = NEW ManaCost();
        morph->copy(_manaCost->morph);
    }
}

int ManaCost::getCost(int color)
{
    return cost[color];
}

ManaCostHybrid * ManaCost::getHybridCost(unsigned int i)
{
    if (nbhybrids <= i)
        return NULL;
    return hybrids[i];
}

int ManaCost::hasColor(int color)
{
    if (cost[color])
        return 1;
    for (unsigned int i = 0; i < nbhybrids; i++)
    {
        if (hybrids[i]->hasColor(color))
            return 1;
    }
    return 0;
}

int ManaCost::isNull()
{
    if (getConvertedCost())
        return 0;
    if (extraCosts)
        return 0;
    return 1;
}

int ManaCost::getConvertedCost()
{
    int result = 0;
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        result += cost[i];
    }
    for (unsigned int i = 0; i < nbhybrids; i++)
    {
        result += hybrids[i]->getConvertedCost();
    }
    return result;
}

int ManaCost::remove(int color, int value)
{
    cost[color] -= value;
    if (cost[color] < 0)
    {
        cost[color] = 0;
    }
    return 1;
}

int ManaCost::add(int color, int value)
{
    if (value < 0)
        value = 0;
    cost[color] += value;
    return 1;
}

int ManaCost::add(ManaCost * _cost)
{
    if (!_cost)
        return 0;
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        cost[i] += _cost->getCost(i);
    }
    for (unsigned int i = 0; i < _cost->nbhybrids; i++)
    {
        hybrids[i] = NEW ManaCostHybrid((*_cost->hybrids[i]));
    }
    nbhybrids = _cost->nbhybrids;
    return 1;
}

int ManaCost::addHybrid(int c1, int v1, int c2, int v2)
{
    ManaCostHybrid * h = NEW ManaCostHybrid(c1, v1, c2, v2);
    hybrids[nbhybrids] = h;
    nbhybrids++;
    return nbhybrids;
}

int ManaCost::addExtraCost(ExtraCost * _cost)
{
    if (!extraCosts)
        extraCosts = NEW ExtraCosts();
    extraCosts->costs.push_back(_cost);
    return 1;
}

int ManaCost::isExtraPaymentSet()
{
    if (!extraCosts)
        return 1;
    return extraCosts->isPaymentSet();
}

int ManaCost::canPayExtra()
{
    if (!extraCosts)
        return 1;
    return extraCosts->canPay();
}

int ManaCost::doPayExtra()
{
    if (!extraCosts)
        return 0;
    return extraCosts->doPay(); //TODO reset ?
}

int ManaCost::setExtraCostsAction(MTGAbility * action, MTGCardInstance * card)
{
    if (extraCosts)
        extraCosts->setAction(action, card);
    return 1;
}

int ManaCost::pay(ManaCost * _cost)
{
    int result = MANA_PAID;
    ManaCost * toPay = NEW ManaCost();
    toPay->copy(_cost);

    if (toPay->kicker)
    {
        toPay->add(toPay->kicker);
        if (!canAfford(toPay))
        {
            toPay->copy(_cost);
        }
        else
        {
            result = MANA_PAID_WITH_KICKER;
        }
    }

    ManaCost * diff = Diff(toPay);
    for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = diff->getCost(i);
    }
    delete diff;
    delete toPay;
    return result;
    //TODO return 0 if can't afford the cost!
}

//return 1 if _cost can be paid with current data, 0 otherwise
int ManaCost::canAfford(ManaCost * _cost)
{
    ManaCost * diff = Diff(_cost);
    int positive = diff->isPositive();
    delete diff;
    if (positive)
    {
        return 1;
    }
    return 0;
}

int ManaCost::isPositive()
{
    for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {

        if (cost[i] < 0)
        {
            return 0;
        }
    }
    return 1;

}

void ManaCost::randomDiffHybrids(ManaCost * _cost, int diff[])
{
    int _nbhybrids = _cost->nbhybrids;
    for (int i = 0; i < _nbhybrids; i++)
    {
        ManaCostHybrid * h = _cost->hybrids[i];
        diff[h->color1 * 2 + 1] -= h->value1;
    }
}

int ManaCost::tryToPayHybrids(ManaCostHybrid * _hybrids[], int _nbhybrids, int diff[])
{
    if (!_nbhybrids)
        return 1;
    int result = 0;
    ManaCostHybrid * h = _hybrids[_nbhybrids - 1];
    if (diff[h->color1 * 2 + 1] >= h->value1)
    {
        diff[h->color1 * 2 + 1] -= h->value1;
        result = tryToPayHybrids(_hybrids, _nbhybrids - 1, diff);
        if (result)
            return 1;
        diff[h->color1 * 2 + 1] += h->value1;
    }
    if (diff[h->color2 * 2 + 1] >= h->value2)
    {
        diff[h->color2 * 2 + 1] -= h->value2;
        result = tryToPayHybrids(_hybrids, _nbhybrids - 1, diff);
        if (result)
            return 1;
        diff[h->color2 * 2 + 1] += h->value2;
    }
    return 0;
}

//compute the difference between two mana costs
ManaCost * ManaCost::Diff(ManaCost * _cost)
{
    int diff[(Constants::MTG_NB_COLORS + 1) * 2];
    diff[Constants::MTG_NB_COLORS * 2] = Constants::MTG_NB_COLORS;
    for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        diff[i * 2] = i;
        diff[i * 2 + 1] = cost[i] - _cost->getCost(i);
    }
    int hybridResult = tryToPayHybrids(_cost->hybrids, _cost->nbhybrids, diff);
    if (!hybridResult)
        randomDiffHybrids(_cost, diff);

    //Colorless mana, special case
    int colorless_idx = Constants::MTG_COLOR_ARTIFACT * 2 + 1;
    if (diff[colorless_idx] < 0)
    {
        for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
        {
            if (diff[i * 2 + 1] > 0)
            {
                if (diff[i * 2 + 1] + diff[colorless_idx] > 0)
                {
                    diff[i * 2 + 1] += diff[colorless_idx];
                    diff[colorless_idx] = 0;
                    break;
                }
                else
                {
                    diff[colorless_idx] += diff[i * 2 + 1];
                    diff[i * 2 + 1] = 0;
                }
            }
        }
    }

    //Cost X
    if (_cost->hasX())
    {
        diff[Constants::MTG_NB_COLORS * 2 + 1] = 0;
        for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
        {
            if (diff[i * 2 + 1] > 0)
            {
                diff[Constants::MTG_NB_COLORS * 2 + 1] += diff[i * 2 + 1];
                diff[i * 2 + 1] = 0;
            }
        }
    }

    ManaCost * result = NEW ManaCost(diff, Constants::MTG_NB_COLORS + 1);
    return result;

}

string ManaCost::toString()
{
    ostringstream oss;
    for (int i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        if (cost[i])
        {
            if ( i == Constants::MTG_COLOR_ARTIFACT)
                oss << "{" << cost[i] << "}";
            else
                for (int colorCount = 0; colorCount < cost[i]; colorCount++ )
                    oss << "{" << Constants::MTGColorChars[i] << "}";
        }
    }

    for (unsigned int i = 0; i < nbhybrids; i++)
    {
        if ( hybrids[i] != NULL )
            oss << hybrids[i];
    }
    return oss.str();
}

#ifdef WIN32
void ManaCost::Dump()
{
    DebugTrace( "\n===ManaCost===" );
    DebugTrace( this->toString() );
    DebugTrace( "\n=============" );
}

#endif

ostream& operator<<(ostream& out, ManaCost& m)
{
    return out << m.toString();
}

ostream& operator<<(ostream& out, ManaCost* m)
{
    return out << m->toString();
}

ostream& operator<<(ostream& out, ManaCost m)
{
    return out << m.toString();
}

void ManaPool::init()
{
    ManaCost::init();
    WEvent * e = NEW WEventEmptyManaPool(this);
    GameObserver::GetInstance()->receiveEvent(e);
}

ManaPool::ManaPool(Player * player) :
    ManaCost(), player(player)
{
}

ManaPool::ManaPool(ManaCost * _manaCost, Player * player) :
    ManaCost(_manaCost), player(player)
{
}

int ManaPool::remove(int color, int value)
{
    int result = ManaCost::remove(color, value);
    for (int i = 0; i < value; ++i)
    {
        WEvent * e = NEW WEventConsumeMana(color, this);
        GameObserver::GetInstance()->receiveEvent(e);
    }
    return result;
}

int ManaPool::add(int color, int value, MTGCardInstance * source)
{
    int result = ManaCost::add(color, value);
    for (int i = 0; i < value; ++i)
    {
        WEvent * e = NEW WEventEngageMana(color, source, this);
        GameObserver::GetInstance()->receiveEvent(e);
    }
    return result;
}

int ManaPool::add(ManaCost * _cost, MTGCardInstance * source)
{
    if (!_cost)
        return 0;
    int result = ManaCost::add(_cost);
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        for (int j = 0; j < _cost->getCost(i); j++)
        {
            WEvent * e = NEW WEventEngageMana(i, source, this);
            GameObserver::GetInstance()->receiveEvent(e);
        }
    }
    return result;
}

int ManaPool::pay(ManaCost * _cost)
{
    int current[Constants::MTG_NB_COLORS];
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        current[i] = cost[i];
    }

    int result = ManaCost::pay(_cost);
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        int value = current[i] - cost[i];
        for (int j = 0; j < value; j++)
        {
            WEvent * e = NEW WEventConsumeMana(i, this);
            GameObserver::GetInstance()->receiveEvent(e);

        }
    }
    return result;
}