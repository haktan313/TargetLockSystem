

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interface_LockTarget.generated.h"


UINTERFACE(MinimalAPI)
class UInterface_LockTarget : public UInterface
{
	GENERATED_BODY()
};


class TARGETLOCK_CPLUSPLUS_API IInterface_LockTarget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "LockTarget")
	bool TargetDead();
};
