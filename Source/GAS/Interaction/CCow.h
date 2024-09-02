#pragma once

#include "CoreMinimal.h"
#include "Interaction/CPickupBase.h"
#include "CCow.generated.h"

class UCAction;

UCLASS()
class GAS_API ACCow : public ACPickupBase
{
	GENERATED_BODY()
public:
	void Interact_Implementation(APawn* InstigatorPawn) override;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UCAction> AddClass;
};
