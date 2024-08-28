#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "CGameMode.generated.h"

class UEnvQuery;

UCLASS()
class GAS_API ACGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACGameMode();

protected:
	virtual void StartPlay() override;

	// Kill
public:
	UFUNCTION(Exec)
	void KillAll();

	virtual void OnActorKilled(AActor* VictimActor, AActor* Killer);

	// Spawn Bots
protected:
	FTimerHandle TimerHandle_SpawnBots;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SpawnTimerDelay;

	UFUNCTION()
	void SpawnBotTimerElapsed();

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UEnvQuery* SpawnBotQuery;

	UFUNCTION()
	void OnSpawnBotQueryFinished(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<AActor> BotClass;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UCurveFloat* SpawnCurve;

	UFUNCTION()
	void RespawnPlayerElapsed(AController* Controller);



	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Credits")
	int32 CreditsPerKill;

	//Spawn Pickup
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	UEnvQuery* SpawnPickupQuery;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TArray<TSubclassOf<AActor>> PickupClassess;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float MinimumPickupDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	int32 MaxPickupCount;

	UFUNCTION()
	void OnSpawnPickupQueryFinished(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);
};
