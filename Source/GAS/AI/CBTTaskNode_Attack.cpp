

#include "CBTTaskNode_Attack.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UCBTTaskNode_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

			
	AAIContoroller* AIC = OwnerComp.GetAIOwner();
	if (ensure(AIC))
	{
		ACharacter* BotCharacter = Cast<ACharacter>(AIC->GetPawn());
		if (!BotCharacter)
		{
			return EBTNodeResult::Failed;
		}

		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		if (ensure(BB))
		{
			AActor* TargetActor = Cast<Actor>(BB->GetValueAsObject("TargetActor");
			if (TargetActor)
			{
				FVector MuzzleLocation = BotCharacter->GetMesh()->GetSocketLocation("BotCharacter");
				FVector Direction = TargetActor->GetActorLocation() - MuzzleLocation;

				FRotator RotationToPlayer = Direction.Rotation();


				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESPawnActorCollisionHandlingMethod::AlwaysSpawn;
				GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, RotationToPlayer, SpawnCollisionHandlingOverride);
			}
		}
	}
}
