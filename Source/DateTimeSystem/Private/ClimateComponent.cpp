// [TEMPLATE_COPYRIGHT]

#include "ClimateComponent.h"
#include "Interfaces.h"
#include "GameFramework/GameState.h"

void UClimateComponent::ClimateSetup()
{
}

void UClimateComponent::Invalidate(EDateTimeSystemInvalidationTypes Type = EDateTimeSystemInvalidationTypes::Frame)
{
	if (Type >= EDateTimeSystemInvalidationTypes::Month)
	{

	}

	// Not cleared on frame-to-frame invalidation
	if (Type >= EDateTimeSystemInvalidationTypes::Day)
	{
		CachedHighTemp.Valid = false;
		CachedLowTemp.Valid = false;
		CachedNextLowTemp.Valid = false;
		CachedAnalyticalMonthlyHighTemp.Valid = false;
		CachedAnalyticalMonthlyLowTemp.Valid = false;
	}
}

FDateTimeSystemStruct UClimateComponent::GetLocalTime()
{
	return LocalTime;
}

float UClimateComponent::GetMonthlyHighTemperature(int MonthIndex)
{
	if (MonthIndex < ClimateBook.Num())
	{
		return ClimateBook[MonthIndex]->MonthlyHighTemp;
	}
	return 0.0f;
}

float UClimateComponent::GetMonthlyLowTemperature(int MonthIndex)
{
	if (MonthIndex < ClimateBook.Num())
	{
		return ClimateBook[MonthIndex]->MonthlyLowTemp;
	}
	return 0.0f;
}

float UClimateComponent::DailyLowModulation_Implementation(UPARAM(ref)FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh)
{
	return Temperature;
}

float UClimateComponent::DailyHighModulation_Implementation(UPARAM(ref)FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh)
{
	return Temperature;
}

float UClimateComponent::ModulateTemperature_Implementation(float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature)
{
	// Get the sun vector. We want to suppress the temperature when it's below the horizon
	if(DateTimeSystem)
	{

		auto SunVector = DateTimeSystem->GetSunVector(ReferenceLatitude, ReferenceLongitude);

		if (FVector::DotProduct(SunVector, FVector::UpVector) > 0)
		{
			// Sun is above the horizon
			// We want to start Lerping to high temperature slowly
			return FMath::FInterpTo(Temperature, HighTemperature, SecondsSinceUpdate, 0.04);
		}
		else
		{
			return FMath::FInterpTo(Temperature, LowTemperature, SecondsSinceUpdate, 0.04);
		}

		//auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);
		//auto Multi = FMath::Sin(PI * FracDay);
		//return FMath::Lerp(LowTemperature, HighTemperature, Multi);
	}

	return 0.f;

}



void UClimateComponent::UpdateCurrentTemperature(float DeltaTime)
{
	// Okay. Our Current Temp is going to be dumb.
	// Which low are we using? The first or last?
	if(DateTimeSystem)
	{
		auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);

		auto HighTemp = GetDailyHigh(LocalTime);
		auto LowTemp = CachedLowTemp.Value;
		if (FracDay > 0.5)
		{
			// Need today's low, rather than the prior
			LowTemp = GetDailyLow(LocalTime);
		}

		CurrentTemperature = ModulateTemperature(CurrentTemperature, DeltaTime, LowTemp, HighTemp);
	}
}

