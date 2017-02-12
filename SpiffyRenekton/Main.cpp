#include "PluginSDK.h"
#include "Indicator.h"
#include "SpiffyC.h"
#include <cmath>
#include <algorithm>

using namespace std;

PluginSetup("SpiffyRenekton - xSlapppz");

ISpell2* Q, *W, *E, *R, *Ignite;

IInventoryItem* titHydra, *ravHydra;

IMenu* MainMenu, *dsettings, *csettings, *Hsettings, *Lcsettings, *Msettings;

//dsettings
IMenuOption* drawqq, *drawee, *drawrr, *drawcombo, *rDamage;
//csettings
IMenuOption* useq, *usew, *usee, *user, *userindanger, *furyMode, *useIgnite;
//Hsettings
IMenuOption* useqH, *usewH, *useCH, *donteqwebtower;
//Misc Setting
IMenuOption* useHydra;

Indicator ID = Indicator(true, true);
SpiffyCommon SC = SpiffyCommon();

static float lastE;
static Vec3 lastEpos;
static bool wChancel = false;

bool BlockE = false;

void Menu()
{
	MainMenu = GPluginSDK->AddMenu("SpiffyRenekton");
	
	dsettings = MainMenu->AddMenu("Drawings");
	{
		drawqq = dsettings->CheckBox("Draw Q", true);
		drawee = dsettings->CheckBox("Draw E", true);
		drawrr = dsettings->CheckBox("Draw R", true);
		drawcombo = dsettings->CheckBox("Draw Combo Damage", true);
	}
	
	csettings = MainMenu->AddMenu("Combo");
	{
		useq = csettings->CheckBox("Use Q",true);
		usew = csettings->CheckBox("Use W", true);
		usee = csettings->CheckBox("Use E", true);
		user = csettings->AddInteger("Use R under", 0,100,15);
		userindanger = csettings->AddInteger("Use R min X enemy", 1, 5, 2);
		furyMode = csettings->AddInteger("Fury Priority", 1, 3, 1);
		useIgnite = csettings->CheckBox("Use Ignite", true);

	}

	Hsettings = MainMenu->AddMenu("Harass");
	{
		useqH = Hsettings->CheckBox("Use Q",true);
		usewH = Hsettings->CheckBox("Use W",true);
		useCH = Hsettings->AddInteger("Harass Mode", 1, 2, 1);
		donteqwebtower = Hsettings->CheckBox("Dont Q Under Tower", true);
	}

	Msettings = MainMenu->AddMenu("Misc");
	{
		useHydra = Msettings->CheckBox("Use Titanic/Ravenous", true);
	}
}

void LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kCircleCast, false, true, kCollidesWithNothing);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, false, kCollidesWithNothing);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, true, kCollidesWithNothing);
	
	if (GEntityList->Player()->GetSpellSlot("SummonerDot") != kSlotUnknown)
		Ignite = GPluginSDK->CreateSpell2(GEntityList->Player()->GetSpellSlot("SummonerDot"), kTargetCast, false, false, kCollidesWithNothing);

	titHydra = GPluginSDK->CreateItemForId(3748, 700);
	ravHydra = GPluginSDK->CreateItemForId(3074, 400);
}

static bool InRange(IUnit* i)
{
	return i != nullptr && SC.GetDistance(GEntityList->Player(), i) < Q->Range() - 40;
}

float CalcComboDmg(IUnit* Target)
{
	float dmg = 0;
	if (Q->IsReady())
		dmg += GDamage->CalcPhysicalDamage(GEntityList->Player(), Target, GDamage->GetSpellDamage(GEntityList->Player(), Target, kSlotQ));
	if(W->IsReady())
		dmg += GDamage->CalcPhysicalDamage(GEntityList->Player(), Target, GDamage->GetSpellDamage(GEntityList->Player(), Target, kSlotW));
	if(E->IsReady())
		dmg += GDamage->CalcPhysicalDamage(GEntityList->Player(), Target, GDamage->GetSpellDamage(GEntityList->Player(), Target, kSlotE));
	if(R->IsReady())
		dmg += GDamage->CalcPhysicalDamage(GEntityList->Player(), Target, GDamage->GetSpellDamage(GEntityList->Player(), Target, kSlotE)) * 15;
	return dmg;
}

