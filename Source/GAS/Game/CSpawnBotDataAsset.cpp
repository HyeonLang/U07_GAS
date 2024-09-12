
#include "CSpawnBotDataAsset.h"

UCSpawnBotDataAsset::UCSpawnBotDataAsset()
{
	BotColor = FLinearColor::White;
}

FPrimaryAssetId UCSpawnBotDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("Bot"), GetFName());
}
