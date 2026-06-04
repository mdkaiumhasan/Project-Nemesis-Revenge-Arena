// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NemesisCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChangedSignature, class ANemesisWeapon*, NewWeapon);

USTRUCT(BlueprintType)
struct FModularCharacterParts
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class USkeletalMesh* BaseMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class USkeletalMesh* HeadMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class USkeletalMesh* TorsoMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class USkeletalMesh* LegsMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class USkeletalMesh* HandsMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class USkeletalMesh* FeetMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class USkeletalMesh* VestMesh = nullptr;
};

UCLASS()
class PROJECTNEMESIS_API ANemesisCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANemesisCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for starting sprint */
	void StartSprint();

	/** Called for stopping sprint */
	void StopSprint();

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SprintAction;

	/** Locomotion settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SpeedInterpSpeed = 8.f;

	/** Stamina settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	float StaminaDrainRate = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	float StaminaRegenRate = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting = false;

	bool bSprintPressed = false;

	/** Decoy parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DecoyAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ANemesisDecoy> DecoyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	float DecoyStaminaCost = 30.f;

	/** Spawns a decoy clone */
	void SpawnDecoy();

	/** Replicated weapon the character is holding */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class ANemesisWeapon* CurrentWeapon;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	/** Default weapon class to spawn on start (USP) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ANemesisWeapon> DefaultWeaponClass;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

protected:
	/** Called for firing input */
	void OnFireTriggered();

	/** Called for reloading input */
	void OnReloadTriggered();

	/** Automated test-run helper to fire weapon */
	void ExecuteTestRunFire();

	/** Automated test-run helper to quit the game */
	void ExecuteTestRunQuit();

	/** Server RPC for executing fire on weapon */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	/** Server RPC for executing reload on weapon */
	UFUNCTION(Server, Reliable)
	void ServerReload();

	/** Server RPC to handle weapon purchase authoritatively */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBuyWeapon(TSubclassOf<class ANemesisWeapon> WeaponClass);

	/** Death sequence execution */
	void Die();

	/** Flag to check if character is dead */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsDead;

public:
	/** Override TakeDamage to route through attributes */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** Resets character state after a round ends */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetCharacterState(const FVector& NewLocation);

	/** Equips a weapon and attaches it to the character mesh */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EquipWeapon(class ANemesisWeapon* NewWeapon);

	/** Buys a weapon from the shop using coins */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BuyWeapon(TSubclassOf<class ANemesisWeapon> WeaponClass);

	/** Attribute Component managing Health, Stamina, and Coins */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
	class UNemesisAttributeComponent* Attributes;

	/* Modular parts for customization */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* TorsoMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* LegsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* HandsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* FeetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* VestMesh;

	/* Holographic 3D Wrist HUD Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* WristHUDComponent;

protected:
	/* Replicated state of character mesh parts */
	UPROPERTY(ReplicatedUsing = OnRep_CharacterParts, EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FModularCharacterParts CharacterParts;

	UFUNCTION()
	void OnRep_CharacterParts();

	void UpdateCharacterMeshes();

public:
	/* Server RPC to set the custom modular character parts authoritatively */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Customization")
	void ServerSetCharacterParts(const FModularCharacterParts& NewParts);

	/* Delegate fired when the character equips or swaps their weapon */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeaponChangedSignature OnWeaponChanged;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns Attributes subobject **/
	FORCEINLINE class UNemesisAttributeComponent* GetAttributes() const { return Attributes; }
	/** Returns modular parts subobjects **/
	FORCEINLINE class USkeletalMeshComponent* GetHeadMesh() const { return HeadMesh; }
	FORCEINLINE class USkeletalMeshComponent* GetTorsoMesh() const { return TorsoMesh; }
	FORCEINLINE class USkeletalMeshComponent* GetLegsMesh() const { return LegsMesh; }
	FORCEINLINE class USkeletalMeshComponent* GetHandsMesh() const { return HandsMesh; }
	FORCEINLINE class USkeletalMeshComponent* GetFeetMesh() const { return FeetMesh; }
	FORCEINLINE class USkeletalMeshComponent* GetVestMesh() const { return VestMesh; }
	/** Returns WristHUDComponent subobject **/
	FORCEINLINE class UWidgetComponent* GetWristHUDComponent() const { return WristHUDComponent; }
};
