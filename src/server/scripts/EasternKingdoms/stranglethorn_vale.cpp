/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Stranglethorn_Vale
SD%Complete: 100
SDComment: Quest support: 592
SDCategory: Stranglethorn Vale
EndScriptData */

/* ContentData
mob_yenniku
EndContentData */



/*######
## mob_yenniku
######*/

class mob_yenniku : public CreatureScript
{
public:
    mob_yenniku() : CreatureScript("mob_yenniku")
    { }

    class mob_yennikuAI : public ScriptedAI
    {
        public:
        mob_yennikuAI(Creature *c) : ScriptedAI(c)
        {
            bReset = false;
        }
    
        uint32 Reset_Timer;
        bool bReset;
    
        void Reset()
        override {
            Reset_Timer = 0;
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
        }
    
        void SpellHit(Unit *caster, const SpellInfo *spell)
        override {
            if (caster->GetTypeId() == TYPEID_PLAYER)
            {
                                                                //Yenniku's Release
                if(!bReset && (caster->ToPlayer())->GetQuestStatus(592) == QUEST_STATUS_INCOMPLETE && spell->Id == 3607)
                {
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STUN);
                    me->CombatStop();                   //stop combat
                    me->DeleteThreatList();             //unsure of this
                    me->SetFaction(FACTION_HORDE_GENERIC);                 //horde generic
    
                    bReset = true;
                    Reset_Timer = 60000;
                }
            }
            return;
        }
    
        void EnterCombat(Unit *who) override {}
    
        void UpdateAI(const uint32 diff)
        override {
            if (bReset)
            {
                if(Reset_Timer < diff)
                {
                    EnterEvadeMode();
                    bReset = false;
                    me->SetFaction(28);                     //troll, bloodscalp
                    return;
                }
                else Reset_Timer -= diff;
    
                if(me->IsInCombat() && me->GetVictim())
                {
                    if(me->GetVictim()->GetTypeId() == TYPEID_PLAYER)
                    {
                        Unit *victim = me->GetVictim();
                        if((victim->ToPlayer())->GetTeam() == HORDE)
                        {
                            me->CombatStop();
                            me->DeleteThreatList();
                        }
                    }
                }
             }
    
            //Return since we have no target
            if (!UpdateVictim() )
                return;
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_yennikuAI(creature);
    }
};


/*######
##
######*/

void AddSC_stranglethorn_vale()
{

    new mob_yenniku();
}
