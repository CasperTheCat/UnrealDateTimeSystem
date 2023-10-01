// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#include "DateTimeReplicationActor.h"

#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

ADateTimeSystemReplication::ADateTimeSystemReplication()
{
    bAlwaysRelevant = true;
    // DateTimeComponent = CreateDefaultSubobject<UDateTimeSystemComponent>(TEXT("DateTimeRepComponent"));
}

void ADateTimeSystemReplication::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate current health.
    DOREPLIFETIME(ThisClass, UTCDate);
}

void ADateTimeSystemReplication::OnRep_DTSSync()
{
    // Client-specific functionality
    if (ROLE_Authority != GetLocalRole())
    {
        // Get the DTS subsystem.
        // Push the new values
    }

    // Server-specific functionality
    if (GetLocalRole() == ROLE_Authority)
    {
    }
}
