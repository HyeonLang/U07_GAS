
#include "CGameMode.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Characters/CBot.h"
#include "Characters/CPlayer.h"
#include "CPlayerState.h"
#include "Components/CAttributeComponent.h"

static TAutoConsoleVariable<bool> CVarSpawnBots(TEXT("Tore.SpawnBots"), true, TEXT("Enable spawn bots via cvar"), ECVF_Cheat);

ACGameMode::ACGameMode()
{
	SpawnTimerDelay = 2.0f;
	CreditsPerKill = 20;

	MinimumPickupDistance = 2000;
	MaxPickupCount = 10;

	PlayerStateClass = ACPlayerState::StaticClass();
}

void ACGameMode::StartPlay()
{
	Super::StartPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &ACGameMode::SpawnBotTimerElapsed, SpawnTimerDelay, true);
	
	if (ensure(PickupClassess.Num() > 0))
	{
		UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnPickupQuery, this, EEnvQueryRunMode::AllMatching, nullptr);
		if (ensure(QueryInstance))
		{
			QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ACGameMode::OnSpawnPickupQueryFinished);
		}
	}
}

void ACGameMode::KillAll()
{
	for (TActorIterator<ACBot> It(GetWorld()); It; ++It)
	{
		ACBot* Bot = *It;

		UCAttributeComponent* AttributeComp = UCAttributeComponent::GetAttributes(Bot);

		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			AttributeComp->Kill(this);
		}
	}
}

void ACGameMode::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	ACPlayer* Player = Cast<ACPlayer>(VictimActor);
	if (Player)
	{
		FTimerHandle TimerHandle_RespawnDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController());

		float RespawnDelay = 2.0f;
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);
	}

	APawn* KillerPawn = Cast<APawn>(Killer);
	if (KillerPawn)
	{
		ACPlayerState* PS = KillerPawn->GetPlayerState<ACPlayerState>();
		if (PS)
		{
			PS->AddCredits(CreditsPerKill);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("OnActorKilled, Victim : %s, Killer : %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));
}

void ACGameMode::SpawnBotTimerElapsed()
{
	if (!CVarSpawnBots.GetValueOnGameThread())
	{
		UE_LOG(LogTemp, Log, TEXT("Disabled bot spawning via CVar"));
		return;
	}

	int32 NrOfAliveBots = 0;
	for (TActorIterator<ACBot> It(GetWorld()); It; ++It)
	{
		ACBot* Bot = *It;

		UCAttributeComponent* AttributeComp = UCAttributeComponent::GetAttributes(Bot);
		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			NrOfAliveBots++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Found %i alive bots."), NrOfAliveBots);

	float MaxBotCount = 10.f;
	if (SpawnCurve)
	{
		MaxBotCount = SpawnCurve->GetFloatValue(GetWorld()->TimeSeconds);
	}
	if (NrOfAliveBots >= (int32)MaxBotCount)
	{
		UE_LOG(LogTemp, Log, TEXT("Reached Maximum bot count %i. Skipping bot Spawn"), NrOfAliveBots);
		return;
	}

	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);
	if (ensure(QueryInstance))
	{
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ACGameMode::OnSpawnBotQueryFinished);
	}
}

void ACGameMode::OnSpawnBotQueryFinished(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn bot EQS query failed!"));
		return;
	}

	

	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();
	if (Locations.IsValidIndex(0))
	{
		if (!ensure(BotClass))
		{
			UE_LOG(LogTemp, Error, TEXT("Bot Class Ref is not set"));
			return;
		}
		GetWorld()->SpawnActor<AActor>(BotClass, Locations[0], FRotator::ZeroRotator);
		DrawDebugSphere(GetWorld(), Locations[0], 50.f, 20, FColor::Blue, false, 60.f);
	}
}

void ACGameMode::RespawnPlayerElapsed(AController* Controller)
{
	if (ensure(Controller))
	{
		Controller->UnPossess();
		RestartPlayer(Controller);
	}
}

void ACGameMode::OnSpawnPickupQueryFinished(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn powerup query failed"));
		return;
	}

	TArray<FVector> EQSLocations = QueryInstance->GetResultsAsLocations();


	int32 SpawnCount = 0;
	while(SpawnCount < MaxPickupCount && EQSLocations.Num() > 0)
	{
		int32 RandonLocationIndex = FMath::RandRange(0, EQSLocations.Num() - 1);
		FVector SelectedLocation =  EQSLocations[RandonLocationIndex];

		SpawnCount++;
		EQSLocations.RemoveAt(RandonLocationIndex);
	}
}
