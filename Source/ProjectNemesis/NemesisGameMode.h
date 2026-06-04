// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NemesisGameMode.generated.h"

UCLASS()
class PROJECTNEMESIS_API ANemesisGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANemesisGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnPostLogin(AController* NewPlayer) override;

	/* Called when a player character dies */
	UFUNCTION(BlueprintCallable, Category = "Nemesis Rules")
	void OnPlayerDeath(ACharacter* KilledPlayer);

protected:
	/* List of active players in the game (expected max 2) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nemesis Rules")
	TArray<APlayerController*> ConnectedPlayers;

	/* Current round number (1 to 5) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nemesis Rules")
	int32 CurrentRound;

	/* Wins for Player 1 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nemesis Rules")
	int32 Player1Wins;

	/* Wins for Player 2 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nemesis Rules")
	int32 Player2Wins;

	/* Round Spawn Locations for Player 1 (Indexes 0 to 4 represent Rounds 1 to 5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nemesis Rules")
	TArray<FVector> RoundSpawnPointsPlayer1;

	/* Round Spawn Locations for Player 2 (Indexes 0 to 4 represent Rounds 1 to 5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nemesis Rules")
	TArray<FVector> RoundSpawnPointsPlayer2;

	/* Delay in seconds before starting a new round after a death */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nemesis Rules")
	float NewRoundDelay;

	/* Starts a new round */
	void StartNewRound();

	/* Resets players and spawns them at the round location */
	void ResetPlayers();

	/* Timer handle for round reset delay */
	FTimerHandle RoundResetTimerHandle;
};
