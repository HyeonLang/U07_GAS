#include "CAttributeComponent.h"
#include "Game/CGameMode.h"

static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("Tore.DamageMultiplier"), 1.f, TEXT("Modify damage multiplier"), ECVF_Cheat);

UCAttributeComponent::UCAttributeComponent()
{
	MaxHealth = 100.f;
	Health = MaxHealth;

	MaxMana = 200.f;
	Mana = 0;
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
	Health = FMath::Clamp(Health += Delta, 0.f, MaxHealth);

	float ActualDelta = Health - PrevHealth;

	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta);
	}

	if (ActualDelta < 0.f && Health <= 0.f)
	{
		ACGameMode* GM = GetWorld()->GetAuthGameMode<ACGameMode>();
		if (GM)
		{
			GM->OnActorKilled(GetOwner(), InstigatorActor);
		}
	}

	return !FMath::IsNearlyZero(ActualDelta);
}

bool UCAttributeComponent::ApplyManaChange(AActor* InstigatorActor, float Delta)
{
	float PrevMana = Mana;
	Mana = FMath::Clamp(Mana += Delta, 0.f, MaxMana);

	float ActualDelta = Mana - PrevMana;

	if (OnManaChanged.IsBound())
	{
		OnManaChanged.Broadcast(InstigatorActor, this, Mana, ActualDelta);
	}

	return !FMath::IsNearlyZero(ActualDelta);
}

bool UCAttributeComponent::IsAlive() const
{
	return Health > 0.f;
}

bool UCAttributeComponent::IsFullHealth() const
{
	return FMath::IsNearlyEqual(Health, MaxHealth);
}

bool UCAttributeComponent::IsFullMana() const
{
	return FMath::IsNearlyEqual(Mana, MaxMana);
}

float UCAttributeComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UCAttributeComponent::GetHealth() const
{
	return Health;
}

float UCAttributeComponent::GetMaxMana() const
{
	return MaxMana;
}

float UCAttributeComponent::GetMana() const
{
	return Mana;
}

bool UCAttributeComponent::Kill(AActor* InstigatorActor)
{
	return ApplyHealthChange(InstigatorActor, -GetMaxHealth());
}
