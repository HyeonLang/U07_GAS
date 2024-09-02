#include "CChest.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"	// ���ø�����Ʈ

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

	bLidOpen = !bLidOpen; // Ŭ��鿡�� OnRep_LidOpen�Լ� ȣ��
	OnRep_LidOpen(); // ���������� ȣ��
}

void ACChest::OnRep_LidOpen()
{
	float CurrentPitch =  bLidOpen ? MaxPitch : 0.f;
	LidMesh->SetRelativeRotation(FRotator(CurrentPitch, 0, 0));
}

// ���ø�����Ʈ ����
void ACChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACChest, bLidOpen);
}