static bool rene()
{
	return GEntityList->Player()->HasBuff("renektonsliceanddicedelay");
}

static bool fury()
{
	return GEntityList->Player()->HasBuff("renektonrageready");
}

static bool renw()
{
	return GEntityList->Player()->HasBuff("renektonpreexecute");
}

bool canBeOpWithQ(Vec3 vector3)
{
	if (fury())
	{
		return false;
	}
	if ((GEntityList->Player()->GetMana() > 45 && !fury()) ||
		(Q->IsReady() &&
			GEntityList->Player()->GetMana() + SC.CountMinionsInRange(vector3, Q->Range()) * 2.5 +
			SC.CountEnemiesInRange(Q->Range()) * 10 > 50))
	{
		return true;
	}
	return false;
}

bool canBeOpWithW()
{
	if (GEntityList->Player()->GetMana() + 20 > 50)
	{
		return true;
	}
	return false;
}

void Harass()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range());
	if (target == nullptr)
		return;
	switch (useCH->GetInteger())
	{
	case 1:
		if (Q->IsReady() && E->IsReady() && lastE == 0 && fury() && !rene())
		{
			if (donteqwebtower->Enabled() && SC.PosUnderTurret(SC.Extend(GEntityList->Player()->GetPosition(), target->GetPosition(), E->Range()), false, true))
			{
				return;
			}
			auto vecMinions = GEntityList->GetAllMinions(false, true, false);
			auto closeGapTarget = std::find_if(vecMinions.begin(), vecMinions.end(), InRange);

			if (closeGapTarget != vecMinions.end())
			{
				auto pUnit = (*closeGapTarget);
				lastEpos = GEntityList->Player()->GetPosition();
				E->CastOnPosition(pUnit->GetPosition());
				lastE = GGame->TickCount();
				return;
			}
			else
			{
				lastEpos = GEntityList->Player()->GetPosition();
				E->CastOnPosition(target->GetPosition());
				lastE = GGame->TickCount();
				return;
			}
		}
		return;
		break;

	case 0:
		if (Q->IsReady() && W->IsReady() && !rene() && GEntityList->Player()->IsValidTarget(target, E->Range()))
		{
			if (donteqwebtower->Enabled() && SC.PosUnderTurret(SC.Extend(GEntityList->Player()->GetPosition(), target->GetPosition(), E->Range()), false, true))
			{
				return;
			}
			if (E->CastOnTarget(target, kHitChanceHigh))
			{
				lastE = GGame->TickCount();
			}
		}
		if (rene() && E->IsReady() && GEntityList->Player()->IsValidTarget(target, E->Range()) && lastE != 0 && GGame->TickCount() - lastE > 3600)
		{
			E->CastOnTarget(target, kHitChanceHigh);
		}
		/*if (SC.GetDistance(GEntityList->Player(), target) < GOrbwalking->GetAutoAttackRange(target) && Q->IsReady() && E->IsReady())
		{
			GOrbwalking->ForceTarget?
		}*/
		return;
		break;

	default:
		break;
	}

	if (useqH->Enabled() && Q->IsReady() && GEntityList->Player()->IsValidTarget(target,Q->Range()))
	{
		Q->CastOnPlayer();
	}
	if (useCH->GetInteger() == 0 && lastE != 0 && rene() && !Q->IsReady() && !renw())
	{
		if (lastEpos.x != 0 && lastEpos.y != 0 && lastEpos.z != 0)
		{
			E->CastOnPosition(lastEpos);
		}
	}
}

bool checkFuryMode(eSpellSlot spellSlot, IUnit* target)
{
	if (GDamage->GetSpellDamage(GEntityList->Player(), target, spellSlot) > target->GetHealth())
	{
		return true;
	}
	if (canBeOpWithQ(GEntityList->Player()->GetPosition()) && spellSlot != kSlotQ)
	{
		return false;
	}
	if (!fury())
	{
		return true;
	}
	if (GEntityList->Player()->IsWindingUp())
	{
		return false;
	}
	switch (furyMode->GetInteger())
	{
	case 0:
		return true;
		break;
	case 1:
		if (spellSlot != kSlotQ && Q->IsReady())
		{
			return false;
		}
		break;
	case 2:
		if (spellSlot != kSlotW && W->IsReady())
		{
			return false;
		}
		break;
	case 3:
		if (spellSlot != kSlotE && E->IsReady())
		{
			return false;
		}
		break;
	}
	return true;
}

