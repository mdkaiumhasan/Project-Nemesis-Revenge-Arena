// Fill out your copyright notice in the Description page of Project Settings.


#include "NemesisAttributeComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UNemesisAttributeComponent::UNemesisAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	MaxHealth = 100.f;
	Health = MaxHealth;

	MaxStamina = 100.f;
	Stamina = MaxStamina;

	Coins = 0;
}


// Called when the game starts
void UNemesisAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// Broadcast starting values to initialize any listeners/UI
	OnHealthChanged.Broadcast(Health, MaxHealth);
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
	OnCoinsChanged.Broadcast(Coins);
}

void UNemesisAttributeComponent::ApplyDamage(float DamageAmount)
{
	if (DamageAmount <= 0.f) return;

	Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void UNemesisAttributeComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.f) return;

	Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void UNemesisAttributeComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.f) return;

	Stamina = FMath::Clamp(Stamina - Amount, 0.f, MaxStamina);
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void UNemesisAttributeComponent::RegenStamina(float Amount)
{
	if (Amount <= 0.f) return;

	Stamina = FMath::Clamp(Stamina + Amount, 0.f, MaxStamina);
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void UNemesisAttributeComponent::AddCoins(int32 Amount)
{
	if (Amount <= 0) return;

	Coins += Amount;
	OnCoinsChanged.Broadcast(Coins);
}

bool UNemesisAttributeComponent::UseCoins(int32 Amount)
{
	if (Amount <= 0) return false;

	if (Coins >= Amount)
	{
		Coins -= Amount;
		OnCoinsChanged.Broadcast(Coins);
		return true;
	}

	return false;
}

void UNemesisAttributeComponent::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void UNemesisAttributeComponent::OnRep_Stamina()
{
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void UNemesisAttributeComponent::OnRep_Coins()
{
	OnCoinsChanged.Broadcast(Coins);
}

void UNemesisAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNemesisAttributeComponent, Health);
	DOREPLIFETIME(UNemesisAttributeComponent, Stamina);
	DOREPLIFETIME(UNemesisAttributeComponent, Coins);
}
