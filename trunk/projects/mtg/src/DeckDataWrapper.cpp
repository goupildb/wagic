#include "../include/config.h"
#include "../include/DeckDataWrapper.h"
#include "../include/MTGDeck.h"
#include "../include/PriceList.h"

DeckDataWrapper::DeckDataWrapper(MTGDeck * deck){
  parent = deck;
  for (int i = 0; i <= Constants::MTG_NB_COLORS; i++){
    colors[i] = 0;
  }
  for (int i = 0; i < deck->totalCards(); i++){
    MTGCard * card = deck->_(i);
    Add(card);

  }
  currentposition = 0;
  currentColor = -1;
}

void DeckDataWrapper::save(){
  parent->removeAll();
  map<MTGCard *,int,Cmp1>::iterator it;
  for ( it=cards.begin() ; it != cards.end(); it++ ){
    MTGCard * current = (*it).first;
    for (int i = 0; i < (*it).second; i++){
      parent->add(current);
    }
  }
  parent->save();
}


DeckDataWrapper::~DeckDataWrapper(){
  SAFE_DELETE(parent);
}

void DeckDataWrapper::updateCounts(MTGCard * card, int removed){
  if (!card){
    for (int i = 0; i < Constants::MTG_NB_COLORS+1; i++){
      colors[i] = 0;
    }
    map<MTGCard *,int,Cmp1>::iterator it;
    for ( it=cards.begin() ; it != cards.end(); it++ ){
      MTGCard * current = (*it).first;
      colors[Constants::MTG_NB_COLORS] += (*it).second;
      for (int i = 0; i < Constants::MTG_NB_COLORS; i++){
	if (current->hasColor(i)) colors[i]+=(*it).second;
      }
    }
  }else{
    int increment = 1;
    if (removed) increment = -1;
    colors[Constants::MTG_NB_COLORS] += increment;
    for (int i = 0; i < Constants::MTG_NB_COLORS; i++){
      if (card->hasColor(i)) colors[i]+=increment;
    }
  }
}

int DeckDataWrapper::Add(MTGCard * card){
  if(cards.find(card) == cards.end()){
    cards[card] = 1;
  }else{
    cards[card]++;
  }
  updateCounts(card);
  return cards[card];
}

int DeckDataWrapper::Remove(MTGCard * card){
  if(cards.find(card) == cards.end() || cards[card] == 0) return 0;
  cards[card]--;
  updateCounts(card,1);
  return 1;
}

int DeckDataWrapper::count(MTGCard * card){
  if(cards.find(card) == cards.end()){
    cards[card] = 0;
  }
  return cards[card];
}

int DeckDataWrapper::countByName(MTGCard * card){
  string name = card->name;
  int total = 0;
  map<MTGCard *,int,Cmp1>::iterator it;
  it = cards.find(card);
  if(cards.find(card) == cards.end()){
    cards[card] = 0;
    it = cards.find(card);
  }

   while(it !=cards.end()){
     MTGCard * _card = (*it).first;
     if (name.compare(_card->name) !=0){
       it = cards.end();
     }else{
      total+= (*it).second;
      it++;
     }
   }

   it = cards.find(card);
   it--;
   while(1){
     MTGCard * _card = (*it).first;
     if (name.compare(_card->name) !=0){
       break;
     }else{
      total+= (*it).second;
      if (it == cards.begin()) break;
      it--;
      
     }
   }
   return total;
}


MTGCard * DeckDataWrapper::getNext(MTGCard * previous, int color){
  map<MTGCard *,int,Cmp1>::iterator it;

  it = cards.find(previous);

  while(1){
    if (it == cards.end()){
      it = cards.begin();
    }else{
      it++;
    }
    if (it == cards.end()) return NULL;
    MTGCard * card = (*it).first;
    if (card == previous) return NULL;
    if ((*it).second >0 && (color ==-1 || card->hasColor(color))){
      return card;
    }
  }
}

MTGCard * DeckDataWrapper::getPrevious(MTGCard * next, int color){
  map<MTGCard *,int,Cmp1>::iterator it;
  it = cards.find(next);

  while(1){
    if (it == cards.begin()){
      it = cards.end();
    }else{
      it--;
    }
    if (it == cards.end()) return NULL;
    MTGCard * card = (*it).first;
    if (card == next) return NULL;
    if ((*it).second >0 && (color ==-1 || card->hasColor(color))){
      return card;
    }
  }
}

void DeckDataWrapper::updateCurrentPosition(MTGCard * currentCard, int color){
  currentposition = 0;
  MTGCard * next = getNext(NULL,color);
  while (next){
    currentposition+=cards[next];
    if (next == currentCard){
      next = NULL;
    }else{
      next = getNext(next,color);
    }
  }
}

int DeckDataWrapper::getCount(int color){
  if (color == -1) return colors[Constants::MTG_NB_COLORS];
  return colors[color];
}

int DeckDataWrapper::totalPrice(){
  int total = 0;
  PriceList * pricelist = NEW PriceList(RESPATH"/settings/prices.dat",this->parent);
  map<MTGCard *,int,Cmp1>::iterator it;
  for ( it=cards.begin() ; it != cards.end(); it++ ){
      MTGCard * current = (*it).first;
      int nb =  (*it).second;
      if (nb) total += pricelist->getPrice(current->getMTGId());
  }
  delete pricelist;
  return total;
}