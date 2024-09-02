#include "CCow.h"
#include "Characters/CPlayer.h"
#include "Components/CActionComponent.h"
#include "Actions/CAction.h"

void ACCow::Interact_Implementation(APawn* InstigatorPawn)
{
	if (ensure(AddClass) && InstigatorPawn)
	{
		ACPlayer* Player = Cast<ACPlayer>(InstigatorPawn);
		if (Player)
		{
			UCActionComponent* ActionComp = Cast<UCActionComponent>(Player->GetComponentByClass(UCActionComponent::StaticClass()));
			ActionComp->AddAction(Player, AddClass);

			HideAndCooldown();
		}
	}
}
