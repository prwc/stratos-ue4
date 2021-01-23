// Fill out your copyright notice in the Description page of Project Settings.

#include "BoosterComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Controller.h"
#include "StartosPlayerController.h"
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
		bDashShootBlocking = false;
	}
}

void UBoosterComponent::Shoot()
{
	AStartosPlayerController *StartosController = Cast<AStartosPlayerController>(Character->GetController());
	if (StartosController != nullptr && StartosController->Enemy != nullptr)
	{
		if (IsDashing())
		{
			bIsLocking = true;
			bDashShootBlocking = true;
			MulticastDashShoot(StartosController->Enemy->GetActorLocation());
		}
		else if (!bNormalShootBlocking)
		{
			bNormalShootBlocking = true;
			GetWorld()->GetTimerManager().SetTimer(NormalShootBlockingTimer, this, &UBoosterComponent::UnblockNormalShoot, NormalShootTime, false);
			MulticastNormalShoot(StartosController->Enemy->GetActorLocation());
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
	Character = Cast<ACharacter>(GetOwner());
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
		LerpCharacterToController(0.1f);
	}

	if (Character->GetVelocity().Z != 0.0f)
	{
		LerpControllerToEnemy(1.0f);
		LerpCharacterToController(1.0f);
	}
	else if (IsLocking())
	{
		LerpControllerToEnemy(0.5f);
		LerpCharacterToController(1.0f);
	}

	if (IsShooting())
	{
		LerpCharacterToEnemy(1.0f);
	}
}

void UBoosterComponent::UnblockNormalShoot()
{
	bNormalShootBlocking = false;
}

void UBoosterComponent::LerpControllerToEnemy(float lerpValue)
{
	AStartosPlayerController *StartosController = Cast<AStartosPlayerController>(Character->GetController());
	if (StartosController != nullptr && StartosController->Enemy != nullptr)
	{
		APawn *Enemy = StartosController->Enemy;
		FRotator NewRotation = StartosController->GetControlRotation();
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), Enemy->GetActorLocation());
		FRotator LerpRotation = FMath::Lerp(NewRotation, LookAtRotation, lerpValue);
		NewRotation.Yaw = LerpRotation.Yaw;
		StartosController->SetControlRotation(NewRotation);
	}
}

void UBoosterComponent::LerpCharacterToEnemy(float lerpValue)
{
	AStartosPlayerController *StartosController = Cast<AStartosPlayerController>(Character->GetController());
	if (StartosController != nullptr && StartosController->Enemy != nullptr)
	{
		APawn *Enemy = StartosController->Enemy;
		FRotator NewRotation = StartosController->GetControlRotation();
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), Enemy->GetActorLocation());
		FRotator LerpRotation = FMath::Lerp(NewRotation, LookAtRotation, lerpValue);
		NewRotation.Yaw = LerpRotation.Yaw;
		Character->SetActorRotation(NewRotation);
	}
}

void UBoosterComponent::LerpCharacterToController(float lerpValue)
{
	if (Character->GetController() != nullptr)
	{
		FRotator newRotator = FMath::Lerp(Character->GetActorRotation(), Character->GetController()->GetControlRotation(), lerpValue);
		Character->SetActorRotation(newRotator);
	}
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

void UBoosterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UBoosterComponent, bIsDashing);
	DOREPLIFETIME(UBoosterComponent, Direction);
	DOREPLIFETIME(UBoosterComponent, bIsLocking);
}
