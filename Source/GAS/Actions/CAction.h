#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "CAction.generated.h"

class UCActionComponent;

UCLASS(Blueprintable)
class GAS_API UCAction : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StopAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsRunning() const;

	UWorld* GetWorld() const override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Action")
	UCActionComponent* GetOwingComponent() const;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FName ActionName;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTag")
	FGameplayTagContainer GrantTags;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTag")
	FGameplayTagContainer BlockedTags;

protected:
	bool bIsRunning;
};
