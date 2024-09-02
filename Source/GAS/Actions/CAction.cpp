#include "CAction.h"
#include "Components/CActionComponent.h"
#include "Components/CAttributeComponent.h"

UCAction::UCAction()
{
	CostMana = 0.f;
}

bool UCAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	if (CostMana > UCAttributeComponent::GetAttributes(Instigator)->GetMana())
	{
		return false;
	}

	UCActionComponent* ActionComp = GetOwningComponent();

	if (ActionComp->ActiveGamePlayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}

void UCAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Running : %s"), *GetNameSafe(this));

	UCActionComponent* ActionComp = GetOwningComponent();
	ActionComp->ActiveGamePlayTags.AppendTags(GrantTags);
	if (CostMana > 0)
	{
		UCAttributeComponent::GetAttributes(Instigator)->ApplyManaChange(Instigator, -CostMana);
	}

	bIsRunning = true;
}

void UCAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Stopped : %s"), *GetNameSafe(this));

	UCActionComponent* ActionComp = GetOwningComponent();
	ActionComp->ActiveGamePlayTags.RemoveTags(GrantTags);

	bIsRunning = false;
}


UWorld* UCAction::GetWorld() const
{
	UCActionComponent* ActionComp = Cast<UCActionComponent>(GetOuter());
	if (ActionComp)
	{
		return ActionComp->GetWorld();
	}
	return nullptr;
}

UCActionComponent* UCAction::GetOwningComponent() const
{
	return Cast<UCActionComponent>(GetOuter());
}

bool UCAction::IsRunning() const
{
	return bIsRunning;
}

