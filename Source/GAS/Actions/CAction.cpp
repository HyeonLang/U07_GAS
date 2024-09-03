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

	UCActionComponent* ActionComp = GetOwningComponent();

	if (ActionComp->ActiveGamePlayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}

void UCAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Started : %s"), *GetNameSafe(this));
	

	UCActionComponent* ActionComp = GetOwningComponent();
	ActionComp->ActiveGamePlayTags.AppendTags(GrantTags);

	// 서버일 경우
	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;
}

void UCAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Stopped : %s"), *GetNameSafe(this));
	

	UCActionComponent* ActionComp = GetOwningComponent();
	ActionComp->ActiveGamePlayTags.RemoveTags(GrantTags);

	// 서버일 경우
	RepData.bIsRunning = false;
	RepData.Instigator = Instigator;
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
	return OwningActionComp;
}

void UCAction::SetOwningComponent(UCActionComponent* NewActionComp)
{
	OwningActionComp = NewActionComp;
}

// 다른 클라의 자기만 호출 : RepData가 서버와 다른 경우
void UCAction::OnRep_RepData()
{
	if (RepData.bIsRunning)
	{
		// 로컬 인스티게이터가 아닌 서버 인스티게이터를 넘겨야함
		// 서버에서 StartAction내의 인스티게이터를 넣어준것을 다시 로컬에 넘겨줌
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
	DOREPLIFETIME(UCAction, OwningActionComp);
}