void ComboLogic()
{
	auto target = GTargetSelector->FindTarget(QuickestKill,PhysicalDamage, E->Range() * 2);
	if (target == nullptr)
		return;

	bool hasIgnite = GEntityList->Player()->GetSpellState(GEntityList->Player()->GetSpellSlot("SummonerDot")) == Ready;
	float FuryQ = GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotQ) * 0.5;
	float FuryW = GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotW) * 0.5;
	float eDmg = GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotE);
	float combodamage = CalcComboDmg(target);
	if (target != nullptr && useIgnite->Enabled() && hasIgnite && GDamage->GetSummonerSpellDamage(GEntityList->Player(), target, kSummonerSpellIgnite) > target->GetHealth())
	{
		Ignite->CastOnTarget(target);
	}
	if (SC.GetDistancePos(GEntityList->Player()->GetPosition(), target->GetPosition()) > E->Range() && E->IsReady() && (W->IsReady() || Q->IsReady() && lastE == 0) && usee->Enabled())
	{
		auto vecMinions = GEntityList->GetAllMinions(false, true, false);
		auto closeGapTarget = std::find_if(vecMinions.begin(), vecMinions.end(), InRange);

		if (closeGapTarget != vecMinions.end())
		{
			auto pUnit = (*closeGapTarget);
			if ((canBeOpWithQ(pUnit->GetPosition()) || fury()) && !rene())
			{
				if (E->IsReady() && GEntityList->Player()->IsValidTarget(pUnit, E->Range()))
				{
					E->CastOnPosition(pUnit->GetPosition());
					lastE = GGame->TickCount();
					return;
				}
			}
		}
	}
	if (useq->Enabled() && Q->IsReady() && GEntityList->Player()->IsValidTarget(target, Q->Range()) && !renw() && !GEntityList->Player()->IsDashing() && checkFuryMode(kSlotQ, target))
	{
		Q->CastOnPlayer();
	}
	float distance = SC.GetDistancePos(GEntityList->Player()->GetPosition(), target->GetPosition());
	if (usee->Enabled() && E->IsReady() && lastE == 0 && GEntityList->Player()->IsValidTarget(target, E->Range()) 
		&& (eDmg > target->GetHealth() || (((W->IsReady() && canBeOpWithQ(target->GetPosition()) && !rene()) ||
		(distance > SC.GetDistancePos(target->GetPosition(),SC.Extend(GEntityList->Player()->GetPosition(),target->GetPosition(),E->Range() )) - distance)))))
	{
		E->CastOnPosition(target->GetPosition());
		lastE = GGame->TickCount();
		return;
	}
	if (usee->Enabled() && checkFuryMode(kSlotE, target) && lastE != 0 &&
		(eDmg + GDamage->CalcPhysicalDamage(GEntityList->Player(), target, GEntityList->Player()->PhysicalDamage()) > target->GetHealth() ||
		(((W->IsReady() && canBeOpWithQ(target->GetPosition()) && !rene() ||
			(distance < SC.GetDistancePos(target->GetPosition(), SC.Extend(GEntityList->Player()->GetPosition(), target->GetPosition(), E->Range())) - distance) ||
			SC.GetDistancePos(GEntityList->Player()->GetPosition(), target->GetPosition()) > E->Range() - 100)))))
	{
		int time = GGame->TickCount() - lastE;
		if (time > 3600.0f || combodamage > target->GetHealth() || SC.GetDistancePos(GEntityList->Player()->GetPosition(), target->GetPosition()) > E->Range() - 100)
		{
			E->CastOnPosition(target->GetPosition());
			lastE = 0;
		}
	}
	if (GEntityList->Player()->HealthPercent() <= user->GetInteger())
	{
		R->CastOnPlayer();
	}
}