void UClimateComponent::InternalDateChanged(FDateTimeSystemStruct& DateStruct)
{
	// Handle what was once done on InternalTick
	Invalidate(EDateTimeSystemInvalidationTypes::Day);

	// Handle rolling the starting temp of day n+1 to the ending of n
	CachedLowTemp.Value = CachedNextLowTemp.Value;
	CachedLowTemp.Valid = true;
	LastHighTemp = CachedHighTemp.Value;
	LastLowTemp = CachedLowTemp.Value;

	auto Row = DateOverrides.Find(GetTypeHash(DateStruct));
	if (Row)
	{
		auto asPtr = *Row;
		if (asPtr)
		{
			// Temperatures
			CachedNextLowTemp.Value = DailyLowModulation(DateStruct, asPtr->MiscData, asPtr->LowTemp, CachedLowTemp.Value, CachedHighTemp.Value);
			CachedHighTemp.Value = DailyHighModulation(DateStruct, asPtr->MiscData, asPtr->HighTemp, CachedLowTemp.Value, CachedHighTemp.Value);
			CachedNextLowTemp.Valid = true;
			CachedHighTemp.Valid = true;
		}
	}
	else
	{
		// Go ahead and compute the new values
		CachedAnalyticalMonthlyHighTemp.Value = GetAnalyticalHighForDate(DateStruct);
		CachedAnalyticalMonthlyHighTemp.Valid = true;
		CachedAnalyticalMonthlyLowTemp.Value = GetAnalyticalLowForDate(DateStruct);
		CachedAnalyticalMonthlyLowTemp.Valid = true;

		auto DummyTagContainer = FGameplayTagContainer();

		CachedNextLowTemp.Value = DailyLowModulation(DateStruct, DummyTagContainer, CachedAnalyticalMonthlyLowTemp.Value, CachedLowTemp.Value, CachedHighTemp.Value);
		CachedNextLowTemp.Valid = true;
		CachedHighTemp.Value = DailyHighModulation(DateStruct, DummyTagContainer, CachedAnalyticalMonthlyHighTemp.Value, CachedLowTemp.Value, CachedHighTemp.Value);
		CachedHighTemp.Valid = true;
	}


	

	// Date has changed
	// Call the BP function
	DateChanged(DateStruct);
}

UDateTimeSystemComponent* UClimateComponent::FindComponent()
{
	auto World = GetWorld();
	if (World)
	{
		auto GameInst = World->GetGameInstance();
		if (GameInst)
		{
			// Try to get from here
			auto AsType = Cast<IDateTimeSystemInterface>(GameInst);
			if (AsType)
			{
				// 
				return AsType->GetDateTimeSystem();
			}
		}

		// If we're here, Instance failed to get the time system.
		auto GameState = World->GetGameState();
		if (GameState)
		{
			auto AsType = Cast<IDateTimeSystemInterface>(GameState);
			if (AsType)
			{
				return AsType->GetDateTimeSystem();
			}
		}
	}

	return nullptr;
}

float UClimateComponent::GetCurrentTemperature()
{
	return CurrentTemperature;
}

void UClimateComponent::InternalTick(float DeltaTime)
{
	Invalidate(EDateTimeSystemInvalidationTypes::Frame);

	if(DateTimeSystem)
	{
		// Update Local Time. We need it for a few things
		DateTimeSystem->GetTodaysDateTZ(LocalTime, TimezoneInfo);

		UpdateCurrentTemperature(DeltaTime);
	}
}

void UClimateComponent::InternalBegin()
{
	// Let's go
	if (ClimateTable)
	{
		ClimateTable->GetAllRows<FDateTimeSystemClimateMonthlyRow>(FString("Climate Rows"), ClimateBook);
	}

	if (ClimateOverridesTable)
	{
		ClimateOverridesTable->GetAllRows<FDateTimeSystemClimateOverrideRow>(FString("Climate Rows"), DOTemps);

		for (auto val : DOTemps)
		{
			DateOverrides.Add(GetTypeHash(val), val);
		}
	}

	// Try to Find DateTime
	DateTimeSystem = FindComponent();

	if (DateTimeSystem)
	{
		DateTimeSystem->DateChangeCallback.AddDynamic(this, &UClimateComponent::DateChanged);

		FDateTimeSystemStruct Today;
		DateTimeSystem->GetTodaysDateTZ(Today, TimezoneInfo);

		// Set Prevalue
		CachedLowTemp.Value = GetDailyLow(Today);
		CachedLowTemp.Valid = true;
	}
}

void UClimateComponent::DateChanged_Implementation(FDateTimeSystemStruct& DateStruct)
{
}

float UClimateComponent::GetAnalyticalHighForDate(FDateTimeSystemStruct& DateStruct)
{
	// We have two things to do here. Return the cache, if it's valid
	if (CachedAnalyticalMonthlyHighTemp.Valid)
	{
		return CachedAnalyticalMonthlyHighTemp.Value;
	}
	else
	{
		// We need 
		if (DateStruct.Month < ClimateBook.Num() && DateTimeSystem)
		{
			// Which do we need. We need the fractional month value
			auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
			auto CurrentMonthHigh = ClimateBook[DateStruct.Month]->MonthlyHighTemp;
			auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

			if (MonthFrac > 0.5)
			{
				// Future Month
				auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
				auto OtherValue = ClimateBook[OtherIndex]->MonthlyHighTemp;

				// High is lerp frac
				CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(CurrentMonthHigh, OtherValue, BlendFrac);
			}
			else
			{
				// Future Month
				auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
				auto OtherValue = ClimateBook[OtherIndex]->MonthlyHighTemp;

				// High is lerp frac
				CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(OtherValue, CurrentMonthHigh, BlendFrac);
			}
			CachedAnalyticalMonthlyHighTemp.Valid = true;
			return CachedAnalyticalMonthlyHighTemp.Value;
		}
	}
	return 0.0f;
}

