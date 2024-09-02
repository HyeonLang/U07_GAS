#include "CAttributeComponent.h"
#include "Game/CGameMode.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("Tore.DamageMultiplier"), 1.f, TEXT("Modify damage multiplier"), ECVF_Cheat);

UCAttributeComponent::UCAttributeComponent()
{
	MaxHealth = 100.f;
	Health = MaxHealth;

	// 컴포넌트 리플리케이트
	SetIsReplicatedByDefault(true); //생성자 전용 리플리케이트 Set
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

	if (!FMath::IsNearlyZero(ActualDelta))
	{
		NetMulticastHealthChanged(InstigatorActor, Health, ActualDelta);
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

void UCAttributeComponent::NetMulticastHealthChanged_Implementation(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(InstigatorActor, this, NewHealth, Delta);
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
	// 최초 한번 리플리케이트

}
