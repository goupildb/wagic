/*
  Filter-like system for determining if a card meats certain criteria, for this and thisforeach autos
*/

#ifndef _THISDESCRIPTOR_H_
#define _THISDESCRIPTOR_H_

#include "Counters.h"
#include "MTGGameZones.h"
#include "MTGCardInstance.h"
#include "CardDescriptor.h"

class ThisDescriptor{
 public:
   int comparisonMode;
   int comparisonCriterion;
   virtual int match(MTGCardInstance * card) = 0;
   int matchValue(int value);  
   virtual ~ThisDescriptor();
};

class ThisDescriptorFactory{
public:
  ThisDescriptor * createThisDescriptor(string s);
};

class ThisCounter:public ThisDescriptor{
 public:
  Counter * counter;
  virtual int match(MTGCardInstance * card);

  ThisCounter(Counter * _counter);
  ThisCounter(int power, int toughness, int nb, const char * name);
  ~ThisCounter();
};

class ThisCounterAny:public ThisDescriptor{
 public:
  virtual int match(MTGCardInstance *card);

  ThisCounterAny(int nb);
};

class ThisControllerlife:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisControllerlife(int life);
};

class ThisOpponentlife:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisOpponentlife(int olife);
};

class ThisEquip:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisEquip(int equipment);
};

class ThisAuras:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisAuras(int auras);
};

class ThisOpponentDamageAmount:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisOpponentDamageAmount(int damagecount);
};

class ThisUntapped:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisUntapped(int untapped);
};

class ThisTapped:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisTapped(int tapped);
};


class ThisAttacked:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);

	ThisAttacked(int attack);
};

class ThisNotBlocked:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);

	ThisNotBlocked(int unblocked);
};

class ThisDamaged:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);

	ThisDamaged(int wasDealtDamage);
};

class ThisPower:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisPower(int power);
};

class ThisToughness:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisToughness(int toughness);
};

class ThisX:public ThisDescriptor{
  public:
    virtual int match(MTGCardInstance * card);
    ThisX(int x);
};

#endif