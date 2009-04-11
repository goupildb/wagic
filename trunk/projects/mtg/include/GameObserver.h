#ifndef _GAMEOBSERVER_H_
#define _GAMEOBSERVER_H_

#include "Player.h"
#include "MTGAbility.h"
#include "DuelLayers.h"
#include "MTGCardInstance.h"
#include "PlayGuiObject.h"
#include "DuelLayers.h"
#include "TargetChooser.h"
#include "PhaseRing.h"

#define MAX_GAME_OBSERVERS 500


class MTGGamePhase;
class MTGAbility;
class MTGCardInstance;
class CardGui;
class Player;
class TargetChooser;


class GameObserver{
 protected:
  int reaction;
  static GameObserver * mInstance;
  MTGCardInstance * cardWaitingForTargets;

  int nbPlayers;
  int currentPlayerId;
  int currentRound;


  
 public:
  int turn;
  int targetListIsSet(MTGCardInstance * card);
  PhaseRing * phaseRing;
  int cancelCurrentAction();
  int currentGamePhase;
  ExtraCosts * waitForExtraPayment;
  int oldGamePhase;
  TargetChooser * targetChooser;
  DuelLayers * mLayers;
  Player * gameOver;
  Player * players[2]; //created outside
  MTGGamePhase * gamePhaseManager; //Created Outside ?

  TargetChooser * getCurrentTargetChooser();
  void stackObjectClicked(Interruptible * action);

  void cardClick(MTGCardInstance * card,Targetable * _object = NULL );
  int enteringPhase(int phase);
  int getCurrentGamePhase();
  void userRequestNextGamePhase();
  void nextGamePhase();
  void cleanupPhase();
  void nextPlayer();
  static void Init(Player * _players[], int _nbplayers);
  static GameObserver * GetInstance();
  static void EndInstance();
  Player * currentPlayer;
  Player * currentActionPlayer;
  Player * isInterrupting;
  Player * opponent();
  Player * currentlyActing();
  GameObserver(Player * _players[], int _nbplayers);
  ~GameObserver();
  void setGamePhaseManager(MTGGamePhase * _phases);
  void stateEffects();
  void eventOccured();
  void addObserver(MTGAbility * observer);
  void removeObserver(ActionElement * observer);
  void startGame(int shuffle = 1, int draw = 1);
  void untapPhase();
  void draw();
  int isInPlay(MTGCardInstance *  card);
  int isACreature(MTGCardInstance *  card);
  int tryToTapOrUntap(MTGCardInstance * card);

  int checkManaCost(MTGCardInstance * card);

  void Update(float dt);
  void Render();
  void ButtonPressed(int, PlayGuiObject*);


};

#endif
