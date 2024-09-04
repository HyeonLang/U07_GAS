// Fill out your copyright notice in the Description page of Project Settings.


#include "CPickupBase_AddAction.h"
#include "Components/CActionComponent.h"
void ACPickupBase_AddAction::Interact_Implementation(APawn* InstigatorPawn)
{
	Super::Interact_Implementation(InstigatorPawn);

	if (!ensure(ActionToGrant && InstigatorPawn))
	{
		return;
	}

	UCActionComponent* ActionComp = Cast<UCActionComponent>(InstigatorPawn->GetComponentByClass(UCActionComponent::StaticClass()));

	if (ActionComp)
	{
		if (ActionComp->GetAction(ActionToGrant)) // 액션이 이미 있으면 리턴
		{
			FString Msg = FString::Printf(TEXT("%s is already learned"), *GetNameSafe(ActionToGrant));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, Msg);
			return;
		}
		ActionComp->AddAction(InstigatorPawn, ActionToGrant);
		HideAndCooldown();
	}
}