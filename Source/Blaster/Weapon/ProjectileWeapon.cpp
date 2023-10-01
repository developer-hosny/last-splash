// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
// #include "Blaster/Helper/DebugHelper.h"

// AProjectileWeapon::AProjectileWeapon()
// {
// 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
// 	PrimaryActorTick.bCanEverTick = true;
// }
// void AProjectileWeapon::Tick(float DeltaTime)
// {
// 	Super::Tick(DeltaTime);
// 	// AWeapon::DoLineTraceStartFromWeapon();
// 	// const USkeletalMeshSocket *WaterSocket = GetWeaponMesh()->GetSocketByName(FName("WaterSocket"));
// 	// if (WaterSocket)
// 	// {
// 	// 	FTransform SocketTransform = WaterSocket->GetSocketTransform(AWeapon::GetWeaponMesh());
// 	// 	// if (SocketTransform)
// 	// 	// {
// 	// 	// 	// Debug::Print(SocketTransform);
// 	// 	// }
// 	// 	// DoLineTraceStartFromWeapon(SocketTransform);
// 	// }
// }

void AProjectileWeapon::Fire(const FVector &HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority())
		return;

	APawn *InstigatorPawn = Cast<APawn>(GetOwner());

	// const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	const USkeletalMeshSocket *WaterSocket = GetWeaponMesh()->GetSocketByName(FName("WaterSocket"));

	if (WaterSocket)
	{
		FTransform SocketTransform = WaterSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParms;
			SpawnParms.Owner = GetOwner();
			SpawnParms.Instigator = InstigatorPawn;

			UWorld *World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParms);
			}
		}
	}
}
