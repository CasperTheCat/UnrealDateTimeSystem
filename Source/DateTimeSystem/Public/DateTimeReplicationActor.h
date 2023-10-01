// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DateTimeSystemComponent.h"

#include "DateTimeReplicationActor.generated.h"

/**
 * @brief DateTimeSystemComponent
 *
 * This actor is owned by the server. It's job is to replicate DTS variables.
 *
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent),
       DisplayName = "Date Time System")
class DATETIMESYSTEM_API ADateTimeSystemReplication : public AActor
{
    GENERATED_BODY()

protected:
    // UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
    // TObjectPtr<UDateTimeSystemComponent> DateTimeComponent;

    UPROPERTY(Replicated, ReplicatedUsing = OnRep_DTSSync)
    FDateTimeSystemStruct UTCDate;

public:
    ADateTimeSystemReplication();

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

public:
    UFUNCTION()
    void OnRep_DTSSync();
};
