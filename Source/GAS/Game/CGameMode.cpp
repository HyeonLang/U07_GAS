#include "CGameMode.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EngineUtils.h"
#include "Engine/AssetManager.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameStateBase.h"
#include "Characters/CPlayer.h"
#include "Characters/CBot.h"
#include "Components/CAttributeComponent.h"
#include "Components/CActionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "CPlayerState.h"
#include "CSaveGame.h"
#include "CGameplayInterface.h"
#include "CSpawnBotDataAsset.h"
#include "GAS.h"

static TAutoConsoleVariable<bool> CVarSpawnBots(TEXT("Tore.SpawnBots"), true, TEXT("Enable spawn bots via cvar"), ECVF_Cheat);

ACGameMode::ACGameMode()
{
	SpawnTimerDelay = 2.f;
	CreditsPerKill = 20;

	MinimumPickupDistance = 2000;
	MaxPickupCount = 10;

	SlotName = "Game01";

	PlayerStateClass = ACPlayerState::StaticClass();
}

void ACGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	FString ParsedValue = UGameplayStatics::ParseOption(Options, "savegame");

	if (ParsedValue.Len() > 0)
	{
		SlotName = ParsedValue;
	}

	LoadSaveGame();
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

void ACGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ACPlayerState* PS = NewPlayer->GetPlayerState<ACPlayerState>();
	if (PS)
	{
		PS->LoadPlayerState(CurrentSaveGame);
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
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

		float RespawnDelay = 2.f;
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);

		Player->SetLifeSpan(3.f);
	}

	APawn* KillerPawn = Cast<APawn>(Killer);
	if (KillerPawn && KillerPawn != VictimActor)
	{
		ACPlayerState* PS = KillerPawn->GetPlayerState<ACPlayerState>();
		if (PS)
		{
			PS->AddCredits(CreditsPerKill);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("OnActorKilled, Victim : %s, Killer : %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));
}

void ACGameMode::RespawnPlayerElapsed(AController* Controller)
{
	if (ensure(Controller))
	{
		Controller->UnPossess();
		RestartPlayer(Controller);
	}
}

void ACGameMode::SpawnBotTimerElapsed()
{
	if (!CVarSpawnBots.GetValueOnGameThread())
	{
		//UE_LOG(LogTemp, Log, TEXT("Disabled bot spawning via CVar"));
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
		UE_LOG(LogTemp, Log, TEXT("Reached Maximum bot count. Skipping bot spawn"));
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
		if (SpawnBotDataTable)
		{
			TArray<FSpawnBotRow*> Rows;
			SpawnBotDataTable->GetAllRows("", Rows);

			int32 RandomIndex = FMath::RandRange(0, Rows.Num() - 1);
			FSpawnBotRow* SelectRow = Rows[RandomIndex];

			// 소프트 래퍼런스 데이터 불러오기
			UAssetManager* AssetManager = UAssetManager::GetIfValid();
			if (AssetManager)
			{
				LogOnScreen(this, "Loading SpawnBot Data Asset...", FColor::Yellow);

				FTransform Transform;
				Transform.SetLocation(Locations[0]);

				TArray<FName> Bundles;	// 빈 번들은 데이터 전부다 가져옴
				FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ACGameMode::OnDataAssetLoaded, SelectRow->BotDataAssetId, Transform); // 데이터 불러온 후 실행할 딜리게이트
				// Bind가 아닌 Create는 파라미터 가능 (파라미터 없고 다이나믹이 아닌 딜리게이트)
				AssetManager->LoadPrimaryAsset(SelectRow->BotDataAssetId, Bundles, Delegate);
			}

		}
	}
}

void ACGameMode::OnDataAssetLoaded(FPrimaryAssetId PrimaryAssetId, FTransform Transform)
{
	LogOnScreen(this, PrimaryAssetId.PrimaryAssetName.ToString() + " is Loaded.", FColor::Blue);


	// 불러온 데이터 사용하기위해 가져오기
	UAssetManager* AssetManager = UAssetManager::GetIfValid();
	if (AssetManager)
	{
		UCSpawnBotDataAsset* DataAsset;
		DataAsset = Cast<UCSpawnBotDataAsset>(AssetManager->GetPrimaryAssetObject(PrimaryAssetId));

		if (DataAsset)
		{
			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(DataAsset->BotClass, Transform);

			ACBot* NewBot = Cast<ACBot>(NewActor);

			if (NewBot)
			{
				NewBot->SetBodyColor(DataAsset->BotColor);

				UCActionComponent* ActionComp = Cast<UCActionComponent>(NewBot->GetComponentByClass(UCActionComponent::StaticClass()));

				if (ActionComp)
				{
					for (TSubclassOf<UCAction> Action : DataAsset->Actions)
					{
						ActionComp->AddAction(NewBot, Action);
					}
				}

				NewBot->FinishSpawning(Transform);
			}
		}
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

	TArray<FVector> UsedLocations;
	int32 SpawnCount = 0;

	while (SpawnCount < MaxPickupCount && EQSLocations.Num() > 0)
	{
		int32 RandomLocationIndex = FMath::RandRange(0, EQSLocations.Num() - 1);
		FVector SelectedLocation = EQSLocations[RandomLocationIndex];

		EQSLocations.RemoveAt(RandomLocationIndex);

		bool bValidLocation = true;
		for (FVector UsedLocation : UsedLocations)
		{
			float Distance = (SelectedLocation - UsedLocation).Size();

			if (Distance < MinimumPickupDistance)
			{
				DrawDebugSphere(GetWorld(), SelectedLocation, 50.f, 20, FColor::Red, false, 10.f);

				bValidLocation = false;
				break;
			}
		}

		if (!bValidLocation)
		{
			continue;
		}

		int32 RandomClassIndex = FMath::RandRange(0, PickupClassess.Num() - 1);;
		TSubclassOf<AActor> SelectedClass = PickupClassess[RandomClassIndex];

		GetWorld()->SpawnActor<AActor>(SelectedClass, SelectedLocation, FRotator::ZeroRotator);

		UsedLocations.Add(SelectedLocation);
		SpawnCount++;
	}
}

void ACGameMode::WriteSaveGame()
{
	//Credit
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		ACPlayerState* PS = Cast<ACPlayerState>(GameState->PlayerArray[i]);
		if (PS)
		{
			PS->SavePlayerState(CurrentSaveGame);
			break;
		}
	}

	//Actor
	CurrentSaveGame->SavedActors.Empty();

	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor->Implements<UCGameplayInterface>())
		{
			continue;
		}

		FActorSaveData ActorData;
		ActorData.ActorName = Actor->GetName();
		ActorData.Transform = Actor->GetTransform();

		FMemoryWriter MemWriter(ActorData.ByteData);
		FObjectAndNameAsStringProxyArchive Desc(MemWriter, true);
		Desc.ArIsSaveGame = true;
		Actor->Serialize(Desc);

		CurrentSaveGame->SavedActors.Add(ActorData);
	}


	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0);
}

void ACGameMode::LoadSaveGame()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		CurrentSaveGame = Cast<UCSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

		if (CurrentSaveGame == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Faild to load SaveGame Data."));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Loaded SaveGame Data."));

		//Actor
		for (FActorIterator It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor->Implements<UCGameplayInterface>())
			{
				continue;
			}

			for (FActorSaveData ActorData : CurrentSaveGame->SavedActors)
			{
				if (ActorData.ActorName == Actor->GetName())
				{
					Actor->SetActorTransform(ActorData.Transform);

					FMemoryReader MemReader(ActorData.ByteData);
					FObjectAndNameAsStringProxyArchive Desc(MemReader, true);
					Desc.ArIsSaveGame = true;
					Actor->Serialize(Desc);

					ICGameplayInterface::Execute_OnActorLoaded(Actor);

					break;
				}
			}
		}
	}
	else
	{
		CurrentSaveGame = Cast<UCSaveGame>(UGameplayStatics::CreateSaveGameObject(UCSaveGame::StaticClass()));
		UE_LOG(LogTemp, Log, TEXT("Created new SaveGame Data."));
	}
}
