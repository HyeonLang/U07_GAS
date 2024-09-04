#include "CAttributeComponent.h"
#include "Game/CGameMode.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("Tore.DamageMultiplier"), 1.f, TEXT("Modify damage multiplier"), ECVF_Cheat);

UCAttributeComponent::UCAttributeComponent()
{
	MaxHealth = 100.f;
	Health = MaxHealth;

	MaxRage = 100.f;
	Rage = 0.f;

	// ������Ʈ ���ø�����Ʈ
	SetIsReplicatedByDefault(true); //������ ���� ���ø�����Ʈ Set
}


void UCAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

UCAttributeComponent* UCAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		return Cast<UCAttributeComponent>(FromActor->GetComponentByClass(UCAttributeComponent::StaticClass()));
	}

	return nullptr;
}

bool UCAttributeComponent::IsActorAlive(AActor* Actor)
{
	UCAttributeComponent* AttributeComp = GetAttributes(Actor);
	if (AttributeComp)
	{
		return GetAttributes(Actor)->IsAlive();
	}
	return false;
}

bool UCAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	if (!GetOwner()->CanBeDamaged() && Delta < 0.f)
	{
		return false;
	}


	if (Delta < 0.f)
	{
		float DamageMultiplier = CVarDamageMultiplier.GetValueOnGameThread();

		Delta *= DamageMultiplier;
	}

	float PrevHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.f, MaxHealth); // ü�°��
	float ActualDelta = NewHealth - PrevHealth;

	if (GetOwner()->HasAuthority()) // ����������
	{
		Health = NewHealth;

		if (!FMath::IsNearlyZero(ActualDelta))
		{
			NetMulticastHealthChanged(InstigatorActor, Health, ActualDelta);
		}

		if (ActualDelta < 0.f && Health <= 0.f) // �������
		{
			ACGameMode* GM = GetWorld()->GetAuthGameMode<ACGameMode>();
			if (GM)
			{
				GM->OnActorKilled(GetOwner(), InstigatorActor);
			}
		}
	}

	return !FMath::IsNearlyZero(ActualDelta);
}

void UCAttributeComponent::NetMulticastHealthChanged_Implementation(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(InstigatorActor, this, NewHealth, Delta);
}

bool UCAttributeComponent::ApplyRageChange(AActor* InstigatorActor, float Delta)
{
	float PrevRage = Rage;
	Rage = FMath::Clamp(Rage + Delta, 0.f, MaxRage); // Rage���
	float ActualDelta = Rage - PrevRage;

	if (!FMath::IsNearlyZero(ActualDelta))
	{
		NetMulticastRageChanged(InstigatorActor, Rage, ActualDelta);
	}

	return !FMath::IsNearlyZero(ActualDelta);
}

void UCAttributeComponent::NetMulticastRageChanged_Implementation(AActor* InstigatorActor, float NewRage, float Delta)
{
	OnRageChanged.Broadcast(InstigatorActor, this, NewRage, Delta);
}

bool UCAttributeComponent::IsAlive() const
{
	return Health > 0.f;
}

bool UCAttributeComponent::IsFullHealth() const
{
	return FMath::IsNearlyEqual(Health, MaxHealth);
}

float UCAttributeComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UCAttributeComponent::GetHealth() const
{
	return Health;
}

float UCAttributeComponent::GetMaxRage() const
{
	return MaxRage;
}

float UCAttributeComponent::GetRage() const
{
	return Rage;
}

bool UCAttributeComponent::Kill(AActor* InstigatorActor)
{
	return ApplyHealthChange(InstigatorActor, -GetMaxHealth());
}


void UCAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCAttributeComponent, Health);
	DOREPLIFETIME(UCAttributeComponent, MaxHealth);

	//	DOREPLIFETIME_CONDITION(UCAttributeComponent, MaxHealth, COND_InitialOnly);
	// ���� �ѹ� ���ø�����Ʈ

}
