// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include <Components/SkeletalMeshComponent.h>
#include <Components/ArrowComponent.h>
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Casing.h"
// #include "Engine/Texture2D.h"

#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeapon();

	ABlasterCharacter *Character;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void Fire(const FVector &HitTarget);
	void Dropped();

	/**
	 * Textures for the weapon crosshairs
	 */

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D *CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D *CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D *CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D *CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D *CrosshairsBottom;

	/**
	 * Zoomed FOV while aiming
	 */

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.5f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = WaterAsFire, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent *WaterNiagara;

	void HandleNiagaraCollision();

#pragma region TracingVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = " Tracing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceSurfaceTraceTypes;

#pragma endregion

#pragma region Traces
	FHitResult DoLineTraceSingleByObject(const FVector &Start, const FVector &End, bool bShowDebugShape = false, bool bDrawPersistantShapes = false);
	FHitResult DoLineTraceStartFromWeapon();
#pragma endregion

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USkeletalMeshComponent *WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent *AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UArrowComponent *Arrow;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent *PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset *FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	// #pragma region TracingVariables

	// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movment: Tracing", meta = (AllowPrivateAccess = "true"))
	// 	TArray<TEnumAsByte<EObjectTypeQuery>> TraceSurfaceTraceTypes;

	// #pragma endregion

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent *GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent *GetWeaponMesh() const { return WeaponMesh; };
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
};
