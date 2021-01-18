// Fill out your copyright notice in the Description page of Project Settings.

#include "BoosterComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	if (!IsDashing())
	{
		FVector CurrentVelocity = Character->GetCharacterMovement()->Velocity;
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
	}
}

void UBoosterComponent::Shoot()
{
	if (IsDashing())
	{
		bIsLocking = true;
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
	Character = Cast<ACharacter>(GetOwner());
}

// Called every frame
void UBoosterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
