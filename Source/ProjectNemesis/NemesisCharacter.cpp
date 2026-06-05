// Fill out your copyright notice in the Description page of Project Settings.


#include "NemesisCharacter.h"
#include "NemesisAttributeComponent.h"
#include "NemesisDecoy.h"
#include "NemesisWeapon.h"
#include "NemesisGameMode.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/CommandLine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMesh.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputCoreTypes.h"

// Sets default values
ANemesisCharacter::ANemesisCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsDead = false;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); // ...at this rotation rate

	// Standard physics feel parameters
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->MaxAcceleration = 1500.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true; // Enable camera lag for smooth damping
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 15.f;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of spring arm
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create the attributes component
	Attributes = CreateDefaultSubobject<UNemesisAttributeComponent>(TEXT("Attributes"));

	// Create modular skeletal mesh components
	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh());

	TorsoMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TorsoMesh"));
	TorsoMesh->SetupAttachment(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh"));
	HandsMesh->SetupAttachment(GetMesh());

	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh"));
	FeetMesh->SetupAttachment(GetMesh());

	VestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VestMesh"));
	VestMesh->SetupAttachment(GetMesh());

	// Ignore camera collision on capsule and all character meshes to prevent camera zoom-in glitches
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	if (HeadMesh) HeadMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	if (TorsoMesh) TorsoMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	if (LegsMesh) LegsMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	if (HandsMesh) HandsMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	if (FeetMesh) FeetMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	if (VestMesh) VestMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Set default weapon class using class finder
	static ConstructorHelpers::FClassFinder<ANemesisWeapon> WeaponClassFinder(TEXT("/Game/Blueprints/Weapons/BP_Weapon_USP.BP_Weapon_USP_C"));
	if (WeaponClassFinder.Class != nullptr)
	{
		DefaultWeaponClass = WeaponClassFinder.Class;
	}

	// Create and configure Holographic 3D Wrist HUD Component
	WristHUDComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WristHUDComponent"));
	if (WristHUDComponent && GetMesh())
	{
		WristHUDComponent->SetupAttachment(GetMesh(), TEXT("Wrist_L_Socket"));
		WristHUDComponent->SetWidgetSpace(EWidgetSpace::World);
		WristHUDComponent->SetDrawSize(FVector2D(250.f, 150.f));
		WristHUDComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WristHUDComponent->SetCastShadow(false);
	}

	// Load default fire montage using ObjectFinder
	static ConstructorHelpers::FObjectFinder<UAnimMontage> FireMontageFinder(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage.MM_Pistol_Fire_Montage"));
	if (FireMontageFinder.Succeeded())
	{
		FireMontage = FireMontageFinder.Object;
	}
}

// Called when the game starts or when spawned
void ANemesisCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Load and set the initial/default character meshes
	UpdateCharacterMeshes();

	// Fallback to load default mapping context and core input actions if they are null
	if (DefaultMappingContext == nullptr)
	{
		DefaultMappingContext = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default")));
		if (DefaultMappingContext)
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Dynamically loaded DefaultMappingContext in BeginPlay"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("NEMESIS_DEBUG: Failed to load DefaultMappingContext in BeginPlay"));
		}
	}

	if (MoveAction == nullptr)
	{
		MoveAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Move.IA_Move")));
	}

	if (LookAction == nullptr)
	{
		LookAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook")));
		if (LookAction == nullptr)
		{
			LookAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Look.IA_Look")));
		}
	}

	// Dynamic warnings for optional input action assets
	if (SprintAction == nullptr)
	{
		SprintAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Sprint.IA_Sprint")));
		if (!SprintAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: IA_Sprint asset not found at /Game/Input/Actions/IA_Sprint. Please create it in the editor."));
		}
	}

	if (DecoyAction == nullptr)
	{
		DecoyAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Decoy.IA_Decoy")));
		if (!DecoyAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: IA_Decoy asset not found at /Game/Input/Actions/IA_Decoy. Please create it in the editor."));
		}
	}

	if (FireAction == nullptr)
	{
		FireAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Fire.IA_Fire")));
		if (!FireAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: IA_Fire asset not found at /Game/Input/Actions/IA_Fire. Please create it in the editor."));
		}
	}

	if (ReloadAction == nullptr)
	{
		ReloadAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Reload.IA_Reload")));
		if (!ReloadAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: IA_Reload asset not found at /Game/Input/Actions/IA_Reload. Please create it in the editor."));
		}
	}

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Added DefaultMappingContext in BeginPlay"));

			// Also add MouseLook mapping context if it exists
			UInputMappingContext* MouseLookContext = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook")));
			if (MouseLookContext)
			{
				Subsystem->AddMappingContext(MouseLookContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Added IMC_MouseLook in BeginPlay"));
			}
		}
	}

	// Fallback to load default weapon class dynamically if it was reset to null in the editor defaults
	if (DefaultWeaponClass == nullptr)
	{
		DefaultWeaponClass = StaticLoadClass(ANemesisWeapon::StaticClass(), nullptr, TEXT("/Game/Blueprints/Weapons/BP_Weapon_USP.BP_Weapon_USP_C"));
		if (DefaultWeaponClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Dynamically loaded DefaultWeaponClass in BeginPlay"));
		}
	}

	// Fallback to load default fire montage dynamically if it was reset to null in editor defaults
	if (FireMontage == nullptr)
	{
		FireMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage.MM_Pistol_Fire_Montage")));
		if (FireMontage)
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Dynamically loaded default FireMontage in BeginPlay"));
		}
	}

	// Spawn and equip default weapon (server only)
	UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: BeginPlay - HasAuthority: %s, DefaultWeaponClass: %s"), 
		HasAuthority() ? TEXT("True") : TEXT("False"), 
		DefaultWeaponClass ? *DefaultWeaponClass->GetName() : TEXT("Null"));

	if (HasAuthority() && DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ANemesisWeapon* DefaultWeapon = GetWorld()->SpawnActor<ANemesisWeapon>(DefaultWeaponClass, SpawnParams);
		if (DefaultWeapon)
		{
			EquipWeapon(DefaultWeapon);
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: Successfully spawned and equipped weapon: %s"), *DefaultWeapon->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: Failed to spawn weapon of class: %s"), *DefaultWeaponClass->GetName());
		}
	}

	// Schedule automated test execution if -testrun is specified in command line
	if (FCString::Strstr(FCommandLine::Get(), TEXT("testrun")) != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: Test Run command active! Scheduling automated fire..."));
		FTimerHandle TestTimerHandle;
		GetWorldTimerManager().SetTimer(TestTimerHandle, this, &ANemesisCharacter::ExecuteTestRunFire, 3.0f, false);
	}
}

