#include "PrecompiledHeader.h"

#include "CardPrimitive.h"

#include "MTGDeck.h"
#include "Subtypes.h"
#include "Translate.h"

using std::string;

SUPPORT_OBJECT_ANALYTICS(CardPrimitive)

CardPrimitive::CardPrimitive()
{
    init();
}

CardPrimitive::CardPrimitive(CardPrimitive * source)
{
    for (map<int, int>::const_iterator it = source->basicAbilities.begin(); it != source->basicAbilities.end(); ++it)
        basicAbilities[it->first] = source->basicAbilities[it->first];

    for (size_t i = 0; i < source->types.size(); ++i)
        types.push_back(source->types[i]);
    for (int i = 0; i < Constants::MTG_NB_COLORS; ++i)
        colors[i] = source->colors[i];
    manaCost.copy(source->getManaCost());
    //reducedCost.copy(source->getReducedManaCost());
    //increasedCost.copy(source->getIncreasedManaCost());
    if(source->getManaCost()->alternative)
        manaCost.alternative->alternativeName = source->getManaCost()->alternative->alternativeName;

    text = source->text;
    setName(source->name);

    power = source->power;
    toughness = source->toughness;
    restriction = source->restriction;
    otherrestriction = source->otherrestriction;
    suspendedTime = source->suspendedTime;

    magicText = source->magicText;
    for (map<string, string>::const_iterator it = source->magicTexts.begin(); it != source->magicTexts.end(); ++it)
        magicTexts[it->first] = source->magicTexts[it->first];
    spellTargetType = source->spellTargetType;
    alias = source->alias;
}

CardPrimitive::~CardPrimitive()
{
}

int CardPrimitive::init()
{
    basicAbilities.clear();

    types.clear();

    for (int i = 0; i < Constants::MTG_NB_COLORS; ++i)
        colors[i] = 0;

    magicText = "";
    magicTexts.clear();
    spellTargetType = "";
    alias = 0;
    hasRestriction = false;
    return 1;
}

bool CardPrimitive::isCreature()
{
    return hasSubtype(Subtypes::TYPE_CREATURE);
}

bool CardPrimitive::isLand()
{
    return hasSubtype(Subtypes::TYPE_LAND);
}

bool CardPrimitive::isSpell()
{
    return (!isCreature() && !isLand());
}

void CardPrimitive::setRestrictions(string _restriction)
{
    restriction = _restriction;
}

void CardPrimitive::getRestrictions()
{
    restriction;
}

void CardPrimitive::setOtherRestrictions(string _restriction)
{
    otherrestriction = _restriction;
}

void CardPrimitive::getOtherRestrictions()
{
    otherrestriction;
}

void CardPrimitive::setColor(string _color, int removeAllOthers)
{
    if (_color.compare("blue") == 0)
        return setColor(Constants::MTG_COLOR_BLUE, removeAllOthers);
    if (_color.compare("red") == 0)
        return setColor(Constants::MTG_COLOR_RED, removeAllOthers);
    if (_color.compare("green") == 0)
        return setColor(Constants::MTG_COLOR_GREEN, removeAllOthers);
    if (_color.compare("black") == 0)
        return setColor(Constants::MTG_COLOR_BLACK, removeAllOthers);
    if (_color.compare("white") == 0)
        return setColor(Constants::MTG_COLOR_WHITE, removeAllOthers);
    if (_color.compare("artifact") == 0)
        return setColor(Constants::MTG_COLOR_ARTIFACT, removeAllOthers);
}

void CardPrimitive::setColor(int _color, int removeAllOthers)
{
    if (removeAllOthers)
        for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
            colors[i] = 0;
    colors[_color] = 1;
}

void CardPrimitive::removeColor(int _color)
{
    colors[_color] = 0;
}

int CardPrimitive::getColor()
{
    for (int i = 1; i < Constants::MTG_NB_COLORS; i++)
        if (colors[i])
            return i;
    return 0;
}

int CardPrimitive::hasColor(int color)
{
    return (colors[color]);
}

int CardPrimitive::countColors()
{
    int result = 0;
    for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
        if (hasColor(i))
            ++result;
    return result;
}

void CardPrimitive::setManaCost(string s)
{
    ManaCost::parseManaCost(s, &manaCost);
    for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; i++)
    {
        if (manaCost.hasColor(i))
        {
            setColor(i);
        }
    }

}

void CardPrimitive::setType(const string& _type_text)
{
    setSubtype(_type_text);
}

void CardPrimitive::addType(char * _type_text)
{
    setSubtype(_type_text);
}

void CardPrimitive::setSubtype(const string& value)
{
    int id = Subtypes::subtypesList->find(value);
    addType(id);
}

void CardPrimitive::addType(int id)
{
    types.push_back(id);
}

//TODO Definitely move some of these functions to CardInstance. There is no reason to remove a type from an CardPrimitive since they represent the Database
//Removes a type from the types of a given card
//If removeAll is true, removes all occurences of this type, otherwise only removes the first occurence
int CardPrimitive::removeType(string value, int removeAll)
{

    int id = Subtypes::subtypesList->find(value);
    return removeType(id, removeAll);
}

int CardPrimitive::removeType(int id, int removeAll)
{
    int result = 0;
    for (int i = types.size() - 1; i >= 0; i--)
    {
        if (types[i] == id)
        {
            types.erase(types.begin() + i);
            result++;
            if (!removeAll)
                return result;
        }
    }
    return result;
}

void CardPrimitive::setText(const string& value)
{
    text = value;
}

const string& CardPrimitive::getText()
{
    return text;
}

void CardPrimitive::addMagicText(string value)
{
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (magicText.size())
        magicText.append("\n");
    magicText.append(value);
}

void CardPrimitive::addMagicText(string value, string key)
{
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (magicTexts[key].size())
        magicTexts[key].append("\n");
    magicTexts[key].append(value);
}

void CardPrimitive::setName(const string& value)
{
    name = value;
    lcname = value;
    std::transform(lcname.begin(), lcname.end(), lcname.begin(), ::tolower);
    //This is a bug fix for plague rats and the "foreach ability"
    //Right now we add names as types, so that they get recognized
    if (lcname.at(value.length() - 1) == 's')
        Subtypes::subtypesList->find(lcname);
}

const string& CardPrimitive::getName() const
{
    return name;
}

const string& CardPrimitive::getLCName() const
{
    return lcname;
}

ManaCost* CardPrimitive::getManaCost()
{
    return &manaCost;
}

bool CardPrimitive::hasType(int _type)
{
    for (size_t i = 0; i < types.size(); i++)
        if (types[i] == _type)
            return true;
    return false;
}

bool CardPrimitive::hasSubtype(int _subtype)
{
    return hasType(_subtype);
}

bool CardPrimitive::hasType(const char * _type)
{
    int id = Subtypes::subtypesList->find(_type);
    return hasType(id);
}

bool CardPrimitive::hasSubtype(const char * _subtype)
{
    int id = Subtypes::subtypesList->find(_subtype);
    return hasType(id);
}

bool CardPrimitive::hasSubtype(string _subtype)
{
    int id = Subtypes::subtypesList->find(_subtype);
    return hasType(id);
}

int CardPrimitive::has(int basicAbility)
{
    return basicAbilities[basicAbility];
}

//---------------------------------------------
// Creature specific
//---------------------------------------------
void CardPrimitive::setPower(int _power)
{
    power = _power;
}

int CardPrimitive::getPower()
{
    return power;
}

void CardPrimitive::setToughness(int _toughness)
{
    toughness = _toughness;
}

int CardPrimitive::getToughness()
{
    return toughness;
}
