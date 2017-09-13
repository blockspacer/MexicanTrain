#include "GameAdmin.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include "Board.h"
#include "Game.h"
#include "PlayerAI.h"

using namespace std;


GameAdmin::GameAdmin() {
}

void GameAdmin::runGame(
  const GameSettings& gameSettings,
  std::function<std::vector<std::unique_ptr<PlayerAI>>(
    const std::vector<Player>&,
    const std::vector<std::vector<EnemyPlayer>>&,
    const Board&
  )> aiCreator
) {
  cout << "Number of players: " << gameSettings.m_numberOfPlayers << endl;
  cout << "Max domino pip count: " << gameSettings.m_maxPips << endl;
  cout << "Starting hand size: " << gameSettings.m_startingHandSize << endl;

  if (gameSettings.m_numberOfPlayers < 1 || gameSettings.m_numberOfPlayers > 8) {
    cerr << "There must be 1 - 8 players." << endl;
    return;
  }
  if (gameSettings.m_maxPips < 0) {
    cerr << "Max pip must be non-negative." << endl;
    return;
  }
  if (gameSettings.m_startingHandSize < 0 || gameSettings.m_startingHandSize > 25) {
    cerr << "Starting hand size must be between 0 and 25." << endl;
    return;
  }

  std::vector<Player> players(gameSettings.m_numberOfPlayers);
  std::vector<std::vector<EnemyPlayer>> enemyPlayerLists(gameSettings.m_numberOfPlayers);
  for (int32 i = 0; i < (int32) players.size(); i++) {
    for (int32 offset = 1; offset < (int32) players.size(); offset++) {
      enemyPlayerLists[i].push_back(players[(i + offset) % players.size()].m_enemyView);
    }
  }
  Board board(players);

  std::vector<std::unique_ptr<PlayerAI>> playerAis(aiCreator(players, enemyPlayerLists, board));
  assert((int32) playerAis.size() == gameSettings.m_numberOfPlayers);
  for (int32 i = 0; i < (int32) players.size(); i++) {
    players[i].m_ai = playerAis[i].get();
    assert(&playerAis[i]->m_player == &players[i]);
  }

  for (auto& ai : playerAis) {
    ai->notifyGameStart();
  }

  std::vector<int32> incompleteRounds;
  for (int32 i = 0; i <= gameSettings.m_maxPips; i++) {
    incompleteRounds.push_back(i);
  }
  reverse(incompleteRounds.begin(), incompleteRounds.end());

  while (incompleteRounds.size() > 0) {
    board.newRound(gameSettings.m_maxPips);
    for (auto& player : players) {
      player.newRound();
    }

    for (auto& ai : playerAis) {
      ai->notifyRoundStart();
    }

    for (int32 i = 0; i < gameSettings.m_startingHandSize; i++) {
      for (auto& player : players) {
        Tile tile = board.dealTile();
        player.m_hand.push_back(tile);
        for (auto& ai : playerAis) {
          ai->notifyTileDraw(player.m_id);
        }
      }
    }

    int32 playerTurn = -1;
    int32 lastActionTurn = -1;
    bool activeDoubles = false;
    id activeDoublesTrainId = NULL_ID;
    Game game(gameSettings, board, players);

    playerTurn = game.playCenterTile(&incompleteRounds);
    lastActionTurn = playerTurn;

    bool roundOver = false;
    while (!roundOver) {
      Player& currPlayer = players[playerTurn];
      Train& currPlayerTrain = board.m_playerTrains[currPlayer.m_id];
      auto drawOrPass = [&] () {
        currPlayerTrain.m_isPublic = true;
        if (board.poolSize() > 0) {
          Tile tile = board.dealTile();
          currPlayer.m_hand.push_back(tile);
          for (auto& ai : playerAis) {
            ai->notifyTileDraw(currPlayer.m_id);
          }
          lastActionTurn = playerTurn;
          playerTurn = (playerTurn + 1) % players.size();
        } else if (lastActionTurn == playerTurn) {
          roundOver = true;
        } else {
          playerTurn = (playerTurn + 1) % players.size();
          for (auto& ai : playerAis) {
            ai->notifyPassTurn(currPlayer.m_id);
          }
        }
      };

      if (activeDoubles) {
        Train& doublesTrain = board.getTrainById(activeDoublesTrainId);
        int32 pips = doublesTrain.m_tiles.back().m_tile.m_highPips;
        if (game.playerHasPlay(currPlayer, {pips})) {
          game.playTile(currPlayer, {activeDoublesTrainId}, "You must play a valid tile from your hand onto the active doubles.", &activeDoubles, &activeDoublesTrainId, &roundOver);
          lastActionTurn = playerTurn;
          if (!activeDoubles) {
            playerTurn = (playerTurn + 1) % players.size();
          }
        } else {
          drawOrPass();
        }
      } else if (currPlayerTrain.m_tiles.size() > 0) {
        std::set<int32> playablePips = game.standardPlayablePips(currPlayer);
        if (game.playerHasPlay(currPlayer, playablePips)) {
          game.playTile(currPlayer, game.standardPlayableTrains(currPlayer), "You must make a valid play.", &activeDoubles, &activeDoublesTrainId, &roundOver);
          lastActionTurn = playerTurn;
          if (!activeDoubles) {
            playerTurn = (playerTurn + 1) % players.size();
          }
        } else {
          drawOrPass();
        }
      } else {
        if (game.playerHasPlay(currPlayer, {board.m_centerTile->m_highPips})) {
          game.playTile(currPlayer, {currPlayerTrain.m_id}, "You must play onto your own train first.", &activeDoubles, &activeDoublesTrainId, &roundOver);
        } else {
          drawOrPass();
        }
      }
    }

    for (auto& ai : playerAis) {
      ai->notifyRoundEnd();
    }
  }

  for (auto& ai : playerAis) {
    ai->notifyGameEnd();
  }
}

