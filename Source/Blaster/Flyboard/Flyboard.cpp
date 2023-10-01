// Fill out your copyright notice in the Description page of Project Settings.

#include "Flyboard.h"
#include "Blaster/Character/BlasterCharacter.h"

AFlyboard::AFlyboard()
{
	PrimaryActorTick.bCanEverTick = true;

	// FlyboardMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	// FlyboardMesh->SetupAttachment(RootComponent);
	// SetRootComponent(FlyboardMesh);

	// DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	// SetRootComponent(RootComponent);
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	// AreaSphere->AttachToComponent(FlyboardMesh, FAttachmentTransformRules::KeepRelativeTransform);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(AreaSphere);

	FlyboardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flyboard"));
	FlyboardMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlyboardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlyboardMesh->AttachToComponent(AreaSphere, FAttachmentTransformRules::KeepRelativeTransform);

	// RightJet = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightJet"));
	// RightJet->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// RightJet->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// JetConnector = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JetConnector"));
	// JetConnector->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// JetConnector->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// RightJet->AttachToComponent(AreaSphere, FAttachmentTransformRules::KeepRelativeTransform);
	// JetConnector->AttachToComponent(AreaSphere, FAttachmentTransformRules::KeepRelativeTransform);

	ThrusterLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ThrusterLeft"));
	ThrusterRight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ThrusterRight"));

	// // FTransform ThrustLeft = FlyboardMesh->GetWeaponMesh()->GetSocketTransform(FName("ThrustLeft"), ERelativeTransformSpace::RTS_World);
	// // FTransform ThrustRight = FlyboardMesh->GetWeaponMesh()->GetSocketTransform(FName("ThrustRight"), ERelativeTransformSpace::RTS_World);

	ThrusterLeft->AttachToComponent(FlyboardMesh, FAttachmentTransformRules::KeepRelativeTransform,
									FName("ThrustLeft"));
	ThrusterRight->AttachToComponent(FlyboardMesh, FAttachmentTransformRules::KeepRelativeTransform,
									 FName("ThrustRight"));
	// // ThrusterRight->AttachToComponent(FlyboardMesh, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFlyboard::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}
}

void AFlyboard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFlyboard::OnSphereOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingFlyboard(this);
	}
}

void AFlyboard::OnSphereEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter *BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingFlyboard(nullptr);
	}
}

void AFlyboard::Dropped()
{
	ThrusterLeft->Deactivate();
	ThrusterRight->Deactivate();

	// AFlyboard::ActiveThruster(false);

	// if (HasAuthority())
	// {
	// 	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// }
	FlyboardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlyboardMesh->SetSimulatePhysics(true);
	FlyboardMesh->SetEnableGravity(true);

	// FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	// FlyboardMesh->DetachFromComponent(DetachRules);
	// // AreaSphere->DetachFromComponent(DetachRules);
	// SetOwner(nullptr);
}

void AFlyboard::ActiveThruster(bool bIsActive)
{
	ThrusterLeft->Activate(bIsActive);
	ThrusterRight->Activate(bIsActive);
}
