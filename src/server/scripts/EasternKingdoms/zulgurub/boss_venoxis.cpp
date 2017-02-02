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
SDName: Boss_Venoxis
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */


#include "def_zulgurub.h"

#define SAY_TRANSFORM       -1309000
#define SAY_DEATH           -1309001

#define SPELL_HOLY_FIRE     23860
#define SPELL_HOLY_WRATH    28883                           //Not sure if this or 23979
#define SPELL_VENOMSPIT     23862
#define SPELL_HOLY_NOVA     23858
#define SPELL_POISON_CLOUD  23861
#define SPELL_SNAKE_FORM    23849
#define SPELL_RENEW         23895
#define SPELL_BERSERK       23537
#define SPELL_DISPELL       23859

struct boss_venoxisAI : public ScriptedAI
{
    boss_venoxisAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript *pInstance;

    uint32 HolyFire_Timer;
    uint32 HolyWrath_Timer;
    uint32 VenomSpit_Timer;
    uint32 Renew_Timer;
    uint32 PoisonCloud_Timer;
    uint32 HolyNova_Timer;
    uint32 Dispell_Timer;
    uint32 TargetInRange;

    bool PhaseTwo;
    bool InBerserk;

    void Reset()
    override {
        HolyFire_Timer = 10000;
        HolyWrath_Timer = 60500;
        VenomSpit_Timer = 5500;
        Renew_Timer = 30500;
        PoisonCloud_Timer = 2000;
        HolyNova_Timer = 5000;
        Dispell_Timer = 35000;
        TargetInRange = 0;

        PhaseTwo = false;
        InBerserk= false;
    }

    void EnterCombat(Unit *who)
    override {
    }

    void JustDied(Unit* Killer)
    override {
        DoScriptText(SAY_DEATH, me);
        if(pInstance)
            pInstance->SetData(DATA_VENOXIS_DEATH, 0);
    }

    void UpdateAI(const uint32 diff)
    override {
          if (!UpdateVictim())
            return;

            if ((me->GetHealth()*100 / me->GetMaxHealth() > 50))
            {
                if (Dispell_Timer < diff)
                {
                    DoCast(me, SPELL_DISPELL);
                    Dispell_Timer = 15000 + rand()%15000;
                }else Dispell_Timer -= diff;

                if (Renew_Timer < diff)
                {
                    DoCast(me, SPELL_RENEW);
                    Renew_Timer = 20000 + rand()%10000;
                }else Renew_Timer -= diff;

                if (HolyWrath_Timer < diff)
                {
                    DoCast(me->GetVictim(), SPELL_HOLY_WRATH);
                    HolyWrath_Timer = 15000 + rand()%10000;
                }else HolyWrath_Timer -= diff;

                if (HolyNova_Timer < diff)
                {
                    TargetInRange = 0;
                    for(int i=0; i<10; i++)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO,i))
                            if(me->IsWithinMeleeRange(target))
                                TargetInRange++;
                    }

                    if(TargetInRange > 1)
                    {
                        DoCast(me->GetVictim(),SPELL_HOLY_NOVA);
                        HolyNova_Timer = 1000;
                    }
                    else
                    {
                        HolyNova_Timer = 2000;
                    }

                }else HolyNova_Timer -= diff;

                if (HolyFire_Timer < diff && TargetInRange < 3)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0))
                        DoCast(target, SPELL_HOLY_FIRE);

                    HolyFire_Timer = 8000;
                }else HolyFire_Timer -= diff;
            }
            else
            {
                if(!PhaseTwo)
                {
                    DoScriptText(SAY_TRANSFORM, me);
                    me->InterruptNonMeleeSpells(false);
                    DoCast(me,SPELL_SNAKE_FORM);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X, 2.00f);
                    /*
                    const CreatureTemplate *cinfo = me->GetCreatureTemplate();
                    me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 25)));
                    me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 25)));
                    me->UpdateDamagePhysical(BASE_ATTACK); */
                    DoResetThreat();
                    PhaseTwo = true;
                }

                if(PhaseTwo && PoisonCloud_Timer < diff)
                {
                    DoCast(me->GetVictim(), SPELL_POISON_CLOUD);
                    PoisonCloud_Timer = 15000;
                }PoisonCloud_Timer -=diff;

                if (PhaseTwo && VenomSpit_Timer < diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0))
                        DoCast(target, SPELL_VENOMSPIT);

                    VenomSpit_Timer = 15000 + rand()%5000;
                }else VenomSpit_Timer -= diff;

                if (PhaseTwo && (me->GetHealth()*100 / me->GetMaxHealth() < 11))
                {
                    if (!InBerserk)
                    {
                        me->InterruptNonMeleeSpells(false);
                        DoCast(me, SPELL_BERSERK);
                        InBerserk = true;
                    }
                }
            }
            DoMeleeAttackIfReady();

    }
};
CreatureAI* GetAI_boss_venoxis(Creature *_Creature)
{
    return new boss_venoxisAI (_Creature);
}

void AddSC_boss_venoxis()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name="boss_venoxis";
    newscript->GetAI = &GetAI_boss_venoxis;
    sScriptMgr->RegisterOLDScript(newscript);
}