float UClimateComponent::GetAnalyticalLowForDate(FDateTimeSystemStruct& DateStruct)
{
	// We have two things to do here. Return the cache, if it's valid
	if (CachedAnalyticalMonthlyLowTemp.Valid)
	{
		return CachedAnalyticalMonthlyLowTemp.Value;
	}
	else
	{
		// We need 
		if (DateStruct.Month < ClimateBook.Num() && DateTimeSystem)
		{
			// Which do we need. We need the fractional month value
			auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
			auto CurrentMonthLow = ClimateBook[DateStruct.Month]->MonthlyLowTemp;
			auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

			if (MonthFrac > 0.5)
			{
				// Future Month
				auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
				auto OtherValue = ClimateBook[OtherIndex]->MonthlyLowTemp;

				// High is lerp frac
				CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(CurrentMonthLow, OtherValue, BlendFrac);
			}
			else
			{
				// Future Month
				auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
				auto OtherValue = ClimateBook[OtherIndex]->MonthlyLowTemp;

				// High is lerp frac
				CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(OtherValue, CurrentMonthLow, BlendFrac);
			}
			CachedAnalyticalMonthlyLowTemp.Valid = true;
			return CachedAnalyticalMonthlyLowTemp.Value;
		}
	}
	return 0.0f;
}

float UClimateComponent::GetDailyHigh(FDateTimeSystemStruct& DateStruct)
{
	if (CachedHighTemp.Valid)
	{
		return CachedHighTemp.Value;
	}

	// Check Override
	auto Row = DateOverrides.Find(GetTypeHash(DateStruct));
	if (Row)
	{
		auto asPtr = *Row;
		if(asPtr)
		{
			CachedHighTemp.Value = DailyHighModulation(DateStruct, asPtr->MiscData, asPtr->HighTemp, LastLowTemp, LastHighTemp);
			CachedHighTemp.Valid = true;
		}
	}
	else
	{
		// Go ahead and compute the new values
		CachedAnalyticalMonthlyHighTemp.Value = GetAnalyticalHighForDate(DateStruct);
		CachedAnalyticalMonthlyHighTemp.Valid = true;

		auto DummyTagContainer = FGameplayTagContainer();

		CachedHighTemp.Value = DailyHighModulation(DateStruct, DummyTagContainer, CachedAnalyticalMonthlyHighTemp.Value, LastLowTemp, LastHighTemp);
		CachedHighTemp.Valid = true;
	}

	return CachedHighTemp.Value;
}

float UClimateComponent::GetDailyLow(FDateTimeSystemStruct& DateStruct)
{
	// Okay. We need to work out which Low we actually want

	if (CachedNextLowTemp.Valid)
	{
		return CachedNextLowTemp.Value;
	}

	auto Row = DateOverrides.Find(GetTypeHash(DateStruct));
	if (Row)
	{
		auto asPtr = *Row;
		if (asPtr)
		{
			CachedNextLowTemp.Value = DailyLowModulation(DateStruct, asPtr->MiscData, asPtr->LowTemp, LastLowTemp, LastHighTemp);
			CachedHighTemp.Valid = true;
		}
	}
	else
	{
		// Go ahead and compute the new values
		CachedAnalyticalMonthlyLowTemp.Value = GetAnalyticalLowForDate(DateStruct);
		CachedAnalyticalMonthlyLowTemp.Valid = true;

		auto DummyTagContainer = FGameplayTagContainer();

		CachedNextLowTemp.Value = DailyLowModulation(DateStruct, DummyTagContainer, CachedAnalyticalMonthlyLowTemp.Value, LastLowTemp, LastHighTemp);
		CachedNextLowTemp.Valid = true;
	}

	return CachedNextLowTemp.Value;
}

UClimateComponent::UClimateComponent()
{
	ClimateSetup();
}

UClimateComponent::UClimateComponent(UClimateComponent& Other)
{
	ClimateSetup();
}

UClimateComponent::UClimateComponent(const FObjectInitializer& ObjectInitializer)
{
	ClimateSetup();
}

