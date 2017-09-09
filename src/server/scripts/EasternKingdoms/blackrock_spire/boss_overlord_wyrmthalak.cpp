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
SDName: Boss_Overlord_Wyrmthalak
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */



#define SPELL_BLASTWAVE         11130
#define SPELL_SHOUT             23511
#define SPELL_CLEAVE            20691
#define SPELL_KNOCKAWAY         20686

#define ADD_1X -39.355381
#define ADD_1Y -513.456482
#define ADD_1Z 88.472046
#define ADD_1O 4.679872

#define ADD_2X -49.875881
#define ADD_2Y -511.896942
#define ADD_2Z 88.195160
#define ADD_2O 4.613114

class boss_overlord_wyrmthalak : public CreatureScript
{
public:
    boss_overlord_wyrmthalak() : CreatureScript("boss_overlord_wyrmthalak")
    { }

    class boss_overlordwyrmthalakAI : public ScriptedAI
    {
        public:
        boss_overlordwyrmthalakAI(Creature *c) : ScriptedAI(c) {}
    
        uint32 BlastWave_Timer;
        uint32 Shout_Timer;
        uint32 Cleave_Timer;
        uint32 Knockaway_Timer;
        bool Summoned;
        Creature *SummonedCreature;
    
        void Reset()
        override {
            BlastWave_Timer = 20000;
            Shout_Timer = 2000;
            Cleave_Timer = 6000;
            Knockaway_Timer = 12000;
            Summoned = false;
        }
    
        void EnterCombat(Unit *who)
        override {
        }
    
        void UpdateAI(const uint32 diff)
        override {
            //Return since we have no target
            if (!UpdateVictim() )
                return;
    
            //BlastWave_Timer
            if (BlastWave_Timer < diff)
            {
                DoCast(me->GetVictim(),SPELL_BLASTWAVE);
                BlastWave_Timer = 20000;
            }else BlastWave_Timer -= diff;
    
            //Shout_Timer
            if (Shout_Timer < diff)
            {
                DoCast(me->GetVictim(),SPELL_SHOUT);
                Shout_Timer = 10000;
            }else Shout_Timer -= diff;
    
            //Cleave_Timer
            if (Cleave_Timer < diff)
            {
                DoCast(me->GetVictim(),SPELL_CLEAVE);
                Cleave_Timer = 7000;
            }else Cleave_Timer -= diff;
    
            //Knockaway_Timer
            if (Knockaway_Timer < diff)
            {
                DoCast(me->GetVictim(),SPELL_KNOCKAWAY);
                Knockaway_Timer = 14000;
            }else Knockaway_Timer -= diff;
    
            //Summon two Beserks
            if ( !Summoned && me->GetHealthPct() < 51 )
            {
                Unit* target = nullptr;
                target = SelectTarget(SELECT_TARGET_RANDOM,0);
    
                SummonedCreature = me->SummonCreature(9216,ADD_1X,ADD_1Y,ADD_1Z,ADD_1O,TEMPSUMMON_TIMED_DESPAWN,300000);
                ((CreatureAI*)SummonedCreature->AI())->AttackStart(target);
                SummonedCreature = me->SummonCreature(9268,ADD_2X,ADD_2Y,ADD_2Z,ADD_2O,TEMPSUMMON_TIMED_DESPAWN,300000);
                ((CreatureAI*)SummonedCreature->AI())->AttackStart(target);
                Summoned = true;
            }
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_overlordwyrmthalakAI(creature);
    }
};


void AddSC_boss_overlordwyrmthalak()
{
    new boss_overlord_wyrmthalak();
}
