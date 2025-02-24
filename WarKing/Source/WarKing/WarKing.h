// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_SUBLOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetLocalRole()))
#define LOG_SUBREMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetRemoteRole()))
#define LOG_GASSUBLOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), CurrentActorInfo->AvatarActor.Get()->GetOwner()->GetLocalRole()))
#define LOG_GASSUBREMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), CurrentActorInfo->AvatarActor.Get()->GetOwner()->GetRemoteRole()))

#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER"))) 
#define LOG_GASNETMODEINFO ((CurrentActorInfo->AvatarActor.Get()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((CurrentActorInfo->AvatarActor.Get()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER"))) 

#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define WK_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define WKGAS_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_GASNETMODEINFO, LOG_GASSUBLOCALROLEINFO, LOG_GASSUBREMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))


DECLARE_LOG_CATEGORY_EXTERN(LogWKNetwork, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWKGAS, Log, All);