// [TEMPLATE]

#pragma once

#include "DateTimeSystemComponent.h"
#include "ClimateComponent.h"

#include "Interfaces.generated.h"

// Interface
UINTERFACE(MinimalAPI, Blueprintable)
class UDateTimeSystemInterface : public UInterface
{
    GENERATED_BODY()
};

class DATETIMESYSTEM_API IDateTimeSystemInterface
{
    GENERATED_BODY()

public:
    /** Add interface function declarations here */
    UFUNCTION(BlueprintNativeEvent)
    UDateTimeSystemComponent *GetDateTimeSystem();
    virtual UDateTimeSystemComponent *GetDateTimeSystem_Implementation();
};

// ClimateInterface
UINTERFACE(MinimalAPI, Blueprintable)
class UDateTimeSystemClimateInterface : public UInterface
{
    GENERATED_BODY()
};

class DATETIMESYSTEM_API IDateTimeSystemClimateInterface
{
    GENERATED_BODY()

public:
    /** Add interface function declarations here */
    UFUNCTION(BlueprintNativeEvent)
    UClimateComponent *GetClimateComponent();
    virtual UClimateComponent *GetClimateComponent_Implementation();
};
