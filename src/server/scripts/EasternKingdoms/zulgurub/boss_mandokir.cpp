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
SDName: Boss_Mandokir
SD%Complete: 90
SDComment: Ohgan function needs improvements.
SDCategory: Zul'Gurub
EndScriptData */


#include "def_zulgurub.h"

#define SAY_AGGRO               -1309015
#define SAY_DING_KILL           -1309016
#define SAY_GRATS_JINDO         -1309017
#define SAY_WATCH               -1309018
#define SAY_WATCH_WHISPER       -1309019                    //is this text for real? easter egg?

#define SPELL_CHARGE            24315
#define SPELL_CLEAVE            20691
#define SPELL_FEAR              29321
#define SPELL_WHIRLWIND         24236
#define SPELL_MORTAL_STRIKE     24573
#define SPELL_ENRAGE            23537
#define SPELL_WATCH             24314
#define SPELL_LEVEL_UP          24312

//Ohgans Spells
#define SPELL_SUNDERARMOR       24317


//Ohgan

class boss_mandokir : public CreatureScript
{
public:
    boss_mandokir() : CreatureScript("boss_mandokir")
    { }

    class boss_mandokirAI : public ScriptedAI
    {
        public:
        boss_mandokirAI(Creature *c) : ScriptedAI(c)
        {
            pInstance = ((InstanceScript*)c->GetInstanceScript());
        }
    
        uint32 KillCount;
        uint32 Watch_Timer;
        uint32 TargetInRange;
        uint32 Cleave_Timer;
        uint32 Whirlwind_Timer;
        uint32 Fear_Timer;
        uint32 MortalStrike_Timer;
        uint32 Check_Timer;
        float targetX;
        float targetY;
        float targetZ;
    
        InstanceScript *pInstance;
    
        bool endWatch;
        bool someWatched;
        bool RaptorDead;
        bool CombatStart;
    
        uint64 WatchTarget;
    
        void Reset()
        override {
            KillCount = 0;
            Watch_Timer = 33000;
            Cleave_Timer = 7000;
            Whirlwind_Timer = 20000;
            Fear_Timer = 1000;
            MortalStrike_Timer = 1000;
            Check_Timer = 1000;
    
            targetX = 0.0;
            targetY = 0.0;
            targetZ = 0.0;
            TargetInRange = 0;
    
            WatchTarget = 0;
    
            someWatched = false;
            endWatch = false;
            RaptorDead = false;
            CombatStart = false;
    
            DoCast(me, 23243);
        }
    
        void KilledUnit(Unit* victim)
        override {
            if(victim->GetTypeId() == TYPEID_PLAYER)
            {
                ++KillCount;
    
                if (KillCount == 3)
                {
                    DoScriptText(SAY_DING_KILL, me);
    
                    if (pInstance)
                    {
                        uint64 JindoGUID = pInstance->GetData64(DATA_JINDO);
                        if (JindoGUID)
                        {
                            if (Unit* jTemp = ObjectAccessor::GetUnit(*me,JindoGUID))
                            {
                                if (jTemp->IsAlive())
                                    DoScriptText(SAY_GRATS_JINDO, jTemp);
                            }
                        }
                    }
                DoCast(me, SPELL_LEVEL_UP, true);
                 KillCount = 0;
                }
            }
        }
    
        void EnterCombat(Unit *who)
        override {
         DoScriptText(SAY_AGGRO, me);
        }
    
