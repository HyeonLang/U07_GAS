#include "CChest.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"	// 리플리케이트

ACChest::ACChest()
{
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseMesh");
	RootComponent = BaseMesh;

	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>("LidMesh");
	LidMesh->SetupAttachment(BaseMesh);

	MaxPitch = 110.f;

	SetReplicates(true);
}

void ACChest::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACChest::Interact_Implementation(APawn* InstigatorPawn)
{

	bLidOpen = !bLidOpen; // 클라들에서 OnRep_LidOpen함수 호출
	OnRep_LidOpen(); // 서버에서만 호출
}

void ACChest::OnRep_LidOpen()
{
	float CurrentPitch =  bLidOpen ? MaxPitch : 0.f;
	LidMesh->SetRelativeRotation(FRotator(CurrentPitch, 0, 0));
}

// 리플리케이트 선언
void ACChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACChest, bLidOpen);
}