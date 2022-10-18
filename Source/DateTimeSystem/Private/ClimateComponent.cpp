// [TEMPLATE_COPYRIGHT]

#include "ClimateComponent.h"

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
	if (MonthIndex < YearBook.Num())
	{
		return YearBook[MonthIndex]->MonthlyHighTemp;
	}
	return 0.0f;
}

float UClimateComponent::GetMonthlyLowTemperature(int MonthIndex)
{
	if (MonthIndex < YearBook.Num())
	{
		return YearBook[MonthIndex]->MonthlyLowTemp;
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
	if(DummyParentComponent)
	{

		auto SunVector = DummyParentComponent->GetSunVector(ReferenceLatitude, ReferenceLongitude);

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

		//auto FracDay = DummyParentComponent->GetFractionalDay(LocalTime);
		//auto Multi = FMath::Sin(PI * FracDay);
		//return FMath::Lerp(LowTemperature, HighTemperature, Multi);
	}

	return 0.f;

}



void UClimateComponent::UpdateCurrentTemperature(float DeltaTime)
{
	// Okay. Our Current Temp is going to be dumb.
	// Which low are we using? The first or last?
	if(DummyParentComponent)
	{
		auto FracDay = DummyParentComponent->GetFractionalDay(LocalTime);

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
	if (DummyParentComponent)
	{
		// Temperatures
		CachedNextLowTemp.Value = DailyLowModulation(DateStruct, asPtr->CallbackAttributes, asPtr->LowTemp, CachedLowTemp.Value, CachedHighTemp.Value);
		CachedHighTemp.Value = DailyHighModulation(DateStruct, asPtr->CallbackAttributes, asPtr->HighTemp, CachedLowTemp.Value, CachedHighTemp.Value);

		CachedNextLowTemp.Valid = true;
		CachedHighTemp.Valid = true;
	}

	// Date has changed
	// Call the BP function
	DateChanged(DateStruct);
}

float UClimateComponent::GetCurrentTemperature()
{
	return CurrentTemperature;
}

void UClimateComponent::InternalTick(float DeltaTime)
{
	if(DummyParentComponent)
	{
		// Update Local Time. We need it for a few things
		DummyParentComponent->GetTodaysDateTZ(LocalTime, TimezoneInfo);

		UpdateCurrentTemperature(DeltaTime);
	}
}

void UClimateComponent::InternalBegin()
{
	if (DummyParentComponent)
	{
		DummyParentComponent->DateChangeCallback.AddDynamic(this, DateChanged);
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
		if (DateStruct.Month < YearBook.Num() && DummyParentComponent)
		{
			// Which do we need. We need the fractional month value
			auto MonthFrac = DummyParentComponent->GetFractionalMonth(DateStruct);
			auto CurrentMonthHigh = YearBook[DateStruct.Month]->MonthlyHighTemp;
			auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

			if (MonthFrac > 0.5)
			{
				// Future Month
				auto OtherIndex = (DateStruct.Month + 1) % YearBook.Num();
				auto OtherValue = YearBook[OtherIndex]->MonthlyHighTemp;

				// High is lerp frac
				CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(CurrentMonthHigh, OtherValue, BlendFrac);
			}
			else
			{
				// Future Month
				auto OtherIndex = (YearBook.Num() + (DateStruct.Month - 1)) % YearBook.Num();
				auto OtherValue = YearBook[OtherIndex]->MonthlyHighTemp;

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
		if (DateStruct.Month < YearBook.Num() && DummyParentComponent)
		{
			// Which do we need. We need the fractional month value
			auto MonthFrac = DummyParentComponent->GetFractionalMonth(DateStruct);
			auto CurrentMonthLow = YearBook[DateStruct.Month]->MonthlyLowTemp;
			auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

			if (MonthFrac > 0.5)
			{
				// Future Month
				auto OtherIndex = (DateStruct.Month + 1) % YearBook.Num();
				auto OtherValue = YearBook[OtherIndex]->MonthlyLowTemp;

				// High is lerp frac
				CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(CurrentMonthLow, OtherValue, BlendFrac);
			}
			else
			{
				// Future Month
				auto OtherIndex = (YearBook.Num() + (DateStruct.Month - 1)) % YearBook.Num();
				auto OtherValue = YearBook[OtherIndex]->MonthlyLowTemp;

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

	if (DummyParentComponent)
	{
		// Check Override
		auto Row = DummyParentComponent->GetDateOverride(&DateStruct);
		if (Row)
		{
			auto asPtr = *Row;
			auto LV = FDateTimeSystemStruct::CreateFromRow(asPtr);

			CachedHighTemp.Value = DailyHighModulation(LV, asPtr->CallbackAttributes, asPtr->HighTemp, LastLowTemp, LastHighTemp);
			CachedHighTemp.Valid = true;
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

	// Hopefully, this gets noticed...
	return 1000.f;
}

float UClimateComponent::GetDailyLow(FDateTimeSystemStruct& DateStruct)
{
	// Okay. We need to work out which Low we actually want

	if (CachedNextLowTemp.Valid)
	{
		return CachedNextLowTemp.Value;
	}

	if(DummyParentComponent)
	{
		// Check Override
		auto Row = DummyParentComponent->GetDateOverride(&DateStruct);
		if (Row)
		{
			auto asPtr = *Row;
			auto LV = FDateTimeSystemStruct::CreateFromRow(asPtr);

			CachedNextLowTemp.Value = DailyLowModulation(LV, asPtr->CallbackAttributes, asPtr->LowTemp, LastLowTemp, LastHighTemp);
			CachedNextLowTemp.Valid = true;
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

	// We freezing a bit
	return -274.f;
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
