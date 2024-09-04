#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CActionComponent.generated.h"

class UCAction;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UCActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCActionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Object류 클래스 리플리케이트 준비작업 1 재정의
	bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;


public:
	UFUNCTION(BlueprintCallable, Category = "Action")
	void AddAction(AActor* Instigator, TSubclassOf<UCAction> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Action")
	UCAction* GetAction(TSubclassOf<UCAction> ActionClass) const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	void RemoveAction(UCAction* ActionToRemove);

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool StartActionByName(AActor* Instigator, FName ActionName);

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool StopActionByName(AActor* Instigator, FName ActionName);

protected:
	// RPC 리플리케이트
	UFUNCTION(Reliable, Server)
	void ServerStartAction(AActor* Instigator, FName ActionName);

	UFUNCTION(Reliable, Server)
	void ServerStopAction(AActor* Instigator, FName ActionName);

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTag")
	FGameplayTagContainer ActiveGamePlayTags;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	TArray<TSubclassOf<UCAction>> DefaultActions;

	UPROPERTY(Replicated)
	TArray<UCAction*> Actions;

	
};
