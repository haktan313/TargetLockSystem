


#include "PlayerCharacter_TargetLock.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy_TargetLockSystem.h"
#include "Interface_LockTarget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

APlayerCharacter_TargetLock::APlayerCharacter_TargetLock()
{

	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 500.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	SphereCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereCollisionComponent->SetupAttachment(RootComponent);
	SphereCollisionComponent->SetSphereRadius(500.0f);
	SphereCollisionComponent->SetCollisionProfileName("OverlapAllDynamic");
	SphereCollisionComponent->bHiddenInGame = false;
	SphereCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter_TargetLock::OnOverlapBegin);
	SphereCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter_TargetLock::OnOverlapEnd);

}

void APlayerCharacter_TargetLock::BeginPlay()
{
	Super::BeginPlay();

	if(APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayer = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			EnhancedInputLocalPlayer->AddMappingContext(InputMappingContext,0);
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PlayerCharacter_TargetLock BeginPlay"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("For LockOn and UnLock press 'TAB'"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("For SwitchLockOnLeft press 'Q'"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("For SwitchLockOnRight press 'E'"));
}
	
void APlayerCharacter_TargetLock::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered, this, &APlayerCharacter_TargetLock::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter_TargetLock::Look);
	EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &APlayerCharacter_TargetLock::LockOn);
	EnhancedInputComponent->BindAction(SwitchLockOnLeftAction, ETriggerEvent::Started, this, &APlayerCharacter_TargetLock::SwitchLockOnLeft);
	EnhancedInputComponent->BindAction(SwitchLockOnRightAction, ETriggerEvent::Started, this, &APlayerCharacter_TargetLock::SwitchLockOnRight);
	EnhancedInputComponent->BindAction(killTargetAction, ETriggerEvent::Started, this, &APlayerCharacter_TargetLock::KillTarget);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

}

void APlayerCharacter_TargetLock::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Enemy"))
	{
		enemysInRange.Add(OtherActor);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enemy in Range"));
	}
}

void APlayerCharacter_TargetLock::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Enemy"))
	{
		enemysInRange.Remove(OtherActor);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enemy out of Range"));
	}
	//if (OtherActor == targetLockedActor)
	//{
	//	bIsLockedOn = false;
	//	targetLockedActor = nullptr;
	//}
}

void APlayerCharacter_TargetLock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsLockedOn)
	{
		if (targetLockedActor)
		{
			if (IInterface_LockTarget::Execute_TargetDead(targetLockedActor))
			{
				AActor* newTarget = FindNearestActor(GetActorLocation(), enemysInRange);
				if (newTarget)
				{
					targetLockedActor = newTarget;
				}
				else
				{
					bIsLockedOn = false;
					targetLockedActor = nullptr;
				}
			}
			else 
			{
				SetControlRotation();
			}
		}
		else
		{
			bIsLockedOn = false;
			targetLockedActor = nullptr;
		}

	}
	else if (!bIsLockedOn)
	{

		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;

	}
}

void APlayerCharacter_TargetLock::Move(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector forwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(forwardDirection, InputVector.Y);
	AddMovementInput(rightDirection, InputVector.X);
}

void APlayerCharacter_TargetLock::Look(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();

	AddControllerYawInput(InputVector.X);
	AddControllerPitchInput(InputVector.Y);
}

void APlayerCharacter_TargetLock::LockOn()
{
	if (bIsLockedOn)
	{
		bIsLockedOn = false;
		targetLockedActor = nullptr;
		bUseControllerRotationYaw = false;
	}
	else
	{
		AActor* newTarget = FindNearestActor(GetActorLocation(), enemysInRange);
		if (newTarget)
		{
			bIsLockedOn = true;
			targetLockedActor = newTarget;
			bUseControllerRotationYaw = true;
		}
		else
		{
			bIsLockedOn = false;
			targetLockedActor = nullptr;
		}
	}
}

