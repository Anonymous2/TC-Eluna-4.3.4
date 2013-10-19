/* 
   Derp Says
   By Tommy
   EmuDevs <http://emudevs.com>
*/
enum NpcIds
{
    NPC_SPOTLIGHT_ONE               = 79900,
    NPC_SPOTLIGHT_TWO               = 79901,
    NPC_SPOTLIGHT_THREE             = 79902,
    NPC_TURTLE_ONE                  = 79903,
    NPC_FIRE_HAWK                   = 53245,
};

enum SpellIds
{
    /* Spells */
    SPELL_SPOTLIGHT                 = 50236,
};

enum EventIds
{
    EVENT_NONE,
    EVENT_EXPLAIN_1,
    EVENT_EXPLAIN_2,
    EVENT_EXPLAIN_SPOTLIGHT,
    EVENT_SPOTLIGHT_1,
    EVENT_SPOTLIGHT_2,
    EVENT_CHECK_SPOTLIGHT,
    EVENT_CHECK_SPOTLIGHT_2,
    EVENT_EXPLAIN_KILL_CREATURE,
    EVENT_EXPLAIN_KILL_CREATURE_START,
    EVENT_CHECK_KILL_CREATURE
};

static bool isEventActive = false;

class simon_himself : public CreatureScript
{
public:
    simon_himself() : CreatureScript("simon_himself") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (isEventActive)
        {
            player->GetSession()->SendNotification("Event is already active!");
            return false;
        }
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Play Simon Says", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nevermind..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
    {
        player->PlayerTalkClass->ClearMenus();
        simon_himselfAI* simonAI = CAST_AI(simon_himselfAI, creature->GetAI());

        if (actions == GOSSIP_ACTION_INFO_DEF)
        {
            simonAI->Start(player->GetGUID());
            player->CLOSE_GOSSIP_MENU();
        }
        else
            player->CLOSE_GOSSIP_MENU();
		return true;
    }

    struct simon_himselfAI : public ScriptedAI
    {
        simon_himselfAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        uint64 playerGUID;
        int killTimer;
        int eventCheck;
        float x, y, z;

