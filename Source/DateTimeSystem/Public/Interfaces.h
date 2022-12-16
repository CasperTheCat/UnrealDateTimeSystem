// [TEMPLATE]

#pragma once

#include "DateTimeSystemComponent.h"

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
