// Fill out your copyright notice in the Description page of Project Settings.


#include "NemesisWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

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
	AttachSocketName = TEXT("Hand_R");
}

// Called when the game starts or when spawned
void ANemesisWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// Reset relative location and rotation of meshes to ensure they align perfectly with the root scene component
	if (WeaponMesh)
	{
		WeaponMesh->SetRelativeLocation(FVector::ZeroVector);
		WeaponMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (StaticWeaponMesh)
	{
		StaticWeaponMesh->SetRelativeLocation(FVector::ZeroVector);
		StaticWeaponMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}

	CurrentAmmo = MaxAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
}

void ANemesisWeapon::Fire()
{
	if (CurrentAmmo <= 0) return;

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner) return;

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

	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			// Apply damage using standard Unreal TakeDamage system
			FDamageEvent DamageEvent;
			HitActor->TakeDamage(Damage, DamageEvent, ControllerOwner, PawnOwner);
		}
	}

	CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
}

void ANemesisWeapon::Reload()
{
	CurrentAmmo = MaxAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
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
