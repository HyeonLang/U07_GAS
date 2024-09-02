
#include "CActionThornCounter.h"
#include "Components/CAttributeComponent.h"

UCActionThornCounter::UCActionThornCounter()
{
	ThornCounterDamagePer = 0.1f;
}

void UCActionThornCounter::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	UCAttributeComponent* AttributeComp = UCAttributeComponent::GetAttributes(Instigator);
	UE_LOG(LogTemp, Log, TEXT("CounterStart"));
	AttributeComp->OnHealthChanged.AddDynamic(this, &UCActionThornCounter::OnExecuteThornCounter);
}

void UCActionThornCounter::StopAction_Implementation(AActor* Instigator)
{
	UCAttributeComponent* AttributeComp = UCAttributeComponent::GetAttributes(Instigator);
	if (AttributeComp->OnHealthChanged.IsAlreadyBound(this, &UCActionThornCounter::OnExecuteThornCounter))
	{
		AttributeComp->OnHealthChanged.RemoveDynamic(this, &UCActionThornCounter::OnExecuteThornCounter);
	}

	Super::StopAction_Implementation(Instigator);
}

void UCActionThornCounter::OnExecuteThornCounter_Implementation(AActor* InstigatorActor, UCAttributeComponent* OwningComp, float NewHealth, float Delta)
{
}