        void UpdateAI(const uint32 diff)
        override {
            if(!UpdateVictim())
                return;
    
            if( me->GetVictim() && me->IsAlive())
            {
                if(!CombatStart)
                {
                    //At combat Start Mandokir is mounted so we must unmount it first
                    me->Dismount();
    
                    //And summon his raptor
                    me->SummonCreature(14988, me->GetVictim()->GetPositionX(), me->GetVictim()->GetPositionY(), me->GetVictim()->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 35000);
                    CombatStart = true;
                }
    
                if (Watch_Timer < diff)                         //Every 20 Sec Mandokir will check this
                {
                    if(WatchTarget)                             //If someone is watched and If the Position of the watched target is different from the one stored, or are attacking, mandokir will charge him
                    {
                        Unit* pUnit = ObjectAccessor::GetUnit(*me, WatchTarget);
    
                        if( pUnit && (
                            targetX != pUnit->GetPositionX() ||
                            targetY != pUnit->GetPositionY() ||
                            targetZ != pUnit->GetPositionZ() ||
                            pUnit->IsInCombat()))
                        {
                            if(me->IsWithinMeleeRange(pUnit))
                            {
                                DoCast(pUnit,24316);
                            }
                            else
                            {
                                DoCast(pUnit,SPELL_CHARGE);
                                //me->SendMonsterMove(pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ(), 0, true,1);
                                AttackStart(pUnit);
                            }
                        }
                    }
                    someWatched = false;
                    Watch_Timer = 20000;
                }else Watch_Timer -= diff;
    
                if ((Watch_Timer < 8000) && !someWatched)       //8 sec(cast time + expire time) before the check for the watch effect mandokir will cast watch debuff on a random target
                {
                    if (Unit* p = SelectTarget(SELECT_TARGET_RANDOM,0))
                    {
                        DoScriptText(SAY_WATCH, me, p);
                        DoCast(p, SPELL_WATCH);
                        WatchTarget = p->GetGUID();
                        someWatched = true;
                        endWatch = true;
                    }
                }
    
                if ((Watch_Timer < 1000) && endWatch)           //1 sec before the debuf expire, store the target position
                {
                    Unit* pUnit = ObjectAccessor::GetUnit(*me, WatchTarget);
                    if (pUnit)
                    {
                        targetX = pUnit->GetPositionX();
                        targetY = pUnit->GetPositionY();
                        targetZ = pUnit->GetPositionZ();
                    }
                    endWatch = false;
                }
    
                if(!someWatched)
                {
                    //Cleave
                    if (Cleave_Timer < diff)
                    {
                        DoCast(me->GetVictim(),SPELL_CLEAVE);
                        Cleave_Timer = 7000;
                    }else Cleave_Timer -= diff;
    
                    //Whirlwind
                    if (Whirlwind_Timer < diff)
                    {
                        DoCast(me,SPELL_WHIRLWIND);
                        Whirlwind_Timer = 18000;
                    }else Whirlwind_Timer -= diff;
    
                    //If more then 3 targets in melee range mandokir will cast fear
                    if (Fear_Timer < diff)
                    {
                        TargetInRange = 0;
    
                        auto i = me->getThreatManager().getThreatList().begin();
                        for(; i != me->getThreatManager().getThreatList().end(); ++i)
                        {
                            Unit* pUnit = ObjectAccessor::GetUnit(*me, (*i)->getUnitGuid());
                            if(pUnit && me->IsWithinMeleeRange(pUnit))
                                TargetInRange++;
                        }
    
                        if(TargetInRange > 3)
                            DoCast(me->GetVictim(),SPELL_FEAR);
    
                        Fear_Timer = 4000;
                    }else Fear_Timer -=diff;
    
                    //Mortal Strike if target below 50% hp
                    if (me->GetVictim() && me->GetVictim()->GetHealth() < me->GetVictim()->GetMaxHealth()*0.5)
                    {
                        if (MortalStrike_Timer < diff)
                        {
                            DoCast(me->GetVictim(),SPELL_MORTAL_STRIKE);
                            MortalStrike_Timer = 15000;
                        }else MortalStrike_Timer -= diff;
                    }
                }
                //Checking if Ohgan is dead. If yes Mandokir will enrage.
                if(Check_Timer < diff)
                {
                    if(pInstance)
                    {
                        if(pInstance->GetData(DATA_OHGANISDEAD))
                        {
                            if (!RaptorDead)
                            {
                                DoCast(me, SPELL_ENRAGE);
                                RaptorDead = true;
                            }
                        }
                    }
    
                    Check_Timer = 1000;
                }else Check_Timer -= diff;
    
                DoMeleeAttackIfReady();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_mandokirAI(creature);
    }
};


class mob_ohgan : public CreatureScript
{
public:
    mob_ohgan() : CreatureScript("mob_ohgan")
    { }

    class mob_ohganAI : public ScriptedAI
    {
        public:
        mob_ohganAI(Creature *c) : ScriptedAI(c)
        {
            pInstance = ((InstanceScript*)c->GetInstanceScript());
        }
    
        uint32 SunderArmor_Timer;
        InstanceScript *pInstance;
    
        void Reset()
        override {
            SunderArmor_Timer = 5000;
        }
    
        void EnterCombat(Unit *who) override {}
    
        void JustDied(Unit* Killer)
        override {
            if(pInstance)
                pInstance->SetData(DATA_OHGAN_DEATH, 0);
        }
    
        void UpdateAI (const uint32 diff)
        override {
            //Return since we have no target
            if (!UpdateVictim() )
                return;
    
            //SunderArmor_Timer
            if(SunderArmor_Timer < diff)
            {
                DoCast(me->GetVictim(), SPELL_SUNDERARMOR);
                SunderArmor_Timer = 10000 + rand()%5000;
            }else SunderArmor_Timer -= diff;
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_ohganAI(creature);
    }
};


void AddSC_boss_mandokir()
{

    new boss_mandokir();

    new mob_ohgan();
}
