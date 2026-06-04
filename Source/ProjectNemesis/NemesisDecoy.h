// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NemesisDecoy.generated.h"

UCLASS()
class PROJECTNEMESIS_API ANemesisDecoy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANemesisDecoy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Override TakeDamage to handle bullet absorption
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	/* Decoy life duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoy Settings")
	float LifeTime;

	/* Decoy health value to absorb damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoy Settings")
	float DecoyHealth;

	/* Timer handle for self destruction */
	FTimerHandle DestroyTimerHandle;

	/* Destroys the decoy */
	void DestroyDecoy();
};
