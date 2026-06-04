// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NemesisWeapon.generated.h"

UCLASS()
class PROJECTNEMESIS_API ANemesisWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANemesisWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	/* Weapon Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* WeaponMesh;

	/* Weapon Static Mesh (for weapons using static meshes) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StaticWeaponMesh;

	/* Weapon cost in shop */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	int32 WeaponCost;

	/* Damage dealt per shot */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float Damage;

	/* Time delay between shots (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float FireRate;

	/* Maximum tracing distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float Range;

	/* Maximum ammo capacity in magazine */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	int32 MaxAmmo;

	/* Current ammo remaining */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	int32 CurrentAmmo;

	/* Socket name to attach to player mesh */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FName AttachSocketName;

	/* Perform line trace and damage target (called on server) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Fire();

	/* Refills the magazine */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Reload();

	/* Returns the weapon mesh */
	FORCEINLINE class USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	/* Returns the static weapon mesh */
	FORCEINLINE class UStaticMeshComponent* GetStaticWeaponMesh() const { return StaticWeaponMesh; }

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
