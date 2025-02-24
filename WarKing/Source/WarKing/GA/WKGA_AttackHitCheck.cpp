// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/WKGA_AttackHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GA/AT/WKAT_Trace.h"
#include "GA/TA/WKTA_Trace.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GA/WKGA_HitReact.h"
#include "Character/WKCharacterPlayer.h"
#include "Warking.h"
#include "Attribute/WKCharacterAttributeSet.h"
#include "Enum/WKTEnumToName.h"
#include "Tag/WKGameplayTag.h"

UWKGA_AttackHitCheck::UWKGA_AttackHitCheck()
{
	static ConstructorHelpers::FClassFinder<UGameplayEffect> StunEffectClassRef(TEXT("/Game/WarKing/Blueprint/GE/BPGE_Stun.BPGE_Stun_C"));

	if (StunEffectClassRef.Class)
	{
		StunEffect = StunEffectClassRef.Class;
	}
}

void UWKGA_AttackHitCheck::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("ActivateAbilityAttack========="));
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CurrentLevel = TriggerEventData->EventMagnitude;

	UWKAT_Trace* AttackTraceTask = UWKAT_Trace::CreateTask(this, TargetActorClass);
	AttackTraceTask->OnComplete.AddDynamic(this, &UWKGA_AttackHitCheck::OnTraceResultCallback);
	AttackTraceTask->ReadyForActivation();
}

void UWKGA_AttackHitCheck::OnTraceResultCallback(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{		
	if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetDataHandle, 0))
	{
		for (int32 i = 0; i < TargetDataHandle.Num(); ++i)
		{
			FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, i);
			UE_LOG(LogTemp, Log, TEXT("Target %s Detected"), *(HitResult.GetActor()->GetName()));

			UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo_Checked();
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor());

			if (!SourceASC || !TargetASC)
			{
				UE_LOG(LogTemp, Log, TEXT("UWKGA_AttackHitCheck::OnTraceResultCallback : ASC not Found!"));
				return;
			}

			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(AttackDamageEffect, CurrentLevel);

			if (EffectSpecHandle.IsValid())
			{
				FGameplayEffectContextHandle CueContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(EffectSpecHandle);
				CueContextHandle.AddHitResult(HitResult);
				FGameplayCueParameters CueParams;
				CueParams.EffectContext = CueContextHandle;

				if (!TargetASC->HasMatchingGameplayTag(WKTAG_EVENT_CHARACTER_ACTION_BLOCKATTACK))
				{
					ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);

					// 불검일 경우
					if (SourceASC->HasMatchingGameplayTag(WKTAG_CHARACTER_STATE_ISFLAMING))
					{
						if (!TargetASC->HasMatchingGameplayTag(WKTAG_GC_CHARACTER_BURN))
						{
							FGameplayEffectSpecHandle DotEffectSpecHandle = MakeOutgoingGameplayEffectSpec(AttackDotDamageEffect, CurrentLevel);
							ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DotEffectSpecHandle, TargetDataHandle);
						}
					}

					if (!TargetASC->HasMatchingGameplayTag(WKTAG_CHARACTER_STATE_ISDEAD) &&
						!TargetASC->HasMatchingGameplayTag(WKTAG_CHARACTER_ACTION_SKILL_AOE))
					{
						// HitReact
						AWKCharacterPlayer* TargetCharacter = Cast<AWKCharacterPlayer>(HitResult.GetActor());

						if (TargetCharacter)
						{
							FGameplayTag HitDirectionTag = TargetCharacter->GetHitReactDirection(HitResult.ImpactPoint);
							TargetCharacter->ServerPlayHitReact(HitDirectionTag);
						}

						TargetASC->ExecuteGameplayCue(WKTAG_GC_CHARACTER_ATTACKHIT, CueParams);
					}
				}
				else
				{
					// Block Attack GC 발동
					TargetASC->ExecuteGameplayCue(WKTAG_GC_CHARACTER_BLOCKATTACK, CueParams);
				}
			}

			// ComboAttack AttackRange Buff Effect
			FGameplayEffectSpecHandle BuffEffectSpecHandle = MakeOutgoingGameplayEffectSpec(AttackBuffEffect, CurrentLevel);

			if (BuffEffectSpecHandle.IsValid())
			{
				ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, BuffEffectSpecHandle);
			}
		}
	} 
	else if (UAbilitySystemBlueprintLibrary::TargetDataHasActor(TargetDataHandle, 0))
	{
		// Actors 정보가 있는지 확인		
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo_Checked();		 
		
		// StunEffect
		FGameplayEffectSpecHandle StunEffectSpecHandle = MakeOutgoingGameplayEffectSpec(StunEffect, CurrentLevel);

		if (StunEffectSpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, StunEffectSpecHandle, TargetDataHandle);
		}

		// DamageEffect
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(AttackDamageEffect, CurrentLevel);
	
		if (EffectSpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);

			FGameplayEffectContextHandle CueContextHandle = UAbilitySystemBlueprintLibrary::GetEffectContext(EffectSpecHandle);
			CueContextHandle.AddActors(TargetDataHandle.Data[0].Get()->GetActors(), false);

			FGameplayCueParameters CueParam;
			CueParam.EffectContext = CueContextHandle;
			SourceASC->ExecuteGameplayCue(WKTAG_GC_CHARACTER_ATTACKHIT, CueParam);
		}
	}


	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}


