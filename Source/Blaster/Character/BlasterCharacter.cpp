// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "BlasterAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Flyboard/Flyboard.h"
#include "Blaster/Helper/DebugHelper.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom2"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350;
	// CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera2"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// // Make mouse look around character
	// // bUseControllerRotationYaw = false;
	// // GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget2"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CompactComponent2"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanFly = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// SetRootComponent(GetMesh());

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent2"));
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!DefaultMappingContext)
		return;

	// Add Input Mapping Context
	if (APlayerController *PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}

	// GetCapsuleComponent()->SetRelativeRotation(FRotator(-360.f, -360.f, -360.f), true);
	// GetMesh()->SetRelativeRotation(FRotator(-360.f, -360.f, -360.f), true);
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug::Print(IsReceviedHit ? "IsReceviedHit: true" : "IsReceviedHit: flase", FColor::Purple, 2);

	HideCameraIfCharacterClose();
	PollInit();

	// CheckDistanceToFloor();

	AimOffset(DeltaTime);
	// RotateCharacterTimer();
	// FRotator InitialRotation = GetActorRotation();
	// if (Combat->EquippedFlyboard && IsReceviedHit)
	// {
	// 	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 	bUseControllerRotationYaw = false;

	// 	// GetMesh()->AddLocalRotation(FRotator(0.f, 0.f, 2.0f * DeltaTime));
	// 	InitialRotation.Yaw = InitialRotation.Yaw + 1000.f * DeltaTime;
	// 	SetActorRotation(InitialRotation);
	// 	// AddActorLocalRotation(FRotator(0.f, 1000.0f * DeltaTime, 0.f));

	// 	// RotateCharacterTenTimes();
	// 	// FRotator InitialRotation = GetActorRotation();
	// 	// SetActorRotation(FRotator(0.f, 360.f, 0.f));
	// 	// Debug::Print(InitialRotation.ToString(), FColor::Yellow, 1);
	// }
	// Debug::Print(InitialRotation.ToString(), FColor::Yellow, 1);

	// Debug::Print(IsMoveInputIgnored() ? "true" : "false", FColor::Yellow, 1);
	// FVector Start = GetActorLocation();
	// FVector End = Start + GetActorForwardVector();

	// DoCapsuleTraceMultiByObject(Start, End, true, true);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingFlyboard, COND_OwnerOnly);
	// DOREPLIFETIME(ABlasterCharacter, ZValue);
	DOREPLIFETIME(ABlasterCharacter, Health);

	DOREPLIFETIME(ABlasterCharacter, ReplicatedLocation);
	DOREPLIFETIME(ABlasterCharacter, ReplicatedRotation);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent *EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMovementInputPressed);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);

		// Equipping
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::EquipButtonPressed);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::CrouchButtonPressed);

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::AimButtonReleased);

		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);

		EnhancedInputComponent->BindAction(VerticalMovementAction, ETriggerEvent::Triggered, this, &ThisClass::HandleVerticalMovement);
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}

	if (Combat && Combat->EquippedFlyboard)
	{
		Combat->EquippedFlyboard->Dropped();
	}

	MulticastElim();

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinished,
		ElimDelay);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation());
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

	if (Combat && Combat->EquippedFlyboard)
	{
		// Combat->EquippedFlyboard->SetSimulatePhysics(true);
		// Combat->EquippedFlyboard->SetEnableGravity(true);
		Combat->EquippedFlyboard->Dropped();
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode *BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();

	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	// if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType *DamageType, AController *InstigatorController, AActor *DamageCauser)
{
	IsReceviedHit = true;

	// Debug::Print(IsReceviedHit ? "IsReceviedHit: true" : "IsReceviedHit: flase", FColor::Purple, 2);

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	ABlasterPlayerController *AttackerController = Cast<ABlasterPlayerController>(InstigatorController);

	AttackerCharacter = Cast<ABlasterCharacter>(GetOwner());
	DamageCharacter = Cast<ABlasterCharacter>(DamagedActor);

	if (BlasterPlayerController == AttackerController)
	{
		// Debug::Print("Hit Self", FColor::Purple, 2);
		return;
	}

	// Debug::Print(FString::SanitizeFloat(Damage), FColor::Purple, 3);

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();

	FVector ForwardVector = AttackerController->GetCharacter()->GetActorForwardVector();
	ForwardVector.Normalize(); // Ensure it's a unit vector
							   // LaunchCharacter(ForwardVector * 1000.f, false, false);

	// FRotator InitialRotation = GetActorRotation();
	// if (Combat->EquippedFlyboard && IsReceviedHit)
	// {

	// 	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 	bUseControllerRotationYaw = false;
	// 	// while (IsReceviedHit)
	// 	// {

	// 	// GetMesh()->AddLocalRotation(FRotator(0.f, 0.f, 2.0f * DeltaTime));
	// 	InitialRotation.Yaw = InitialRotation.Yaw + 1000.f * GetWorld()->GetDeltaSeconds();
	// 	AddActorWorldRotation(InitialRotation);
	// 	// SetActorRotation(InitialRotation);
	// 	// AddActorLocalRotation(FRotator(0.f, 1000.0f * DeltaTime, 0.f));

	// 	// RotateCharacterTenTimes();
	// 	// FRotator InitialRotation = GetActorRotation();
	// 	// SetActorRotation(FRotator(0.f, 360.f, 0.f));
	// 	// Debug::Print(InitialRotation.ToString(), FColor::Yellow, 1);
	// 	// }
	// }

	// if (Combat->EquippedFlyboard && IsReceviedHit)
	// {
	// 	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 	bUseControllerRotationYaw = false;

	// 	// GetMesh()->AddLocalRotation(FRotator(0.f, 0.f, 2.0f * DeltaTime));
	// 	InitialRotation.Yaw = InitialRotation.Yaw + 1000.f * DeltaTime;
	// 	SetActorRotation(InitialRotation);
	// 	// AddActorLocalRotation(FRotator(0.f, 1000.0f * DeltaTime, 0.f));

	// 	// RotateCharacterTenTimes();
	// 	// FRotator InitialRotation = GetActorRotation();
	// 	// SetActorRotation(FRotator(0.f, 360.f, 0.f));
	// 	// Debug::Print(InitialRotation.ToString(), FColor::Yellow, 1);
	// }
	// Debug::Print(InitialRotation.ToString(), FColor::Yellow, 1);

	// RotateCharacterTenTimes();

	DamageCharacter->LaunchCharacter(ForwardVector * 1000.f, false, false);

	// DamageCharacter->AddActorLocalRotation(FRotator(0.f, 0.f, -90.f), true);

	// GetCapsuleComponent()->AddLocalRotation(FRotator(0.f, 0.f, -360.f), true);

	// DamageCharacter->GetCharacterMovement()->AddInputVector(ForwardVector * 1000.f);
	// SetMovementMode(MOVE_Falling);
	// DamageCharacter->GetCharacterMovement()->StopMovementImmediately();
	// if (Combat->EquippedFlyboard)
	// {
	// 	// if (HasAuthority())
	// 	// {
	// 	// 	DamageCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	// 	// }
	// 	// else
	// 	// {
	// 	// GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	// 	ServerResetAfterHit();
	// 	// }

	// 	// FVector Location = DamageCharacter->GetCh
	// 	// aracterMovement()->GetActorLocation();

	// 	// DamageCharacter->GetCharacterMovement() - SetActorLocation(FVector(0.f, 0.f, Location.Z));
	// }

	// AddMovementInput(ForwardVector * 1000.f, 1.0f, true);
	// DamageCharacter->GetMovementComponent()->AddInputVector(ForwardVector * 1000.f);

	// FVector ImpulseForce = ForwardVector * 1000.f; // ImpulseMagnitude is the strength of the impulse
	// FVector Location = DamageCharacter->GetActorLocation();
	// UPrimitiveComponent *PrimitiveComponent = Cast<UPrimitiveComponent>(RootComponent);
	// if (PrimitiveComponent)
	// {
	// 	// PrimitiveComponent->AddForce(FVector(ImpulseForce.X, ImpulseForce.Y, ForwardVector.Z));
	// 	PrimitiveComponent->AddForceAtLocation(ImpulseForce, Location);
	// 	PrimitiveComponent->AddImpulseAtLocation(ImpulseForce, Location);
	// }

	if (Health <= 0.f)
	{
		ABlasterGameMode *BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();

		if (BlasterGameMode)
		{
			// BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			// ABlasterPlayerController *AttackerController = Cast<ABlasterPlayerController>(InstigatorController);

			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{

	if (BlasterPlayerController == nullptr)
	{
		BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
	}

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0);
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (IsWeaponEquipped())
	{
		Combat->FireButtonPressed(true);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (IsWeaponEquipped())
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

#pragma region Overlapping
void ABlasterCharacter::SetOverlappingWeapon(AWeapon *Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::SetOverlappingFlyboard(AFlyboard *Flyboard)
{
	OverlappingFlyboard = Flyboard;
}

void ABlasterCharacter::OnRep_OverlappingFlyboard(AFlyboard *LastFlyboard)
{
	// OverlappingFlyboard = LastFlyboard;
	// if (OverlappingFlyboard)
	// {
	// 	ABlasterCharacter::EquipOverlappingFlyboard();
	// }
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon *LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
#pragma endregion

void ABlasterCharacter::CheckDistanceToFloor()
{
	// Draw forward line
	const FVector ActorLocation = GetActorLocation();
	const FVector EyeHeightOffset = GetActorUpVector() * (100.0f + 100.0f);
	const FVector Start = ActorLocation + EyeHeightOffset;
	const FVector End = Start + GetActorForwardVector() * 2000.0f;
	DoLineTraceSingleByObject(Start, End, ShowTraceCheckDistanceToFloor, false);

	// Draw line to floor
	FVector DownVector = GetActorRotation().RotateVector(FVector(0, 0, -1));
	FVector StartLocation = End;
	FVector EndLocation = StartLocation - DownVector * -5000.0f;
	FHitResult HitResult = DoLineTraceSingleByObject(StartLocation, EndLocation, ShowTraceCheckDistanceToFloor, false);

	DistanceToFloor = FVector::Distance(Start, HitResult.ImpactPoint);

	// Debug::Print("DistanceToFloor: " + FString::SanitizeFloat(DistanceToFloor), FColor::Red, 1);

	// DoCapsuleTraceMultiByObject(ActorLocation, ActorLocation, true, false);

	if (HitResult.GetActor() == nullptr || HitResult.GetActor()->GetName().IsEmpty())
		return;

	if (HitResult.GetActor()->GetName().StartsWith("Water"))
	{
		// Debug::Print("Water", FColor::Red, 1);
		CanMoveForward = true;
	}
	else
	{
		// Debug::Print("Land", FColor::Red, 1);
		// CanMoveForward = false;
	}
}
void ABlasterCharacter::HandleMovementInputPressed(const FInputActionValue &Value)
{
	if (Combat && Combat->EquippedFlyboard)
	{
		HandleFlyboardMovementInput(Value);
	}
	else
	{
		HandleGroundMovementInput(Value);
	}
}

void ABlasterCharacter::HandleGroundMovementInput(const FInputActionValue &Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABlasterCharacter::HandleFlyboardMovementInput(const FInputActionValue &Value)
{

	CheckDistanceToFloor();

	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (GetCanMoveForward())
		{
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void ABlasterCharacter::HandleVerticalMovement(const FInputActionValue &Value)
{
	// Fix Vertical Move Afrer Hit
	// TODO: Later create function for this fix
	if (!GetCharacterMovement()->IsFlying())
	{
		EquipOverlappingFlyboard();
	}

	if (IsReceviedHit)
	{
		// EquipButtonPressed();
		IsReceviedHit = false;
	}

	// CheckDistanceToFloor();

	if (Combat && Combat->EquippedFlyboard && Controller != nullptr)
	{

		float ZValue = Value.Get<float>();
		float VerticalSpeed = 1.5f;

		if (ZValue > 0.f && DistanceToFloor >= MaxFlyUp)
		{
			VerticalSpeed = 1.5f;
			return;
		}

		if (ZValue < 0.f && DistanceToFloor <= MaxFlyDown)
		{
			VerticalSpeed = 0.1f;
			return;
		}

		AddMovementInput(FVector(0.0f, 0.0f, ZValue), VerticalSpeed, true);
	}
}

void ABlasterCharacter::Look(const FInputActionValue &Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}
AWeapon *ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)
		return nullptr;
	return Combat->EquippedWeapon;
}

AFlyboard *ABlasterCharacter::GetEquippedFlyboard()
{
	if (Combat == nullptr)
		return nullptr;
	return Combat->EquippedFlyboard;
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())
		return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

#pragma region Equipment
void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			ABlasterCharacter::EquipOverlappingWeapon();
		}
		else if (OverlappingFlyboard)
		{
			ABlasterCharacter::EquipOverlappingFlyboard();
		}
	}
}

void ABlasterCharacter::EquipOverlappingWeapon()
{
	if (HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		ServerEquipWeapon();
	}
}

void ABlasterCharacter::EquipOverlappingFlyboard()
{
	if (HasAuthority())
	{
		Combat->EquipFlyboard(OverlappingFlyboard);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		GetCharacterMovement()->MaxFlySpeed = 3000.f;
		GetCharacterMovement()->BrakingDecelerationFlying = 300.f;
	}
	else
	{
		ServerEquipFlyboard();
	}
}

void ABlasterCharacter::ServerEquipWeapon_Implementation()
{
	Combat->EquipWeapon(OverlappingWeapon);
}

void ABlasterCharacter::ServerEquipFlyboard_Implementation()
{
	Combat->EquipFlyboard(OverlappingFlyboard);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = 500.f;
	GetCharacterMovement()->BrakingDecelerationFlying = 300.f;
}
#pragma endregion

#pragma region Tracing

TArray<FHitResult> ABlasterCharacter::DoCapsuleTraceMultiByObject(const FVector &Start, const FVector &End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;

		if (bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this, Start, End,
		500.0f,
		70.0f,
		TraceSurfaceTraceTypes,
		false,
		TArray<AActor *>(),
		DebugTraceType,
		OutCapsuleTraceHitResults,
		false,
		FLinearColor::Red);

	return OutCapsuleTraceHitResults;
}

FHitResult ABlasterCharacter::DoLineTraceSingleByObject(const FVector &Start, const FVector &End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	FHitResult OutHit;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;

		if (bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this, Start, End,
		TraceSurfaceTraceTypes,
		false,
		TArray<AActor *>(),
		DebugTraceType,
		OutHit,
		false);

	return OutHit;
}
#pragma endregion

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr)
		return;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool IsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !IsInAir) // Standing, Still not jumbing
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StatringAimRotation);
		AO_Yaw = DeltaRotation.Yaw;
		// bUseControllerRotationYaw = false;
	}

	if (Speed > 0.f && IsInAir) // Jumbing
	{
		StatringAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		// bUseControllerRotationYaw = true;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
}

void ABlasterCharacter::OnRep_CharacterMovment()
{
	// ReplicatedRotation = GetActorRotation();
	// Debug::Print(ReplicatedLocation.ToString());
}

// Example function to rotate the character around itself 10 times
void ABlasterCharacter::RotateCharacterTimer()
{

	FRotator InitialRotation = GetActorRotation();
	if (Combat->EquippedFlyboard && IsReceviedHit)
	{
		// GetCharacterMovement()->
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		// while (IsReceviedHit)
		// {

		// GetMesh()->AddLocalRotation(FRotator(0.f, 0.f, 2.0f * DeltaTime));
		InitialRotation.Yaw = InitialRotation.Yaw + 1000.f * GetWorld()->GetDeltaSeconds();
		AddActorWorldRotation(InitialRotation);
		// SetActorRotation(InitialRotation);
		// AddActorLocalRotation(FRotator(0.f, 1000.0f * DeltaTime, 0.f));

		// RotateCharacterTenTimes();
		// FRotator InitialRotation = GetActorRotation();
		// SetActorRotation(FRotator(0.f, 360.f, 0.f));
		// Debug::Print(InitialRotation.ToString(), FColor::Yellow, 1);
		// }
	}
}

// void ABlasterCharacter::ServerResetAfterHit_Implementation()
// {
// 	DamageCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
// }