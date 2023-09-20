// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
// #include "Particles/ParticleSystemComponent.h"
// #include "Particles/ParticleSystem.h"

#include "ProjectileWater.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileWater : public AProjectile
{
	GENERATED_BODY()

public:
	// UPROPERTY(EditAnywhere)
	// class UParticleSystem *ImpactParticle;

protected:
	virtual void OnHit(UPrimitiveComponent *HitComp,
					   AActor *OtherActor, UPrimitiveComponent *OtherComp,
					   FVector NormalImpulse, const FHitResult &Hit) override;
};