// Called every frame
void ANemesisCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if character is moving horizontally (ignoring Z axis)
	const bool bIsMoving = GetVelocity().Size2D() > 10.f;

	// Check if we should sprint: must press sprint key, be moving, and have stamina in our Attribute Component
	if (Attributes)
	{
		if (bSprintPressed && bIsMoving && Attributes->Stamina > 0.f)
		{
			bIsSprinting = true;
			Attributes->ConsumeStamina(StaminaDrainRate * DeltaTime);
		}
		else
		{
			bIsSprinting = false;
			Attributes->RegenStamina(StaminaRegenRate * DeltaTime);
		}
	}

	// Dynamic momentum-based locomotion speed transition
	const float TargetSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, TargetSpeed, DeltaTime, SpeedInterpSpeed);
	}

	// Bulletproof input polling fallback for Fire and Reload
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton) || PC->WasInputKeyJustPressed(EKeys::RightMouseButton))
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Mouse Button Press detected in Tick fallback. Triggering fire..."));
			OnFireTriggered();
		}
		if (PC->WasInputKeyJustPressed(EKeys::R))
		{
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: R Key Press detected in Tick fallback. Triggering reload..."));
			OnReloadTriggered();
		}
	}
}

// Called to bind functionality to input
void ANemesisCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Fallback to load default mapping context and core input actions if they are null
	if (DefaultMappingContext == nullptr)
	{
		DefaultMappingContext = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default")));
	}
	if (MoveAction == nullptr)
	{
		MoveAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Move.IA_Move")));
	}
	if (LookAction == nullptr)
	{
		LookAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook")));
		if (LookAction == nullptr)
		{
			LookAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, TEXT("/Game/Input/Actions/IA_Look.IA_Look")));
		}
	}

	// Add Input Mapping Context for the local player possessed pawn
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Added DefaultMappingContext in SetupPlayerInputComponent"));

			// Also add MouseLook mapping context if it exists
			UInputMappingContext* MouseLookContext = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook")));
			if (MouseLookContext)
			{
				Subsystem->AddMappingContext(MouseLookContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: Added IMC_MouseLook in SetupPlayerInputComponent"));
			}
		}
	}

	// Legacy / Direct Key Fallback bindings to ensure movement works even without Enhanced Input Action assets
	PlayerInputComponent->BindAxis(TEXT("Turn Right / Left Mouse"), this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis(TEXT("Look Up / Down Mouse"), this, &APawn::AddControllerPitchInput);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANemesisCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANemesisCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ANemesisCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ANemesisCharacter::StopSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ANemesisCharacter::StopSprint);

		// Decoy Skill
		EnhancedInputComponent->BindAction(DecoyAction, ETriggerEvent::Started, this, &ANemesisCharacter::SpawnDecoy);

		// Weapon Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ANemesisCharacter::OnFireTriggered);

		// Weapon Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ANemesisCharacter::OnReloadTriggered);
	}
}

void ANemesisCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// Get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANemesisCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANemesisCharacter::StartSprint()
{
	bSprintPressed = true;
}

void ANemesisCharacter::StopSprint()
{
	bSprintPressed = false;
}

void ANemesisCharacter::SpawnDecoy()
{
	if (!Attributes || !DecoyClass) return;

	// Check if we have enough stamina
	if (Attributes->Stamina >= DecoyStaminaCost)
	{
		// Consume stamina
		Attributes->ConsumeStamina(DecoyStaminaCost);

		// Calculate spawn transform
		FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f; // Spawn 100 units in front to avoid overlapping
		FRotator SpawnRotation = GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Spawn the decoy
		GetWorld()->SpawnActor<ANemesisDecoy>(DecoyClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
}

void ANemesisCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANemesisCharacter, CurrentWeapon);
	DOREPLIFETIME(ANemesisCharacter, CharacterParts);
}

void ANemesisCharacter::EquipWeapon(ANemesisWeapon* NewWeapon)
{
	if (!NewWeapon) return;

	if (HasAuthority())
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->Destroy();
		}

		CurrentWeapon = NewWeapon;
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->SetInstigator(GetInstigator());

		OnRep_CurrentWeapon();
	}
}

void ANemesisCharacter::OnRep_CurrentWeapon()
{
	if (CurrentWeapon)
	{
		USkeletalMeshComponent* CharacterMesh = GetMesh();
		if (CharacterMesh)
		{
			CurrentWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeapon->AttachSocketName);
		}
	}
	OnWeaponChanged.Broadcast(CurrentWeapon);
}

void ANemesisCharacter::PlayFireMontage()
{
	if (FireMontage && !bIsDead)
	{
		float Duration = PlayAnimMontage(FireMontage);
		UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: PlayFireMontage. Montage: %s, Mesh: %s, AnimInstance: %s, Duration: %f"), 
			*FireMontage->GetName(),
			GetMesh() && GetMesh()->GetSkeletalMeshAsset() ? *GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("Null"),
			GetMesh() && GetMesh()->GetAnimInstance() ? *GetMesh()->GetAnimInstance()->GetName() : TEXT("Null"),
			Duration);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: PlayFireMontage skipped. Montage: %s, IsDead: %s"), 
			FireMontage ? *FireMontage->GetName() : TEXT("Null"), 
			bIsDead ? TEXT("True") : TEXT("False"));
	}
}

void ANemesisCharacter::OnFireTriggered()
{
	UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: ANemesisCharacter::OnFireTriggered called."));
	
	// Play locally for instant prediction
	PlayFireMontage();

	ServerFire();
}

void ANemesisCharacter::OnReloadTriggered()
{
	UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: ANemesisCharacter::OnReloadTriggered called."));
	ServerReload();
}

bool ANemesisCharacter::ServerFire_Validate()
{
	return true;
}

void ANemesisCharacter::ServerFire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("NEMESIS_DEBUG: ANemesisCharacter::ServerFire_Implementation. CurrentWeapon: %s"), CurrentWeapon ? *CurrentWeapon->GetName() : TEXT("None"));
	if (CurrentWeapon)
	{
		CurrentWeapon->Fire();
	}
}

void ANemesisCharacter::ServerReload_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

float ANemesisCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;

	float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (Attributes)
	{
		Attributes->ApplyDamage(DamageAmount);
		if (Attributes->Health <= 0.f)
		{
			Die();
		}
	}

	return DamageApplied;
}

void ANemesisCharacter::Die()
{
	if (bIsDead) return;

	bIsDead = true;

	// Disable Input
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		DisableInput(PC);
	}

	// Disable Capsule Collision
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// Enable Ragdoll Physics on mesh
	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetSimulatePhysics(true);
	}

	// Destroy held weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	// Notify GameMode
	if (HasAuthority())
	{
		ANemesisGameMode* GM = Cast<ANemesisGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnPlayerDeath(this);
		}
	}
}