PLUGIN_EVENT(void) OnInterruptible(InterruptibleSpell const& Args)
{
	if (Args.Target != nullptr && W->IsReady() && GEntityList->Player()->IsValidTarget(Args.Target, W->Range() + GEntityList->Player()->BoundingRadius()))
		W->CastOnUnit(Args.Target);
}

PLUGIN_EVENT(void) OnGapCloser(GapCloserSpell const& Args)
{

}

PLUGIN_EVENT(void) OnOrbwalkAfterAttack(IUnit* Source, IUnit* Target)
{
	GGame->PrintChat("Step 1");
	if (Target->IsHero() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && checkFuryMode(kSlotW, Target) || GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		GGame->PrintChat("Step 2");
		float time = GGame->Time() - GEntityList->Player()->GetSpellRemainingCooldown(kSlotW);
		GGame->PrintChat("Step 3");
		if (useHydra->Enabled() &&
			(GEntityList->Player()->GetSpellRemainingCooldown(kSlotW) - abs(time) < 1 || time < -6 || GEntityList->Player()->HealthPercent() < 50))
		{
			GGame->PrintChat("Step 4");
			if (titHydra->IsOwned() && titHydra->IsReady())
				titHydra->CastOnPlayer();
			if (ravHydra->IsOwned() && ravHydra->IsReady())
				ravHydra->CastOnPlayer();
			GGame->PrintChat("Step 5");
		}
		GGame->PrintChat("Step 6");
		if (Target != nullptr && W->IsReady() && Target->IsHero() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && usew->Enabled() && checkFuryMode(kSlotW, Target))
		{
			W->CastOnPlayer();
			return;
		}
		if (Target != nullptr && Target->IsHero() && GOrbwalking->GetOrbwalkingMode() == kModeMixed && useCH->GetInteger() == 0)
		{
			if (Target != nullptr && W->IsReady())
			{
				W->CastOnPlayer();
				return;
			}
			if (Target != nullptr && Q->IsReady())
			{
				Q->CastOnPlayer();
				return;
			}
			if (Target != nullptr && E->IsReady() && GEntityList->Player()->IsValidTarget(Target, E->Range() - 100))
			{
				E->CastOnPosition(Target->GetPosition());
				return;
			}
		}
	}
}

PLUGIN_EVENT(void) OnOrbwalkBeforeAttack(IUnit* Target)
{
	if (Target != nullptr && W->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && Target->IsHero() && checkFuryMode(kSlotW, Target) && usew->Enabled())
	{
		if (GEntityList->Player()->GetMana() > 40 && !fury() || (Q->IsReady() && canBeOpWithQ(GEntityList->Player()->GetPosition())))
		{
			return;
		}

		W->CastOnPlayer();
		return;
	}
	if (Target != nullptr && W->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeMixed && usewH->Enabled() && Target->IsHero() && useCH->GetInteger() != 0)
	{
		W->CastOnPlayer();
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	if (GEntityList->Player()->IsDead())
		return;

	if (GGame->TickCount() - lastE > 4100)
		lastE = 0;

	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		ComboLogic();
		break;
	case kModeMixed:
		Harass();
		break;
	}
}

PLUGIN_EVENT(void) OnRender()
{
	for (auto enemy : GEntityList->GetAllHeros(false, true))
	{
		if (enemy != nullptr && enemy->IsVisible() && !enemy->IsDead())
		{
			ID.drawDmg(CalcComboDmg(enemy));
		}
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	// Initializes global interfaces for core access
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	GGame->PrintChat("SpiffyRenekton Loaded!");

	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->AddEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->AddEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OnOrbwalkAfterAttack);
	GEventManager->AddEventHandler(kEventOrbwalkBeforeAttack, OnOrbwalkBeforeAttack);

}

// Called when plugin is unloaded
PLUGIN_API void OnUnload()
{
	MainMenu->Remove();

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnOrbwalkAfterAttack);
	GEventManager->RemoveEventHandler(kEventOrbwalkBeforeAttack, OnOrbwalkBeforeAttack);

}