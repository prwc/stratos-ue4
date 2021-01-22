// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StartosPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STRATOS_API AStartosPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, replicated)
	APawn *Enemy;
};
