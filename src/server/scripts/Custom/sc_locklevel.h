#ifndef SC_LOCKLEVEL_H
#define SC_LOCKLEVEL_H

#include "../../game/Scripting/ScriptMgr.h"
#include "../../game/Entities/Player/Player.h"

class LockLevelPlayer : public PlayerScript
{
public:

    // Called when a player completes a quest
    void OnPlayerCompleteQuest(Player* player, Quest const* quest_id) override;

    // Called when a player's level changes (right after the level is applied)
    void OnLevelChanged(Player* player, uint8 oldlevel) override;

    // Called when a player logs in.
    void OnLogin(Player* player) override;

    // Called when a player is created.
    void OnCreate(Player* player) override;

    // Called when a player is deleted.
    void OnDelete(uint64 guid, uint32 accountId) override;

private:

    // Returns the current "maxlevel" in quest_maxlevel for this quest
    static uint32 GetQuestMaxlevel(const Quest* quest);

    // Returns the current "maxlevel" in character_maxlevel for this player
    static uint32 GetCharacterMaxlevel(const Player* player);

    // Returns true when a row in character_maxlevel is present for this player
    static bool CheckCharacterMaxlevelCreated(const Player* player);

    // Create a row in character_maxlevel for this player
    static void CreateCharacterMaxlevel(const Player* player);

    // Update the "maxlevel" in character_maxlevel for this player
    static void UpdateCharacterMaxlevel(const Player* player,
        uint32 quest_maxlevel);

    // Delete the row in character_maxlevel for this player
    static void DeleteCharacterMaxlevel(const uint64 guid);
};

class LockLevelQuest : public WorldScript
{
public:

    // Called when loading custom database tables
    void OnLoadCustomDatabaseTable() override;

private:

    void InitQuestMaxLevel();

    static uint32 RetrieveLowestMaxLevelItem();
};

void AddSC_LockLevel();

#endif
