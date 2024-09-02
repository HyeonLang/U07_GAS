#pragma once

#include "CoreMinimal.h"
#include "Actions/CAction.h"
#include "CActionThornCounter.generated.h"

class UCAttributeComponent;

UCLASS()
class GAS_API UCActionThornCounter : public UCAction
{
	GENERATED_BODY()

public:
	UCActionThornCounter();

public:
	void StartAction_Implementation(AActor* Instigator) override;
	void StopAction_Implementation(AActor* Instigator) override;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Effect")
	void OnExecuteThornCounter(AActor* InstigatorActor, UCAttributeComponent* OwningComp, float NewHealth, float Delta);

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Effect")
	float ThornCounterDamagePer;
};
