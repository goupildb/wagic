#include "../include/DeckDataWrapper.h"
#include "../include/MTGDeck.h"

DeckDataWrapper::DeckDataWrapper(MTGDeck * deck){
  parent = deck;
  for (int i = 0; i <= MTG_NB_COLORS; i++){
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
    for (int i = 0; i < MTG_NB_COLORS+1; i++){
      colors[i] = 0;
    }
    map<MTGCard *,int,Cmp1>::iterator it;
    for ( it=cards.begin() ; it != cards.end(); it++ ){
      MTGCard * current = (*it).first;
      colors[MTG_NB_COLORS] += (*it).second;
      for (int i = 0; i < MTG_NB_COLORS; i++){
	if (current->hasColor(i)) colors[i]+=(*it).second;
      }
    }
  }else{
    int increment = 1;
    if (removed) increment = -1;
    colors[MTG_NB_COLORS] += increment;
    for (int i = 0; i < MTG_NB_COLORS; i++){
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

MTGCard * DeckDataWrapper::getNext(MTGCard * previous, int color){
  map<MTGCard *,int,Cmp1>::iterator it;

  it = cards.find(previous);
  int found = 0;

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
  int found = 0;

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
  if (color == -1) return colors[MTG_NB_COLORS];
  return colors[color];
}
