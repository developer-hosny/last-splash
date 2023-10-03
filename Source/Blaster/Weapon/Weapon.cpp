// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "NiagaraComponent.h"
#include "Blaster/Helper/DebugHelper.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	// Attach the arrow component to the actor.
	Arrow->AttachToComponent(WeaponMesh, FAttachmentTransformRules::KeepRelativeTransform);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(WeaponMesh);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(WeaponMesh);

	WaterNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NS_Water"));
	WaterNiagara->AttachToComponent(WeaponMesh, FAttachmentTransformRules::KeepRelativeTransform, FName("WaterSocket"));
	WaterNiagara->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	if (WaterNiagara)
	{

		// WaterNiagara->OnNiagaraCollision.AddDynamic(this, &AWeapon::HandleNiagaraCollision);
	}
}

void AWeapon::HandleNiagaraCollision()
{
	// You can access collision information from CollisionPayload
	// FVector CollisionLocation = CollisionPayload.Position;
	// FVector CollisionNormal = CollisionPayload.Normal;

	// Your custom collision handling logic here
	// For example, spawn effects, apply damage, etc.
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{

	Character = Cast<ABlasterCharacter>(OtherActor);

	if (Character)
	{
		Character->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	Character = Cast<ABlasterCharacter>(OtherActor);

	if (Character)
	{
		Character->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;
	case EWeaponState::EWS_EquippedSecondary:
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{

	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_EquippedSecondary:
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DoLineTraceStartFromWeapon();
}

FHitResult AWeapon::DoLineTraceStartFromWeapon()
{
	FHitResult HitResult;

	if (Character)
	{
		const USkeletalMeshSocket *WaterSocket = GetWeaponMesh()->GetSocketByName(FName("WaterSocket"));
		if (WaterSocket)
		{
			FTransform SocketTransform = WaterSocket->GetSocketTransform(GetWeaponMesh());
			FVector Start = SocketTransform.GetLocation();
			FVector SocketForwardVector = SocketTransform.GetRotation().GetForwardVector();
			FVector End = Start + SocketForwardVector * 8000.0f;

			HitResult = DoLineTraceSingleByObject(
				Start,
				End,
				Character->ShowTraceAimDebugLine,
				false);
		}
	}

	return HitResult;
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

// Add variables that you need to replicate
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::Fire(const FVector &HitTarget)
{

	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	// const USkeletalMeshSocket *WaterSocket = WeaponMesh->GetSocketByName(FName("WaterSocket"));

	// if (WaterSocket)
	// {
	// 	FTransform SocketTransform = WaterSocket->GetSocketTransform(GetWeaponMesh());
	// 	UWorld *World = GetWorld();

	// 	if (World)
	// 	{
	// 		// AActor *SpawnedObject = GetWorld()->SpawnActor<USphereComponent>(ASphere::StaticClass(), HitTarget, FRotator::ZeroRotator);

	// 			World->SpawnActor<ACasing>(
	// 				Water,
	// 				SocketTransform.GetLocation(),
	// 				SocketTransform.GetRotation().Rotator());

	// 		// World->SpawnActor<USphereComponent>(
	// 		// 	CasingClass,
	// 		// 	SocketTransform.GetLocation(),
	// 		// 	SocketTransform.GetRotation().Rotator());
	// 	}
	// }

	// if (WaterNiagara)
	// {
	// 	WaterNiagara->SetAutoActivate(true);
	// }
	// return;

	// if (CasingClass)
	// {
	// 	const USkeletalMeshSocket *AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));

	// 	if (AmmoEjectSocket)
	// 	{
	// 		FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
	// 		UWorld *World = GetWorld();

	// 		if (World)
	// 		{
	// 			World->SpawnActor<ACasing>(
	// 				CasingClass,
	// 				SocketTransform.GetLocation(),
	// 				SocketTransform.GetRotation().Rotator());
	// 		}
	// 	}
	// }
}

void AWeapon::Dropped()
{
	if (WaterNiagara)
	{
		WaterNiagara->SetAutoActivate(false);
		WaterNiagara->Deactivate();
	}
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
}

FHitResult AWeapon::DoLineTraceSingleByObject(const FVector &Start, const FVector &End, bool bShowDebugShape, bool bDrawPersistentShapes)
{
	FHitResult OutHit;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;

		if (bDrawPersistentShapes)
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
		true);

	return OutHit;
}