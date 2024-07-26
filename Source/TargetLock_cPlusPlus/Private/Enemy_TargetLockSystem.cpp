


#include "Enemy_TargetLockSystem.h"


AEnemy_TargetLockSystem::AEnemy_TargetLockSystem()
{

	PrimaryActorTick.bCanEverTick = true;

	Tags.Add("Enemy");

}

void AEnemy_TargetLockSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AEnemy_TargetLockSystem::TargetDead_Implementation()
{
	return bIsTargetDead;
}
