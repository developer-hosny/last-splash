// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "Components/WidgetComponent.h"
#include <Net/UnrealNetwork.h>
#include "Blaster/CompactComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include <Animation/AnimMontage.h>
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Components/TimelineComponent.h"

#include "BlasterCharacter.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter,
									  public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;

	// void PhysicsClimb(float deltaTime, int32 Iterations);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent *OverheadWidget;
	void PlayFireMontage(bool bAiming);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	void PlayElimMontage();
	void Elim();

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent *Combat;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void PlayHitReactMontage();
	UFUNCTION()
	void ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType *DamageType, class AController *InstigatorController, AActor *DamageCauser);
	void UpdateHUDHealth();

	// Poll for any relelvant classes and initialize our HUD
	void PollInit();

	// Rotation variables
	float RotationProgress;
	bool bRotatingForward;

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent *CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent *FollowCamera;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon *OverlappingWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingFlyboard)
	class AFlyboard *OverlappingFlyboard;

	// UPROPERTY()
	// class AFlyboard *EquippedFlyboard;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon *LastWeapon);

	UFUNCTION()
	void OnRep_OverlappingFlyboard(AFlyboard *LastFlyboard);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon();

	UFUNCTION(Server, Reliable)
	void ServerEquipFlyboard();

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage *FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage *HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage *ElimMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY()
	class ABlasterPlayerController *BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	/**
	 * Player health
	 */

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
	 * Dissolve effect
	 */

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent *DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat *DissolveCurve;

	UPROPERTY()
	class ABlasterPlayerState *BlasterPlayerState;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic *DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance *DissolveMaterialInstance;

	/**
	 * Elim effects
	 */

	UPROPERTY(EditAnywhere)
	UParticleSystem *ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent *ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue *ElimBotSound;

	float AO_Yaw;
	float AO_Pitch;
	FRotator StatringAimRotation;

public:
#pragma region ClimbTraces

	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector &Start, const FVector &End, bool bShowDebugShape = false, bool bDrawPersistantShapes = false);

	FHitResult DoLineTraceSingleByObject(const FVector &Start, const FVector &End, bool bShowDebugShape = false, bool bDrawPersistantShapes = false);

#pragma endregion

	void SetOverlappingWeapon(AWeapon *Weapon);
	void SetOverlappingFlyboard(AFlyboard *Flyboard);
	bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon *GetEquippedWeapon();
	AFlyboard *GetEquippedFlyboard();
	FORCEINLINE UCameraComponent *GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

#pragma region Inputs

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext *DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *VerticalMovementAction;

#pragma endregion

#pragma region InputCallback
	void EquipButtonPressed();
	void EquipOverlappingWeapon();
	void EquipOverlappingFlyboard();

	void HandleMovementInputPressed(const FInputActionValue &Value);
	void HandleGroundMovementInput(const FInputActionValue &Value);
	void HandleFlyboardMovementInput(const FInputActionValue &Value);
	void HandleVerticalMovement(const FInputActionValue &Value);

	void Look(const FInputActionValue &Value);
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void AimOffset(float DeltaTime);

	void CheckDistanceToFloor();
#pragma endregion

#pragma region FlyBoardVars
	float DistanceToFloor = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxFlyUp = 10000.0f;
	UPROPERTY(EditAnywhere)
	float MaxFlyDown = 100.0f;

	bool CanMoveForward = true;

	FORCEINLINE bool GetCanMoveForward() const { return CanMoveForward; }
#pragma region

#pragma region TracingVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movment: Tracing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceSurfaceTraceTypes;

#pragma endregion

	FORCEINLINE float GetAO_Yaw() const
	{
		return AO_Yaw;
	}

	FORCEINLINE float GetAO_Pitch() const
	{
		return AO_Pitch;
	}

#pragma region FixVerticalMoveAfrerHit

	FTimerHandle TimerHandleResetIsReceviedHit;
	void ResetIsReceviedHit();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Hit, meta = (AllowPrivateAccess = "true"))
	bool IsReceviedHit = false;
	FORCEINLINE bool GetIsReceviedHit() const { return IsReceviedHit; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Hit, meta = (AllowPrivateAccess = "true"))
	ABlasterCharacter *AttackerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Hit, meta = (AllowPrivateAccess = "true"))
	ABlasterCharacter *DamageCharacter;

#pragma endregion

#pragma region TraceLinesVarables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Debug Lines", meta = (AllowPrivateAccess = "true"))
	bool ShowTraceAimDebugLine = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Debug Lines", meta = (AllowPrivateAccess = "true"))
	bool ShowTraceCheckDistanceToFloor = false;
#pragma endregion

#pragma region ClientMovement
	// Replicated variables
	UPROPERTY(ReplicatedUsing = OnRep_CharacterMovment)
	FVector ReplicatedLocation;
	UPROPERTY(ReplicatedUsing = OnRep_CharacterMovment)
	FRotator ReplicatedRotation;

	// // Override ReplicateMovement to set bReplicateMovement
	// virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	UFUNCTION()
	void OnRep_CharacterMovment();

	FTimerHandle TimerHandleActorRotation;
	void RotateCharacterTimer();

	// UFUNCTION(Server, Reliable)
	// void ServerResetAfterHit();
#pragma endregion
};
