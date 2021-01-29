// Fill out your copyright notice in the Description page of Project Settings.

#include "BoosterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "StratosCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UBoosterComponent::UBoosterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UBoosterComponent::Start()
{
	if (!IsDashing() && !bDashShootCooldown)
	{
		FVector CurrentVelocity = Character->GetCharacterMovement()->Velocity;
		if(CurrentVelocity.Size() <= 0)
		{
			CurrentVelocity = Character->GetActorForwardVector();
		}
		CurrentVelocity.Normalize();
		Direction = CurrentVelocity;
		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &UBoosterComponent::Stop, BoostTime, false);
		Character->GetCharacterMovement()->MaxWalkSpeed *= SpeedMultiplier;
		bIsDashing = true;
		bIsLocking = false;
	}
}

void UBoosterComponent::Stop()
{
	if (IsDashing())
	{
		GetWorld()->GetTimerManager().ClearTimer(DashTimer);
		Character->GetCharacterMovement()->MaxWalkSpeed /= SpeedMultiplier;
		bIsDashing = false;
		bIsLocking = false;
		bDashShootBlocking = false;
		bDashShootCooldown = true;
		GetWorld()->GetTimerManager().SetTimer(DashCooldownTimer, this, &UBoosterComponent::ClearDashCooldown, BoostCooldownTime, false);
	}
}

void UBoosterComponent::Shoot()
{
	if (Character != nullptr && Character->Enemy != nullptr)
	{
		if (IsDashing())
		{
			bIsLocking = true;
			if(!bDashShootBlocking)
			{
				MulticastDashShoot(Character->Enemy->GetActorLocation());
			}
			bDashShootBlocking = true;
		}
		else if (!bNormalShootBlocking)
		{
			bNormalShootBlocking = true;
			GetWorld()->GetTimerManager().SetTimer(NormalShootBlockingTimer, this, &UBoosterComponent::UnblockNormalShoot, NormalShootTime, false);
			MulticastNormalShoot(Character->Enemy->GetActorLocation());
		}
	}
}

bool UBoosterComponent::IsDashing() const
{
	return bIsDashing;
}

bool UBoosterComponent::IsLocking() const
{
	return bIsLocking;
}

// Called when the game starts
void UBoosterComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<AStratosCharacter>(GetOwner());
}

// Called every frame
void UBoosterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsDashing())
	{
		Character->AddMovementInput(Direction);
	}
	else if (Character->GetVelocity().Size() <= 0)
	{
		RotateCharacterToController(0.1f);
	}

	if (Character->GetVelocity().Z != 0.0f)
	{
		RotateControllerToEnemy(0.5);
		RotateCharacterToController(1.0f);
	}
	else if (IsLocking())
	{
		RotateControllerToEnemy(0.25f);
		RotateCharacterToController(0.5f);
	}

	if (IsShooting())
	{
		RotateCharacterToEnemy(1.0f);
	}
}

void UBoosterComponent::UnblockNormalShoot()
{
	bNormalShootBlocking = false;
}

void UBoosterComponent::RotateControllerToEnemy(float lerpValue)
{
	Character->GetController()->SetControlRotation(LerpControllerRotationToTarget(lerpValue));
}

void UBoosterComponent::RotateCharacterToEnemy(float lerpValue)
{
	Character->SetActorRotation(LerpControllerRotationToTarget(lerpValue));
}

void UBoosterComponent::RotateCharacterToController(float lerpValue)
{
	if (Character->GetController() != nullptr)
	{
		FRotator newRotator = FMath::Lerp(Character->GetActorRotation(), Character->GetController()->GetControlRotation(), lerpValue);
		Character->SetActorRotation(newRotator);
	}
}

FVector UBoosterComponent::GetTargetLocation() const
{
	if (Character != nullptr && Character->Enemy != nullptr)
	{
		return Character->Enemy->GetActorLocation();
	}
	return Character->GetActorLocation() + Character->GetActorForwardVector();
}

FRotator UBoosterComponent::LerpControllerRotationToTarget(float lerpValue) const
{
	FRotator NewRotation = Character->GetController()->GetControlRotation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), GetTargetLocation());
	FRotator LerpRotation = FMath::Lerp(NewRotation, LookAtRotation, lerpValue);
	NewRotation.Yaw = LerpRotation.Yaw;
	return NewRotation;
}

void UBoosterComponent::ClearDashCooldown() 
{
	bDashShootCooldown = false;
}

void UBoosterComponent::MulticastDashShoot_Implementation(FVector TargetLocation) 
{
	OnDashShootEvent.Broadcast(TargetLocation);
}

void UBoosterComponent::MulticastNormalShoot_Implementation(FVector TargetLocation) 
{
	OnNormalShootEvent.Broadcast(TargetLocation);
}

bool UBoosterComponent::IsShooting() const
{
	return bNormalShootBlocking;
}

bool UBoosterComponent::IsDashShooting() const
{
	return bDashShootBlocking;
}

void UBoosterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UBoosterComponent, bIsDashing);
	DOREPLIFETIME(UBoosterComponent, Direction);
	DOREPLIFETIME(UBoosterComponent, bIsLocking);
}
