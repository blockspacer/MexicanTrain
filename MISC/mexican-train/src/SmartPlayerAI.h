#pragma once

#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include "Common.h"
#include "EnemyPlayer.h"
#include "RandomPlayerAI.h"

class Board;
class Player;
class Train;


class SmartPlayerAI : public RandomPlayerAI {
public:
  SmartPlayerAI(const Player& player, const std::vector<EnemyPlayer>& enemyPlayers, const Board& board, const GameSettings& gameSettings, std::ostream* out = nullptr);
  virtual ~SmartPlayerAI();
  virtual TilePlay playTile() override;
  virtual void notifyRoundStart();
  virtual void notifyTilePlay(id playerId, id placeId, id tileId) override;
  virtual void notifyTileDraw(id playerId) override;
  virtual void notifyPassTurn(id playerId) override;
  virtual void notifyGameResult(int32 placeFinished) override;
  virtual void message(const std::string& msg) override;

private:
  std::vector<id> m_plannedTrain;
  std::vector<id> m_spareTiles;
  const Train& m_playerTrain;
  std::vector<const Train*> m_otherTrains;
  bool m_reevaluatePlays = true;
  bool m_tryingLockPlay = false;
  bool m_tryingSparePlay = false;
  bool m_tryingTrainPlay = false;
  bool m_fallbackRandomPlay = false;
  uint32 m_spareTrainIndex;
  uint32 m_spareTileIndex;
};