AActor* APlayerCharacter_TargetLock::FindNearestActor(FVector origin, TArray<AActor*> actorsToCheck)
{
	closestActor = nullptr;
	float closesDistance = FLT_MAX;

	for (AActor* actor : actorsToCheck)
	{
		float distance = FVector::Dist(actor->GetActorLocation(), origin);
		if (distance < closesDistance)
		{
			closesDistance = distance;
			closestActor = actor;
		}
	}
	return closestActor;
}

void APlayerCharacter_TargetLock::SwitchLockOnLeft()
{
	if (bIsLockedOn)
	{
		closestActor = nullptr;
		enemysInRangeLeft.Empty();
		for (AActor* enemyInRangeleft : enemysInRange)
		{
			if (enemyInRangeleft != targetLockedActor)
			{
				FVector dot2 = UKismetMathLibrary::Normal(targetLockedActor->GetActorLocation() - enemyInRangeleft->GetActorLocation());
				FVector dot1 = CameraComponent->GetRightVector();
				float dot = UKismetMathLibrary::Dot_VectorVector(dot1, dot2);
				if (dot > 0)
				{
					enemysInRangeLeft.Add(enemyInRangeleft);
				}
			}
		}
		if (enemysInRangeLeft.Num() > 0 && enemysInRangeLeft[0])
		{
			closestActor = enemysInRangeLeft[0];
			for (AActor* newTargetLeft : enemysInRangeLeft)
			{
				if (newTargetLeft != targetLockedActor)
				{
					float distance1 = FVector::Dist(newTargetLeft->GetActorLocation(), targetLockedActor->GetActorLocation());
					float distance2 = FVector::Dist(closestActor->GetActorLocation(), targetLockedActor->GetActorLocation());
					if (distance1 <= distance2)
					{
						newTargetLeft = closestActor;
					}
				}
			}
			targetLockedActor = closestActor;
		}
	}
}

void APlayerCharacter_TargetLock::SwitchLockOnRight()
{
	if (bIsLockedOn)
	{
		closestActor = nullptr;
		enemysInRangeRight.Empty();
		for (AActor* enemyInRangeRight : enemysInRange)
		{
			if (enemyInRangeRight != targetLockedActor)
			{
				FVector dot2 = UKismetMathLibrary::Normal(targetLockedActor->GetActorLocation() - enemyInRangeRight->GetActorLocation());
				FVector dot1 = CameraComponent->GetRightVector();
				float dot = UKismetMathLibrary::Dot_VectorVector(dot1, dot2);
				if (dot <= 0)
				{
					enemysInRangeRight.Add(enemyInRangeRight);
				}
			}
		}
		if (enemysInRangeRight.Num() > 0 && enemysInRangeRight[0])
		{
			closestActor = enemysInRangeRight[0];
			for (AActor* newTargetRight : enemysInRangeRight)
			{
				if (newTargetRight != targetLockedActor)
				{
					float distance1 = FVector::Dist(newTargetRight->GetActorLocation(), targetLockedActor->GetActorLocation());
					float distance2 = FVector::Dist(closestActor->GetActorLocation(), targetLockedActor->GetActorLocation());
					if (distance1 <= distance2)
					{
						newTargetRight = closestActor;
					}
				}
			}
			targetLockedActor = closestActor;
		}
	}
}

void APlayerCharacter_TargetLock::SetControlRotation()
{
	FVector targetLookAt = targetLockedActor->GetActorLocation() - FVector(0.0f, 0.0f, 100.0f);
	FRotator lookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), targetLookAt);

	FRotator controlRotation = GetController()->GetControlRotation();
	FRotator rinterp = UKismetMathLibrary::RInterpTo(controlRotation, lookAtRotation, GetWorld()->GetDeltaSeconds(), 5.0f);

	//FRotator newRotation = FRotator(controlRotation.Roll, rinterp.Pitch, rinterp.Yaw);
	//GetController()->SetControlRotation(newRotation);

	GetController()->SetControlRotation(rinterp);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void APlayerCharacter_TargetLock::KillTarget()
{
	AEnemy_TargetLockSystem* target = Cast<AEnemy_TargetLockSystem>(targetLockedActor);
	if (target)
	{
		target->bIsTargetDead = true;
	}
}




