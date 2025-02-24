﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/WKGASPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Attribute//WKCharacterAttributeSet.h"
#include "Character/WKCharacterBase.h"
#include "Tag/WKGameplayTag.h"
#include "Net/UnrealNetwork.h"

AWKGASPlayerState::AWKGASPlayerState()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	// 원본에서 복제본으로 계속 전송이 되어야 하므로 속성 설정
	ASC->SetIsReplicated(true);

	// Mixed모드로 설정해야 네트워크 대역폭에서 최고의 성능을 얻을 수 있다.
	// Mixed Mode : GameplayTag 및 GameplayCues가 모든 사람들에게 복제됨
	// + 최소한의 게임플레이 효과 정보만 SimulatedProxy에 복제, 전체 정보는 Owner, AutonomousPorxy에 복제
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UWKCharacterAttributeSet>(TEXT("AttributeSet"));
	SkillAttributeSet = CreateDefaultSubobject<UWKCharacterSkillAttributeSet>(TEXT("SkillAttributeSet"));
	
	// 값 조정 필요
	NetUpdateFrequency = 100.f;

	TeamTag = WKTAG_GAME_TEAM_NONE;
}

void AWKGASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWKGASPlayerState, TeamTag);
	DOREPLIFETIME(AWKGASPlayerState, WKCharacter);
}

UAbilitySystemComponent* AWKGASPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

UWKCharacterAttributeSet* AWKGASPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

UWKCharacterSkillAttributeSet* AWKGASPlayerState::GetSkillAttributeSet() const
{
	return SkillAttributeSet;
}

void AWKGASPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AWKGASPlayerState::HealthChanged);
		MaxHealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddUObject(this, &AWKGASPlayerState::MaxHealthChanged);
	}

	WKCharacter = Cast <AWKCharacterBase>(GetPawn());
}

void AWKGASPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	float Health = Data.NewValue;

	if ((Health < 1.0f) && !ASC->HasMatchingGameplayTag(WKTAG_CHARACTER_STATE_ISDEAD))
	{
		// 죽으면 해당 Target에는 IsDead Tag가 부착
		ASC->AddLooseGameplayTag(WKTAG_CHARACTER_STATE_ISDEAD);
		OnOutOfHealth.Broadcast();
	}
}

void AWKGASPlayerState::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
}

void AWKGASPlayerState::OnRep_Team()
{
	if (WKCharacter)
	{
		WKCharacter->SetTeamColor(TeamTag);
	}
}

void AWKGASPlayerState::SetTeam(FGameplayTag TeamToSet)
{
	WKCharacter = WKCharacter == nullptr ? Cast<AWKCharacterBase>(GetPawn()) : WKCharacter;

	if (TeamTag == WKTAG_GAME_TEAM_NONE)
	{
		TeamTag = TeamToSet;
		ASC->AddLooseGameplayTag(TeamToSet);
		ASC->AddReplicatedLooseGameplayTag(TeamToSet);
	}

	if (WKCharacter)
	{
		WKCharacter->SetTeamColor(TeamToSet);
	}
}


