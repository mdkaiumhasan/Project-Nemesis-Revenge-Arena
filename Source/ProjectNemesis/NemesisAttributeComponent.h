// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NemesisAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedSignature, float, CurrentVal, float, MaxVal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinsChangedSignature, int32, CurrentCoins);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTNEMESIS_API UNemesisAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNemesisAttributeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/* Health Properties */
	UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere, BlueprintReadWrite, Category = "Attributes|Health")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes|Health")
	float MaxHealth;

	/* Stamina Properties */
	UPROPERTY(ReplicatedUsing = OnRep_Stamina, EditAnywhere, BlueprintReadWrite, Category = "Attributes|Stamina")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes|Stamina")
	float MaxStamina;

	/* Coin Properties */
	UPROPERTY(ReplicatedUsing = OnRep_Coins, EditAnywhere, BlueprintReadWrite, Category = "Attributes|Economy")
	int32 Coins;

	/* Events */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttributeChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCoinsChangedSignature OnCoinsChanged;

	/* Helper Functions */
	UFUNCTION(BlueprintCallable, Category = "Attributes|Health")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Attributes|Health")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Attributes|Stamina")
	void ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Attributes|Stamina")
	void RegenStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Attributes|Economy")
	void AddCoins(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Attributes|Economy")
	bool UseCoins(int32 Amount);

protected:
	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_Stamina();

	UFUNCTION()
	void OnRep_Coins();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
