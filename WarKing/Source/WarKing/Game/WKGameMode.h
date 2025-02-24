// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WKGameMode.generated.h"
namespace MatchState
{
	extern WARKING_API const FName Cooldown;
}
/**
 * 
 */
UCLASS()
class WARKING_API AWKGameMode : public AGameMode
{
	GENERATED_BODY()
	
private:
	AWKGameMode();

	virtual void Tick(float DeltaTime) override;
	// Network Check Session
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void StartPlay() override;
	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;
	virtual void InitSeamlessTravelPlayer(AController* NewController) override;
	virtual void OnMatchStateSet() override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	void DesytoySession();
	void SetTeam(APlayerState* WKPlayerState);

	UFUNCTION()
	void OnSetWinnerTeam();

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

public:
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	virtual void HandleMatchHasStarted() override;
	void CheckDestroySession();
private:
	float CountdownTime = 0.f;
	class AWKGameState* WKGameState;
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	FORCEINLINE float GetMatchTime() const { return MatchTime; }
	FORCEINLINE float GetWarmupTime() const { return WarmupTime; }
	FORCEINLINE float GetCooldownTime() const { return CooldownTime; }
	FORCEINLINE float GetLevelStartingTime() const { return LevelStartingTime; }
};
