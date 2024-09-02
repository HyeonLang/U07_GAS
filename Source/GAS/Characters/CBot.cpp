#include "CBot.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CAttributeComponent.h"
#include "Components/CActionComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BrainComponent.h"
#include "UI/CWorldWidget.h"

ACBot::ACBot()
{
	PawnSesningComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSesningComp");
	AttributeComp = CreateDefaultSubobject<UCAttributeComponent>("AttributeComp");
	ActionComp = CreateDefaultSubobject<UCActionComponent>("ActionComp");

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	TimeToHitParamName = "TimeToHit";
}

void ACBot::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	PawnSesningComp->OnSeePawn.AddDynamic(this, &ACBot::OnSeePawn);
	AttributeComp->OnHealthChanged.AddDynamic(this, &ACBot::OnHealthChanged);
}

void ACBot::OnSeePawn_Implementation(APawn* Pawn)
{
	SetTargetActor(Pawn);

	//DrawDebugString(GetWorld(), GetActorLocation(), "I found you!!", nullptr, FColor::White, 3.f, true);
	
}

void ACBot::OnHealthChanged(AActor* InstigatorActor, UCAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.f)
	{
		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName, GetWorld()->TimeSeconds);

		AAIController* AIC = GetController<AAIController>();
		if (NewHealth <= 0.f)
		{
			if (AIC)
			{
				AIC->GetBrainComponent()->StopLogic("Killed");
			}

			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName("Ragdoll");

			SetLifeSpan(5.f);
			return;
		}

		if (HealthBarWidget == nullptr)
		{
			HealthBarWidget = CreateWidget<UCWorldWidget>(GetWorld(), HealthBarWidgetClass);
			if (HealthBarWidget)
			{
				HealthBarWidget->AttachToActor = this;
				HealthBarWidget->AddToViewport();
			}
		}
		SetTargetActor(InstigatorActor);
	}
}

void ACBot::SetTargetActor(AActor* NewTarget)
{
	AAIController* AIC = GetController<AAIController>();
	if (ensure(AIC) && NewTarget && NewTarget != this)
	{
		AIC->GetBlackboardComponent()->SetValueAsObject("TargetActor", NewTarget);
	}
}