void ANemesisCharacter::ResetCharacterState(const FVector& NewLocation)
{
	bIsDead = false;

	// Reset ragdoll physics
	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		// Re-attach mesh relative to root capsule
		GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f)); // Standard capsule offset
	}

	// Re-enable Capsule Collision
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // Ignore camera collision to prevent zoom glitches
	}

	// Teleport to new spawn location
	TeleportTo(NewLocation, GetActorRotation());

	// Reset attributes
	if (Attributes)
	{
		Attributes->Health = Attributes->MaxHealth;
		Attributes->Stamina = Attributes->MaxStamina;
		// Broadcast updates to UI listeners
		Attributes->OnHealthChanged.Broadcast(Attributes->Health, Attributes->MaxHealth);
		Attributes->OnStaminaChanged.Broadcast(Attributes->Stamina, Attributes->MaxStamina);
	}

	// Re-enable Input
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		EnableInput(PC);
	}

	// Re-spawn USP weapon (server only)
	if (HasAuthority() && DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ANemesisWeapon* DefaultWeapon = GetWorld()->SpawnActor<ANemesisWeapon>(DefaultWeaponClass, SpawnParams);
		if (DefaultWeapon)
		{
			EquipWeapon(DefaultWeapon);
		}
	}
}

void ANemesisCharacter::BuyWeapon(TSubclassOf<ANemesisWeapon> WeaponClass)
{
	ServerBuyWeapon(WeaponClass);
}

bool ANemesisCharacter::ServerBuyWeapon_Validate(TSubclassOf<ANemesisWeapon> WeaponClass)
{
	return WeaponClass != nullptr;
}

void ANemesisCharacter::ServerBuyWeapon_Implementation(TSubclassOf<ANemesisWeapon> WeaponClass)
{
	if (!Attributes || !WeaponClass) return;

	// Query CDO to find the cost of the weapon class
	ANemesisWeapon* WeaponCDO = WeaponClass->GetDefaultObject<ANemesisWeapon>();
	if (!WeaponCDO) return;

	int32 Cost = WeaponCDO->WeaponCost;

	// Attempt to deduct the coins
	if (Attributes->UseCoins(Cost))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		ANemesisWeapon* PurchasedWeapon = GetWorld()->SpawnActor<ANemesisWeapon>(WeaponClass, SpawnParams);
		if (PurchasedWeapon)
		{
			EquipWeapon(PurchasedWeapon);
		}
	}
}

void ANemesisCharacter::ExecuteTestRunFire()
{
	UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: Simulating weapon fire..."));
	OnFireTriggered();
	
	if (CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: Weapon equipped: %s, Current Ammo: %d / %d"), 
			*CurrentWeapon->GetClass()->GetName(), CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: No weapon equipped!"));
	}
	
	FTimerHandle QuitTimerHandle;
	GetWorldTimerManager().SetTimer(QuitTimerHandle, this, &ANemesisCharacter::ExecuteTestRunQuit, 2.0f, false);
}

void ANemesisCharacter::ExecuteTestRunQuit()
{
	UE_LOG(LogTemp, Warning, TEXT("NEMESIS_TEST: Quitting game..."));
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void ANemesisCharacter::UpdateCharacterMeshes()
{
	USkeletalMeshComponent* MainMesh = GetMesh();
	if (!MainMesh) return;

	// Set skeletal meshes on the components
	MainMesh->SetSkeletalMeshAsset(CharacterParts.BaseMesh);
	
	if (HeadMesh) HeadMesh->SetSkeletalMeshAsset(CharacterParts.HeadMesh);
	if (TorsoMesh) TorsoMesh->SetSkeletalMeshAsset(CharacterParts.TorsoMesh);
	if (LegsMesh) LegsMesh->SetSkeletalMeshAsset(CharacterParts.LegsMesh);
	if (HandsMesh) HandsMesh->SetSkeletalMeshAsset(CharacterParts.HandsMesh);
	if (FeetMesh) FeetMesh->SetSkeletalMeshAsset(CharacterParts.FeetMesh);
	if (VestMesh) VestMesh->SetSkeletalMeshAsset(CharacterParts.VestMesh);

	// Sync animations with the MainMesh using LeaderPoseComponent
	if (HeadMesh) HeadMesh->SetLeaderPoseComponent(MainMesh);
	if (TorsoMesh) TorsoMesh->SetLeaderPoseComponent(MainMesh);
	if (LegsMesh) LegsMesh->SetLeaderPoseComponent(MainMesh);
	if (HandsMesh) HandsMesh->SetLeaderPoseComponent(MainMesh);
	if (FeetMesh) FeetMesh->SetLeaderPoseComponent(MainMesh);
	if (VestMesh) VestMesh->SetLeaderPoseComponent(MainMesh);

	// Re-attach the current weapon to the new mesh sockets to handle replication/initialization order issues on clients
	if (CurrentWeapon)
	{
		OnRep_CurrentWeapon();
	}
}

void ANemesisCharacter::OnRep_CharacterParts()
{
	UpdateCharacterMeshes();
}

bool ANemesisCharacter::ServerSetCharacterParts_Validate(const FModularCharacterParts& NewParts)
{
	return true;
}

void ANemesisCharacter::ServerSetCharacterParts_Implementation(const FModularCharacterParts& NewParts)
{
	CharacterParts = NewParts;
	UpdateCharacterMeshes();
}


