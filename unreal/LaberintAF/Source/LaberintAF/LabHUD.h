// Interfaz provisoria dibujada con el lienzo del HUD (sin assets): reloj, farmeo, vida, aire, oro, llave, avisos, barra del
// jefe y pantalla final. La definitiva se hace con UMG en el estilo de historieta de la versión web (ver MIGRACION.md).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LabHUD.generated.h"

UCLASS()
class LABERINTAF_API ALabHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void Barra(float X, float Y, float W, float H, float F, FLinearColor Col);
	void Texto(const FString& T, float X, float Y, FLinearColor Col, float Escala, bool bCentrado = false);
};
