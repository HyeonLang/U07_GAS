// Fill out your copyright notice in the Description page of Project Settings.


#include "CPlayerController.h"

void ACPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	OnPawnChanged.Broadcast(InPawn);
}