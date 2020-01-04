#include <charconv>
#include <string_view>
#include "Common.h"
#include "DatabaseEnv.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "Player.h"


typedef std::unordered_map<uint32, uint32> LockLevelQuestStore;

static LockLevelQuestStore QuestMaxLevelItems;
static uint32 QuestLowestMaxLevelItem;

class LockLevelPlayer : public PlayerScript
{
public:
    LockLevelPlayer() : PlayerScript("LockLevelPlayer")
    {
    }

    // Called when a player completes a quest
    void OnPlayerCompleteQuest(Player* player, Quest const* quest_id) override
    {
        const uint32 quest_level = GetQuestMaxlevel(quest_id);

        if (quest_level > player->getLevel())
        {
            const uint32 character_current_maxlevel = GetCharacterMaxlevel(
                player);

            if (character_current_maxlevel < quest_level)
            {
                UpdateCharacterMaxlevel(player, quest_level);
                if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN))
                {
                    player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
                }
            }
        }
    }

    // Called when a player's level changes (right after the level is applied)
    void OnLevelChanged(Player* player, uint8 oldlevel) override
    {
        const uint32 max_level = GetCharacterMaxlevel(player);
        if (player->getLevel() >= max_level)
        {
            player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        } else if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN))
        {
            player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        }
    }

    // Called when a player logs in.
    void OnLogin(Player* player) override
    {
        if (!CheckCharacterMaxlevelCreated(player))
        {
            CreateCharacterMaxlevel(player);
        }
    }

    // Called when a player is created.
    void OnCreate(Player* player) override
    {
        CreateCharacterMaxlevel(player);
    }

    // Called when a player is deleted.
    void OnDelete(uint64 guid, uint32 accountId) override
    {
        DeleteCharacterMaxlevel(guid);
    }

private:

    // Returns the current "maxlevel" in quest_maxlevel for this quest
    static uint32 GetQuestMaxlevel(const Quest* quest)
    {
        const uint32 result = QuestMaxLevelItems[quest->GetQuestId()];
        return result ? result : 0;
    }

    // Returns the current "maxlevel" in character_maxlevel for this player
    static uint32 GetCharacterMaxlevel(const Player* player)
    {

        const auto result = CharacterDatabase.PQuery(
            "SELECT maxlevel FROM character_maxlevel WHERE guid = %u",
            player->GetGUID());

        if (result)
        {
            const auto fields = result->Fetch();
            return fields[0].GetUInt32();
        }
        else
        {
            return 0;
        }
    }

    // Returns true when a row in character_maxlevel is present for this player
    static bool CheckCharacterMaxlevelCreated(const Player* player)
    {
        const auto result = CharacterDatabase.PQuery(
            "SELECT 1 FROM character_maxlevel WHERE guid = %u",
            player->GetGUID());

        return result;
    }

    // Create a row in character_maxlevel for this player
    static void CreateCharacterMaxlevel(const Player* player)
    {
        CharacterDatabase.DirectPExecute(
            "INSERT INTO character_maxlevel(guid, maxlevel) VALUES (%u, %u)",
            player->GetGUID(),
            QuestLowestMaxLevelItem
        );
    }

    // Update the "maxlevel" in character_maxlevel for this player
    static void UpdateCharacterMaxlevel(const Player* player,
                                          uint32 quest_maxlevel)
    {
        CharacterDatabase.DirectPExecute(
            "UPDATE character_maxlevel SET maxlevel = %u WHERE guid = %u",
            quest_maxlevel,
            player->GetGUID()
        );
    }

    // Delete the row in character_maxlevel for this player
    static void DeleteCharacterMaxlevel(const uint64 guid)
    {
        CharacterDatabase.DirectPExecute(
            "DELETE FROM character_maxlevel WHERE guid = %u",
            guid
        );
    }
};

class LockLevelQuest : public WorldScript
{
public:
    LockLevelQuest() : WorldScript("LockLevelQuest")
    {
    }

    // Called when loading custom database tables
    void OnLoadCustomDatabaseTable() override
    {
        InitQuestMaxLevel();
    }

private:

    void InitQuestMaxLevel()
    {
        const auto result = WorldDatabase.PQuery(
            "SELECT id, maxlevel FROM quest_maxlevel");

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();

                uint32 id = fields[0].GetUInt32();
                uint32 maxlevel = fields[1].GetUInt32();

                QuestMaxLevelItems[id] = maxlevel;

            } while (result->NextRow());
        }

        QuestLowestMaxLevelItem = RetrieveLowestMaxLevelItem();
    }

    static uint32 RetrieveLowestMaxLevelItem()
    {
        uint32 LowestMaxLevel = UINT_MAX;
        for (const pair<uint32, uint32> pair : QuestMaxLevelItems)
        {
            if (pair.second < LowestMaxLevel)
            {
                LowestMaxLevel = pair.second;
            }
        }

        return LowestMaxLevel;
    }
};

void AddSC_LockLevel()
{
    new LockLevelQuest();
    new LockLevelPlayer();
}
