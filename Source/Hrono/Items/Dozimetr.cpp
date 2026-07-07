#include "Items/Dozimetr.h"
#include "HronoCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

static bool IsOwnedByRemoteClient(ABase_Item* Item)
{
	if (AHronoCharacter* Char = Cast<AHronoCharacter>(Item->OwningCharacter))
	{
		if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
		{
			// True if this PC is controlled on a different machine than the one running this code
			return !PC->IsLocalController();
		}
	}
	return false;
}

void ADozimetr::On()
{
	// Runs here directly — i.e. on whichever machine called this (the server,
	// since OnPickedUp() is gated behind HasAuthority()). Covers dedicated server
	// AND the listen-server host's own local player.
	ReceiveOn();
	UGameplayStatics::PlaySoundAtLocation(this, TurnOnSound, GetActorLocation());

	// Only RPC out if the owner is a genuinely remote client — otherwise the
	// listen-server host would run ReceiveOn() twice (once above, once via RPC).
	if (IsOwnedByRemoteClient(this))
	{
		Client_On();
	}
}

void ADozimetr::Off()
{
	ReceiveOff();
	UGameplayStatics::PlaySoundAtLocation(this, TurnOffSound, GetActorLocation());

	if (IsOwnedByRemoteClient(this))
	{
		Client_Off();
	}
}

void ADozimetr::Client_On_Implementation()
{
	ReceiveOn();
	UGameplayStatics::PlaySoundAtLocation(this, TurnOnSound, GetActorLocation());
}

void ADozimetr::Client_Off_Implementation()
{
	ReceiveOff();
	UGameplayStatics::PlaySoundAtLocation(this, TurnOffSound, GetActorLocation());
}