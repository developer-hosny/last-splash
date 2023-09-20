// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
// #include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
// #include "Engine/SkeletalMeshSocket.h"

#include "Flyboard.generated.h"

class UParticleSystemComponent;
class UNiagaraComponent;

UCLASS()
class BLASTER_API AFlyboard : public AActor
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	AFlyboard();
	void Dropped();
	void ActiveThruster(bool bIsActive);

private:
	// UPROPERTY(VisibleAnywhere, Category = "Flyboard Properties")
	// class USkeletalMeshComponent *FlyboardMesh;

	UPROPERTY(VisibleAnywhere, Category = "Flyboard Properties")
	class USphereComponent *AreaSphere;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult &SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex);

	// UPROPERTY(EditAnywhere)
	// class UBoxComponent *CollisionBox;

	UPROPERTY()
	USceneComponent *DefaultRoot = nullptr;

	UPROPERTY(EditAnywhere, Category = FlyboardMesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent *FlyboardMesh = nullptr;
	// UPROPERTY(EditAnywhere, Category = Jet, meta = (AllowPrivateAccess = "true"))
	// UStaticMeshComponent *RightJet = nullptr;

	// UPROPERTY(EditAnywhere, Category = Jet, meta = (AllowPrivateAccess = "true"))
	// UStaticMeshComponent *JetConnector = nullptr;

	UPROPERTY(EditAnywhere, Category = Thruster, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent *ThrusterLeft;
	UPROPERTY(EditAnywhere, Category = Thruster, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent *ThrusterRight;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsOnFlyingBoard;

	// UPROPERTY(BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	// bool bActiveThrusters = false;

	// FORCEINLINE void SetThrusters(bool bIsActive) { bActiveThrusters = bIsActive; }
	FORCEINLINE bool IsOnFlyingBoard() const { return bIsOnFlyingBoard; }
};
