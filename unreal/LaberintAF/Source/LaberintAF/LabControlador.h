#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LabControlador.generated.h"

UCLASS()
class LABERINTAF_API ALabControlador : public APlayerController
{
	GENERATED_BODY()

public:
	ALabControlador()
	{
		bShowMouseCursor = true;
		DefaultMouseCursor = EMouseCursor::Crosshairs;
	}
};
