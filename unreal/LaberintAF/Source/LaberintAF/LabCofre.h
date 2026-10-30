// Cofres: común (callejones sin salida), rico (salas de tesoro), de oro (bóvedas: dos armas épicas o mejores) y legendario
// (salas de jefe: sellado con cadenas hasta que muere el jefe; trae un arma legendaria seguro). Como openChest/openTier de la web.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabCofre.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class ALabPersonaje;
class ALabEnemigo;

UENUM(BlueprintType)
enum class ELabCofre : uint8 { Comun, Rico, Oro, Leyenda };

UCLASS()
class LABERINTAF_API ALabCofre : public AActor
{
	GENERATED_BODY()

public:
	ALabCofre();

	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Caja;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Tapa;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> Brillo;
	UPROPERTY(BlueprintReadOnly) ELabCofre Clase = ELabCofre::Comun;
	UPROPERTY(BlueprintReadOnly) bool bAbierto = false;
	TWeakObjectPtr<ALabEnemigo> Sello;   // el jefe que lo tiene sellado

	void Configurar(ELabCofre NuevaClase, ALabEnemigo* Jefe = nullptr);
	bool Sellado() const;
	FString TextoAbrir() const;
	void Abrir(ALabPersonaje* Quien);

private:
	FName SortearArma(bool bRico) const;
	int32 SortearRareza(bool bRico) const;
};
