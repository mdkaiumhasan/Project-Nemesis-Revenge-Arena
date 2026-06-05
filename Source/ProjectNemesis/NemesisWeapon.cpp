// Fill out your copyright notice in the Description page of Project Settings.


#include "NemesisWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"

// Sets default values
ANemesisWeapon::ANemesisWeapon()
{
 	// We do not need Tick for the weapon
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = SceneRoot;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	StaticWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticWeaponMesh"));
	StaticWeaponMesh->SetupAttachment(RootComponent);
	StaticWeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	WeaponCost = 500;
	Damage = 20.f;
	FireRate = 0.15f;
	Range = 10000.f;
	MaxAmmo = 30;
	CurrentAmmo = MaxAmmo;
	AttachSocketName = TEXT("hand_r");

	// Load default sound using ObjectFinder
	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundFinder(TEXT("/Game/SoulCity/Sound/Cue/Footstep_Metal_Cue.Footstep_Metal_Cue"));
	if (FireSoundFinder.Succeeded())
	{
		FireSound = FireSoundFinder.Object;
	}

	// Load default particles using ObjectFinder
	static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleFlashFinder(TEXT("/Game/ParagonLtBelica/FX/Particles/Belica/Abilities/Primary/FX/P_BelicaMuzzle.P_BelicaMuzzle"));
	if (MuzzleFlashFinder.Succeeded())
	{
		MuzzleFlash = MuzzleFlashFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactEffectFinder(TEXT("/Game/ParagonLtBelica/FX/Particles/Belica/Abilities/Primary/FX/P_BelicaHitWorld.P_BelicaHitWorld"));
	if (ImpactEffectFinder.Succeeded())
	{
		ImpactEffect = ImpactEffectFinder.Object;
	}
}

// Called when the game starts or when spawned
void ANemesisWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentAmmo = MaxAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
}

void ANemesisWeapon::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: ANemesisWeapon::Fire called. Actor: %s, CurrentAmmo: %d, MaxAmmo: %d, Owner: %s"), 
		*GetName(), CurrentAmmo, MaxAmmo, GetOwner() ? *GetOwner()->GetName() : TEXT("None"));

	if (CurrentAmmo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Cannot fire - No Ammo remaining!"));
		return;
	}

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Cannot fire - PawnOwner is null!"));
		return;
	}

	AController* ControllerOwner = PawnOwner->GetController();
	if (!ControllerOwner) return;

	// Perform Line Trace from Player Viewpoint (Camera)
	FVector EyeLocation;
	FRotator EyeRotation;
	ControllerOwner->GetPlayerViewPoint(EyeLocation, EyeRotation);

	FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * Range);

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(PawnOwner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams);

	// Visual Feedback: Draw a bright red tracer line from the weapon's location to the target/impact point
	FVector TracerStart = GetActorLocation();
	FVector TracerEnd = bHit ? Hit.ImpactPoint : TraceEnd;
	DrawDebugLine(GetWorld(), TracerStart, TracerEnd, FColor::Red, false, 0.2f, 0, 2.0f);

	// Play gunshot sound at gun location
	USoundBase* SoundToPlay = FireSound;
	if (!SoundToPlay)
	{
		SoundToPlay = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, TEXT("/Game/SoulCity/Sound/Cue/Footstep_Metal_Cue.Footstep_Metal_Cue")));
	}
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, TracerStart);
	}

	// Spawn AAA Paragon muzzle flash attached to the active weapon component
	if (MuzzleFlash)
	{
		if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset())
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, TEXT("Muzzle"));
		}
		else if (StaticWeaponMesh && StaticWeaponMesh->GetStaticMesh())
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, StaticWeaponMesh, TEXT("Muzzle"));
		}
		else
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, TracerStart, GetActorRotation());
		}
	}

	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			// Apply damage using standard Unreal TakeDamage system
			FDamageEvent DamageEvent;
			HitActor->TakeDamage(Damage, DamageEvent, ControllerOwner, PawnOwner);
		}

		// Draw a debug sphere at impact point
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 12.f, 8, FColor::Red, false, 0.5f);

		// Spawn AAA Paragon impact visual effects at the hit location
		if (ImpactEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}
	}

	CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);

	// On-screen message showing ammo count
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, FString::Printf(TEXT("Weapon Fired! Ammo: %d / %d"), CurrentAmmo, MaxAmmo));
	}
}

void ANemesisWeapon::Reload()
{
	CurrentAmmo = MaxAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);

	// On-screen message showing reload
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Weapon Reloaded!"));
	}
}

void ANemesisWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANemesisWeapon, CurrentAmmo);
}

void ANemesisWeapon::OnRep_CurrentAmmo()
{
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
}
