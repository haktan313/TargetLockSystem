

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface_LockTarget.h"
#include "Enemy_TargetLockSystem.generated.h"

UCLASS()
class TARGETLOCK_CPLUSPLUS_API AEnemy_TargetLockSystem : public ACharacter, public IInterface_LockTarget
{
	GENERATED_BODY()

public:

	AEnemy_TargetLockSystem();

public:	

	virtual void Tick(float DeltaTime) override;

	virtual bool TargetDead_Implementation() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	bool bIsTargetDead;
};
