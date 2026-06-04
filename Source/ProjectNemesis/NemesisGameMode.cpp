// Fill out your copyright notice in the Description page of Project Settings.


#include "NemesisGameMode.h"
#include "NemesisCharacter.h"
#include "NemesisAttributeComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ANemesisGameMode::ANemesisGameMode()
{
	CurrentRound = 1;
	Player1Wins = 0;
	Player2Wins = 0;
	NewRoundDelay = 4.0f;

	// Set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ANemesisGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ANemesisGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	if (NewPlayer)
	{
		APlayerController* PC = Cast<APlayerController>(NewPlayer);
		if (PC)
		{
			ConnectedPlayers.AddUnique(PC);
		}
	}
}

void ANemesisGameMode::OnPlayerDeath(ACharacter* KilledPlayer)
{
	if (!KilledPlayer) return;

	APlayerController* KilledPC = Cast<APlayerController>(KilledPlayer->GetController());
	if (!KilledPC) return;

	// Find the surviving player
	APlayerController* WinnerPC = nullptr;
	for (APlayerController* PC : ConnectedPlayers)
	{
		if (PC && PC != KilledPC)
		{
			WinnerPC = PC;
			break;
		}
	}

	// Update wins and pay coins
	if (WinnerPC)
	{
		// Identify who won
		if (ConnectedPlayers.Num() > 0 && WinnerPC == ConnectedPlayers[0])
		{
			Player1Wins++;
			UE_LOG(LogTemp, Warning, TEXT("Player 1 Wins Round %d! Total Wins: %d"), CurrentRound, Player1Wins);
		}
		else if (ConnectedPlayers.Num() > 1 && WinnerPC == ConnectedPlayers[1])
		{
			Player2Wins++;
			UE_LOG(LogTemp, Warning, TEXT("Player 2 Wins Round %d! Total Wins: %d"), CurrentRound, Player2Wins);
		}

		// Award Coins to Winner
		APawn* WinnerPawn = WinnerPC->GetPawn();
		if (WinnerPawn)
		{
			ANemesisCharacter* WinnerChar = Cast<ANemesisCharacter>(WinnerPawn);
			if (WinnerChar && WinnerChar->GetAttributes())
			{
				WinnerChar->GetAttributes()->AddCoins(1000);
			}
		}
	}

	// Award Coins to Loser
	if (KilledPC)
	{
		ANemesisCharacter* LoserChar = Cast<ANemesisCharacter>(KilledPlayer);
		if (LoserChar && LoserChar->GetAttributes())
		{
			LoserChar->GetAttributes()->AddCoins(600);
		}
	}

	// Check if match over (Best of 5 -> first to 3 wins)
	if (Player1Wins >= 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("MATCH OVER: PLAYER 1 WINS THE MATCH!"));
		return;
	}
	else if (Player2Wins >= 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("MATCH OVER: PLAYER 2 WINS THE MATCH!"));
		return;
	}

	// Start timer for next round
	CurrentRound++;
	GetWorldTimerManager().SetTimer(RoundResetTimerHandle, this, &ANemesisGameMode::StartNewRound, NewRoundDelay, false);
}

void ANemesisGameMode::StartNewRound()
{
	ResetPlayers();
}

void ANemesisGameMode::ResetPlayers()
{
	int32 RoundIndex = CurrentRound - 1;

	// Reset Player 1
	if (ConnectedPlayers.Num() > 0 && ConnectedPlayers[0])
	{
		APlayerController* PC = ConnectedPlayers[0];
		APawn* PlayerPawn = PC->GetPawn();
		if (PlayerPawn)
		{
			ANemesisCharacter* Character = Cast<ANemesisCharacter>(PlayerPawn);
			if (Character)
			{
				FVector SpawnLoc = RoundSpawnPointsPlayer1.IsValidIndex(RoundIndex) ? RoundSpawnPointsPlayer1[RoundIndex] : FVector(0.f, 0.f, 100.f);
				Character->ResetCharacterState(SpawnLoc);
			}
		}
	}

	// Reset Player 2
	if (ConnectedPlayers.Num() > 1 && ConnectedPlayers[1])
	{
		APlayerController* PC = ConnectedPlayers[1];
		APawn* PlayerPawn = PC->GetPawn();
		if (PlayerPawn)
		{
			ANemesisCharacter* Character = Cast<ANemesisCharacter>(PlayerPawn);
			if (Character)
			{
				FVector SpawnLoc = RoundSpawnPointsPlayer2.IsValidIndex(RoundIndex) ? RoundSpawnPointsPlayer2[RoundIndex] : FVector(500.f, 0.f, 100.f);
				Character->ResetCharacterState(SpawnLoc);
			}
		}
	}
}
