#include "CActionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Actions/CAction.h"
#include "Gas.h"

UCActionComponent::UCActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 컴포넌트 리플리케이트
	SetIsReplicatedByDefault(true);
}


void UCActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 서버 캐릭터들에서만 액션 등록 후 리플리케이트
	if (GetOwner()->HasAuthority()) // 스폰 위치가 Authority
	{
		for (TSubclassOf<UCAction> ActionClass : DefaultActions)
		{
			AddAction(GetOwner(), ActionClass);
		}
	}
	
}


void UCActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (UCAction* Action : Actions)
	{
		FString ActionMsg = FString::Printf(TEXT("[%s] Action : %s"), 
			*GetNameSafe(GetOwner()), *GetNameSafe(Action));

		FColor TextColor = Action->IsRunning() ? FColor::Blue : FColor::White;

		LogOnScreen(this, ActionMsg, TextColor, 0.f);
	}
}

bool UCActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bChangedSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UCAction* Action : Actions)
	{
		if (Action)
		{
			bChangedSomething |= Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);
		}
	}

	return bChangedSomething;
}


void UCActionComponent::AddAction(AActor* Instigator, TSubclassOf<UCAction> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client attempting to AddAction. [Class : %s]"), *GetNameSafe(ActionClass));
		return;
	}

	UCAction* NewAction = NewObject<UCAction>(GetOwner(), ActionClass);

	if (ensure(NewAction))
	{
		NewAction->SetOwningComponent(this);
		Actions.Add(NewAction);
		if (NewAction->bAutoStart && NewAction->CanStart(Instigator))
		{
			NewAction->StartAction(Instigator);
		}
	}
}

UCAction* UCActionComponent::GetAction(TSubclassOf<UCAction> ActionClass) const
{
	for (UCAction* Action : Actions)
	{
	}

	return nullptr;
}

void UCActionComponent::RemoveAction(UCAction* ActionToRemove)
{
	if (!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	
	Actions.Remove(ActionToRemove);
}

bool UCActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	for (UCAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			if (!Action->CanStart(Instigator))
			{
				FString Message = FString::Printf(TEXT("Faild to run : %s"), *ActionName.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, Message);
				continue;
			}

			// 1.로컬에서 실행시키고 2. 서버로 보냄 (조작감)
			if (!GetOwner()->HasAuthority())
			{
				// 서버에서 실행되지만 서버에서는 여기로 들어오지 않음.
				ServerStartAction(Instigator, ActionName);
			}

			Action->StartAction(Instigator);
			return true;
			
		}
	}
	return false;
}

void UCActionComponent::ServerStartAction_Implementation(AActor* Instigator, FName ActionName)
{
	StartActionByName(Instigator, ActionName);
}

bool UCActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	for (UCAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			if (Action->IsRunning())
			{
				// 1.로컬에서 실행시키고 2. 서버로 보냄 (조작감)
				if (!GetOwner()->HasAuthority())
				{
					// 서버에서 실행되지만 서버에서는 여기로 들어오지 않음.
					ServerStopAction(Instigator, ActionName);
				}

				Action->StopAction(Instigator);
				return true;
			}
		}
	}
	return false;
}

void UCActionComponent::ServerStopAction_Implementation(AActor* Instigator, FName ActionName)
{
	StopActionByName(Instigator, ActionName);
}

void UCActionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCActionComponent, Actions);
}


