// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HronoSharedTools.generated.h"

/**
 

 #include "HronoSharedTools.h"
 
 
 
 */

 /** Represents which timeline dimension an item or character belongs to */
UENUM(BlueprintType)
enum class EItemTimeline : uint8
{
	Past    UMETA(DisplayName = "Past"),
	Future  UMETA(DisplayName = "Future"),
	Both    UMETA(DisplayName = "Both")
};

