// Fill out your copyright notice in the Description page of Project Settings.


#include "NemesisDecoy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ANemesisDecoy::ANemesisDecoy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	LifeTime = 5.0f;
	DecoyHealth = 50.0f;

	// Configure movement settings for the decoy (running speed)
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	// Ignore camera collision on capsule and mesh components
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
}

// Called when the game starts or when spawned
void ANemesisDecoy::BeginPlay()
{
	Super::BeginPlay();
	
	// Start timer for self-destruction
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ANemesisDecoy::DestroyDecoy, LifeTime, false);
}

// Called every frame
void ANemesisDecoy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Move forward continuously
	AddMovementInput(GetActorForwardVector(), 1.0f);
}

float ANemesisDecoy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (DamageApplied <= 0.f) return 0.f;

	DecoyHealth = FMath::Clamp(DecoyHealth - DamageApplied, 0.f, 50.f);
	if (DecoyHealth <= 0.f)
	{
		DestroyDecoy();
	}

	return DamageApplied;
}

void ANemesisDecoy::DestroyDecoy()
{
	GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
	Destroy();
}
