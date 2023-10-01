// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWater.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void AProjectileWater::OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
    ABlasterCharacter *OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        ABlasterPlayerController *OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
        if (OwnerController)
        {
            if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
            {
                // const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
                const float DamageToCause = Damage;
                UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
                // Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
                // return;
            }
        }
    }

    Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
