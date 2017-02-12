#include "SpiffyC.h"

Vec3& SpiffyCommon::Extend(Vec3 from, Vec3 to, float distance)
{
	return from + distance * (to - from).VectorNormalize();
}

bool SpiffyCommon::CanHarass()
{
	return false;
}

bool SpiffyCommon::ShouldWait()
{
	return false;
}

float SpiffyCommon::GetEchoLudenDamage(IUnit * target)
{
	return 0.0f;
}

bool SpiffyCommon::CanHitSkillShot(IUnit * target, Vec3 Start, Vec3 End, ISpellData * SData)
{
	return false;
}

float SpiffyCommon::GetKsDamage(IUnit * t, ISpell2 * QWER, bool includeIncomingDamage)
{
	return 0.0f;
}

bool SpiffyCommon::ValidUlt(IUnit * target)
{
	return false;
}

bool SpiffyCommon::CanMove(IUnit * target)
{
	return false;
}

float SpiffyCommon::GetDistancePos(Vec3 from, Vec3 to)
{
	float x1 = from.x;
	float x2 = to.x;
	float y1 = from.y;
	float y2 = to.y;
	float z1 = from.z;
	float z2 = to.z;
	return static_cast<float>(sqrt(pow((x2 - x1), 2.0) + pow((y2 - y1), 2.0) + pow((z2 - z1), 2.0)));
}

float SpiffyCommon::GetDistance(IUnit * player, IUnit * target)
{
	auto x1 = player->GetPosition().x;
	auto x2 = target->GetPosition().x;
	auto y1 = player->GetPosition().y;
	auto y2 = target->GetPosition().y;
	auto z1 = player->GetPosition().z;
	auto z2 = target->GetPosition().z;
	return static_cast<float>(sqrt(pow((x2 - x1), 2.0) + pow((y2 - y1), 2.0) + pow((z2 - z1), 2.0)));
}

bool SpiffyCommon::IsUnderTurret(IUnit * Source, bool CheckAllyTurrets, bool CheckEnemyTurrets)
{
	for (auto turret : GEntityList->GetAllTurrets(CheckAllyTurrets, CheckEnemyTurrets))
	{
		if (Source->IsValidTarget(turret, 950.0f))
			return true;
	}

	return false;
}

bool SpiffyCommon::PosUnderTurret(Vec3 Pos, bool CheckAllyTurrets, bool CheckEnemyTurrets)
{
	for (auto turret : GEntityList->GetAllTurrets(CheckAllyTurrets, CheckEnemyTurrets))
	{
		if (SpiffyCommon::GetDistancePos(turret->GetPosition(),Pos) <= 950)
			return true;
	}

	return false;
}

int SpiffyCommon::CountEnemiesInRange(float range)
{
	int enemies = 0;
	for (auto enemy : GEntityList->GetAllHeros(false, true))
	{
		if (enemy != nullptr && GEntityList->Player()->IsValidTarget(enemy, range))
			enemies++;
	}
	return enemies;
}

int SpiffyCommon::CountMinionsInRange(float range)
{
	int minions = 0;
	for (auto enemy : GEntityList->GetAllMinions(false, true,false))
	{
		if (enemy != nullptr && GEntityList->Player()->IsValidTarget(enemy, range))
			minions++;
	}
	return minions;
}

int SpiffyCommon::CountMinionsInRange(Vec3 pos, float range)
{
	auto mingons = GEntityList->GetAllMinions(false, true, false);
	auto mingonsInRange = 0;
	for (auto mingon : mingons)
	{
		//counts enemies checking if they are enemy heroes and are within radius parameter
		if (mingon != nullptr && mingon->IsValidTarget() && !mingon->IsDead())
		{
			auto mingonDistance = (mingon->GetPosition() - pos).Length();
			if (mingonDistance < range)
			{
				mingonsInRange++;
			}
		}
	}
	return mingonsInRange;
}

int SpiffyCommon::CountKillableMinionsInRange(float range)
{
	int minionCount = 0;
	for (auto minion : GEntityList->GetAllMinions(false, true, false))
	{
		if (GEntityList->Player()->IsValidTarget(minion, range) && GDamage->CalcPhysicalDamage(GEntityList->Player(), minion, GDamage->GetSpellDamage(GEntityList->Player(), minion, kSlotQ)) > minion->GetHealth())
		{
			minionCount++;
		}
	}
	return minionCount;
}

int SpiffyCommon::CountEnemiesInRangePos(float range, Vec3 pos)
{
	return 0;
}

int SpiffyCommon::AutosToKill(IUnit * target)
{
	return target->GetHealth() / GDamage->CalcPhysicalDamage(GEntityList->Player(),target,GEntityList->Player()->PhysicalDamage());
}


void SpiffyCommon::PotionManager()
{
}
