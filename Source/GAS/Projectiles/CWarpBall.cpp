#include "CWarpBall.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ACWarpBall::ACWarpBall()
{
	DetonateDelay = 0.1f;
	TeleportDelay = 0.1f;

	MoveComp->InitialSpeed = 6000.f;
}

void ACWarpBall::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TimeHandle_DelayDetonate, this, &ACWarpBall::Explode, DetonateDelay);
}

void ACWarpBall::Explode_Implementation()
{
	GetWorldTimerManager().ClearTimer(TimeHandle_DelayDetonate);

	UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());

	EffectComp->DeactivateSystem();
	MoveComp->StopMovementImmediately();

	SetActorEnableCollision(false);

	FTimerHandle TimerHandle_Teleport;
	GetWorldTimerManager().SetTimer(TimerHandle_Teleport, this, &ACWarpBall::TeleportInstigator, TeleportDelay);

}

void ACWarpBall::TeleportInstigator()
{
	AActor* ActorToTeleport = GetInstigator();

	if (ensure(ActorToTeleport))
	{
		ActorToTeleport->TeleportTo(GetActorLocation(), ActorToTeleport->GetActorRotation());
	}

	Destroy();
}
