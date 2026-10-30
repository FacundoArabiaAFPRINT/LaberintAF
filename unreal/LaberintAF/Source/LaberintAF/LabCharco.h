// Zonas que quedan en el piso: charco de veneno, fuego (solo quema jugadores), telaraña que frena y el cepo de Roberto.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabCharco.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UENUM(BlueprintType)
enum class ELabCharco : uint8 { Veneno, Fuego, Tela, Cepo };

UCLASS()
class LABERINTAF_API ALabCharco : public AActor
{
	GENERATED_BODY()

public:
	ALabCharco();
	static ALabCharco* Crear(UWorld* Mundo, ELabCharco Tipo, const FVector& Lugar, float RadioCm, float Duracion, AActor* Duenio);

	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Malla;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> Luz;

	ELabCharco Tipo = ELabCharco::Veneno;
	float Radio = 150, Duracion = 4;
	bool bArmado = true;   // el cepo, hasta que alguien lo pisa
	TWeakObjectPtr<AActor> Duenio;

	virtual void Tick(float Dt) override;
};
