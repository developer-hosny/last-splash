// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"

#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Destroyed() override;

	/**
	 * Used with server-side rewind
	 */

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 30000.f;

	// Only set this for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	// Doesn't matter for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit);

private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent *CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent *ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem *Tracer;

	UPROPERTY()
	class UParticleSystemComponent *TracerComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem *ImpactParticle;

	UPROPERTY(EditAnywhere)
	class USoundCue *ImpactSound;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditDefaultsOnly)
	float DestroyDelay = 0.15f;

	void DestroyTimerFinished();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
