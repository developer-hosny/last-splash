// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Flyboard/Flyboard.h"

#include "CombatComponent.generated.h"

class AWeapon;
class USkeletalMeshSocket;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	friend class ABlasterCharacter;
	void EquipWeapon(AWeapon *WeaponToEquip);
	void EquipFlyboard(AFlyboard *FlyBoardToEquip);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_EquippedFlyboard();

	void FireButtonPressed(bool bPressed);

	// UFUNCTION(Server, Reliable)
	// void ServerFire(const FVector_NetQuantize &TraceHitTarget);

	// UFUNCTION(NetMulticast, Reliable)
	// void MulticastFire(const FVector_NetQuantize &TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerWater(bool bIsWaterPressed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastWater(bool bIsWaterPressed);

	void TraceUnderCrosshairs(FHitResult &TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	UPROPERTY()
	class ABlasterCharacter *Character;
	UPROPERTY()
	class ABlasterPlayerController *Controller;
	UPROPERTY()
	class ABlasterHUD *HUD;

	FHUDPackage HUDPackage;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon *EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool bFireButtonPressed;

	/**
	 * HUD and crosshairs
	 */

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	// FVector HitTarget;

	/**
	 * Aiming and FOV
	 */

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	 * Automatic fire
	 */

	FTimerHandle FireTimer;
	bool bCanFire = true;

	// FVector_NetQuantize HitTarget;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	void Fire();

public:
	UPROPERTY(ReplicatedUsing = OnRep_EquippedFlyboard)
	AFlyboard *EquippedFlyboard;
};
