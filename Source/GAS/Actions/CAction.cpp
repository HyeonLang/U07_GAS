#include "CAction.h"
#include "Net/UnrealNetwork.h"
#include "Components/CActionComponent.h"
#include "GAS.h"

bool UCAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	UCActionComponent* Comp = GetOwningComponent();

	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}

void UCAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Started : %s"), *GetNameSafe(this));

	UCActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.AppendTags(GrantTags);
	
	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;

	if (Comp->GetOwnerRole() == ROLE_Authority) // 서버라면 서버에서만 실행
	{
		TimeStarted = GetWorld()->TimeSeconds;
	}

	Comp->OnActionStarted.Broadcast(Comp, this);
}

void UCAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Stopped : %s"), *GetNameSafe(this));

	UCActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.RemoveTags(GrantTags);

	RepData.bIsRunning = false;
	RepData.Instigator = Instigator;

	Comp->OnActionStopped.Broadcast(Comp, this);
}

UWorld* UCAction::GetWorld() const
{
	AActor* Actor = Cast<AActor>(GetOuter());
	if (Actor)
	{
		return Actor->GetWorld();
	}

	return nullptr;
}

UCActionComponent* UCAction::GetOwningComponent() const
{
	return ActionComp;
}

void UCAction::SetOwningComponent(UCActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
}

void UCAction::OnRep_RepData()
{
	if (RepData.bIsRunning)
	{
		StartAction(RepData.Instigator);
	}
	else
	{
		StopAction(RepData.Instigator);
	}
}

bool UCAction::IsRunning() const
{
	return RepData.bIsRunning;
}

void UCAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCAction, RepData);
	DOREPLIFETIME(UCAction, ActionComp);
	DOREPLIFETIME(UCAction, TimeStarted);
}