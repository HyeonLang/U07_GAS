#include "CAttributeComponent.h"

UCAttributeComponent::UCAttributeComponent()
{
	MaxHealth = 100.f;
	Health = MaxHealth;
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
	float PrevHealth = Health;
	Health = FMath::Clamp(Health += Delta, 0.f, MaxHealth);

	float ActualDelta = Health - PrevHealth;

	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta);
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

float UCAttributeComponent::GetMaxHealth() const
{
	return MaxHealth;
}
