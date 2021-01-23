// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/EngineTypes.h"
#include "Math/Vector.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BoosterComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STRATOS_API UBoosterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBoosterComponent();

	void Start();
	void Stop();
	void Shoot();

	UFUNCTION(BlueprintCallable)
	bool IsDashing() const;

	UFUNCTION(BlueprintCallable)
	bool IsLocking() const;

	UFUNCTION(BlueprintPure)
	FVector GetDashDirection() { return Direction; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NormalShootTime = 1.0f;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNormalShootDelegate, FVector, TargetLocation);
	UPROPERTY(BlueprintAssignable)
	FNormalShootDelegate OnNormalShootEvent;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDashShootDelegate, FVector, TargetLocation);
	UPROPERTY(BlueprintAssignable)
	FDashShootDelegate OnDashShootEvent;

	UFUNCTION(BlueprintCallable)
	bool IsShooting() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	void UnblockNormalShoot();
	void LerpControllerToEnemy(float lerpValue);
	void LerpCharacterToEnemy(float lerpValue);
	void LerpCharacterToController(float lerpValue);

	class ACharacter *Character;

	bool bNormalShootBlocking = false;
	bool bDashShootBlocking = false;
	FTimerHandle DashTimer;

	UPROPERTY(Replicated)
	bool bIsDashing = false;

	UPROPERTY(Replicated)
	bool bIsLocking = false;

	UPROPERTY(Replicated)
	FVector Direction;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastNormalShoot(FVector TargetLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDashShoot(FVector TargetLocation);

	FTimerHandle NormalShootBlockingTimer;
};