        void Reset()
        {
            events.Reset();
            eventCheck = 0;
            playerGUID = 0;
            killTimer = 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_EXPLAIN_1:
                        Explain("Waz up!? Today, we're going to play Simon Says! What's Simon says? Well, you do as I say and you will win! Else, you will BE A LOSER!");
                        events.ScheduleEvent(EVENT_EXPLAIN_2, 7000);
                        break;
                    case EVENT_EXPLAIN_2:
                        Explain("Let's play.");
                        events.ScheduleEvent(EVENT_SPOTLIGHT_1, 3000);
                        break;
                    case EVENT_SPOTLIGHT_1:
                        me->GetPosition(x, y, z);
                        spotLight = me->SummonCreature(NPC_SPOTLIGHT_ONE, x, y - 7, z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
                        AddLight(spotLight);
                        Explain("We're going to play a game called spotlight! Alright, Simon Says jump into the spotlight!");
                        events.ScheduleEvent(EVENT_CHECK_SPOTLIGHT, 1000);
                        break;
                    case EVENT_CHECK_SPOTLIGHT:
                        if (spotLight)
                        {
                            if (CheckPlayerDistance(spotLight) <= 2.0f && eventCheck == 0)
                            {
                                events.CancelEvent(EVENT_CHECK_SPOTLIGHT);
                                spotLight->DespawnOrUnsummon(100);
                                Explain("Ha! Okay, that much was obvious!");
                                events.ScheduleEvent(EVENT_EXPLAIN_SPOTLIGHT, 2000);
                                eventCheck++;
                            }
                        }
                        if (eventCheck == 0)
                            events.ScheduleEvent(EVENT_CHECK_SPOTLIGHT, 1000);
                        break;
                    case EVENT_EXPLAIN_SPOTLIGHT:
                        Explain("Now I want you to beg for mercy! Wait.. that was harsh.. Simon says GET INTO THE SPOTLIGHT YOU WERE PREVIOUSLY IN!");
                        events.ScheduleEvent(EVENT_SPOTLIGHT_2, 8000);
                        break;
                    case EVENT_SPOTLIGHT_2:
                        spotLight = me->SummonCreature(NPC_SPOTLIGHT_ONE, x - 10, y - 7, z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
                        spotLightTwo = me->SummonCreature(NPC_SPOTLIGHT_TWO, x + 10, y - 7, z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
                        spotLightThree = me->SummonCreature(NPC_SPOTLIGHT_THREE, x + 25, y, z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
                        AddLight(spotLight);
                        AddLight(spotLightTwo);
                        AddLight(spotLightThree);
                        events.ScheduleEvent(EVENT_CHECK_SPOTLIGHT_2, 1000);
                        break;
                    case EVENT_CHECK_SPOTLIGHT_2:
                        if (spotLight && spotLightTwo && spotLightThree)
                        {
                            if (CheckPlayerDistance(spotLightTwo) <= 2.0f || CheckPlayerDistance(spotLightThree) <= 2.0f)
                            {
                                spotLight->DespawnOrUnsummon(100);
                                spotLightTwo->DespawnOrUnsummon(100);
                                spotLightThree->DespawnOrUnsummon(100);
                                _Reset();
                                Explain("HAHA, you lost! No rewards for you!");
                                return;
                            }

                            if (CheckPlayerDistance(spotLight) <= 2.0f && eventCheck == 1)
                            {
                                events.CancelEvent(EVENT_CHECK_SPOTLIGHT_2);
                                spotLight->DespawnOrUnsummon(100);
                                spotLightTwo->DespawnOrUnsummon(100);
                                spotLightThree->DespawnOrUnsummon(100);
                                Explain("Darn! You actually did it... Simon is getting MAD!");
                                eventCheck++;
                                events.ScheduleEvent(EVENT_EXPLAIN_KILL_CREATURE, 5000);
                            }
                        }
                        if (eventCheck == 1)
                            events.ScheduleEvent(EVENT_CHECK_SPOTLIGHT_2, 1000);
                        break;
                    case EVENT_EXPLAIN_KILL_CREATURE:
                        Explain("Simon says kill this Fire Hawk in 6 seconds!");
                        events.ScheduleEvent(EVENT_EXPLAIN_KILL_CREATURE_START, 5000);
                        break;
                    case EVENT_EXPLAIN_KILL_CREATURE_START:
                        Explain("GO!");
                        fireHawk = me->SummonCreature(NPC_FIRE_HAWK, x, y - 5, z, 1.0, TEMPSUMMON_MANUAL_DESPAWN, 0);
                        events.ScheduleEvent(EVENT_CHECK_KILL_CREATURE, 1000);
                        break;
                    case EVENT_CHECK_KILL_CREATURE:
                        if (fireHawk)
                        {
                            if (killTimer >= 6 && eventCheck == 2)
                            {
                                if (!fireHawk->IsAlive())
                                {
                                     events.CancelEvent(EVENT_CHECK_KILL_CREATURE);
                                     fireHawk->DespawnOrUnsummon(100);
                                     Explain("Wow, you win! Congratulations!");
                                     WinOrLose(true);
                                     eventCheck++;
                                     _Reset();
                                }
                                else
                                {
                                    fireHawk->DespawnOrUnsummon(100);
                                    _Reset();
                                    Explain("HA, you lose!");
                                }
                            }
                            killTimer++;
                        }
                        if (eventCheck == 2)
                            events.ScheduleEvent(EVENT_CHECK_KILL_CREATURE, 1000);
                        break;
                }
            }
        }

        float CheckPlayerDistance(Creature* target)
        {
            if (!target)
                return 0;

            if (Player* player = target->GetPlayer(*target, playerGUID))
                return target->GetDistance2d(player);
            return NULL;
        }

        void Explain(const char* msg)
        {
            me->MonsterSay(msg, LANG_UNIVERSAL, 0);
            me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
        }

        void WinOrLose(bool won) // Call this if the player wins or loses
        {
            Player* player = me->GetPlayer(*me, playerGUID);
            if (player)
            {
                if (player->GetGUID() == playerGUID)
                {
                    if (won) // If the player won
                    {
                    }
                    else // If the player lost
                    {
                    }
                }
            }
        }

        void Start(uint64 guid) // Starts the event
        {
            playerGUID = guid;
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            events.ScheduleEvent(EVENT_EXPLAIN_1, 1000);
        }

        void AddLight(Creature* creature) // Adds spotlight to the specified creature
        {
            if (!creature)
                return;

            creature->AddAura(SPELL_SPOTLIGHT, creature);
        }

        void _Reset()
        {
            spotLight = NULL;
            spotLightTwo = NULL;
            spotLightThree = NULL;
            fireHawk = NULL;
            eventCheck = 0;
            playerGUID = 0;
            killTimer = 0;
            isEventActive = false;
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            events.Reset();
        }

        private:
            Creature* spotLight;
            Creature* spotLightTwo;
            Creature* spotLightThree;
            Creature* fireHawk;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new simon_himselfAI(creature);
    }
};

void AddSC_simon_says()
{
    new simon_himself();
}