// Proyectiles de cazadores y jefes: perdigones, kunais, escupidas, telarañas, fuego, murciélagos, el gancho del Carnicero,
// cuchillas y calaveras. Vuelan en línea recta (o persiguen), chocan con los muros y aplican los efectos al jugador.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabTipos.h"
#include "LabProyectil.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class LABERINTAF_API ALabProyectil : public AActor
{
	GENERATED_BODY()

public:
	ALabProyectil();

	static ALabProyectil* Disparar(AActor* Duenio, float Yaw, float VelocidadCm, float Vida, float Danio, const FLabEfectos& Efectos, const FString& Flags, FLinearColor Color, AActor* Objetivo = nullptr);

	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Malla;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> Luz;

	FVector Vel = FVector::ZeroVector;
	float Vida = 1, Danio = 0;
	FLabEfectos Efectos;
	FString Flags;
	TWeakObjectPtr<AActor> Duenio;
	TWeakObjectPtr<AActor> Objetivo;

	virtual void Tick(float Dt) override;

private:
	void Terminar();
	bool Tiene(const TCHAR* F) const { TArray<FString> P; Flags.ParseIntoArray(P, TEXT("|")); return P.Contains(F); }
};
