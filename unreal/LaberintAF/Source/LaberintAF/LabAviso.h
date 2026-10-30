// Aviso en el piso (tele de la versión web): una marca roja con la forma del golpe que se va llenando; cuando se llena,
// pega a los jugadores que siguen adentro (la voltereta los salva) o corre lo que le pasen en AlTerminar.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabAviso.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class ALabPersonaje;

UENUM(BlueprintType)
enum class ELabFormaAviso : uint8 { Circulo, Linea, Cono, Nada };

UCLASS()
class LABERINTAF_API ALabAviso : public AActor
{
	GENERATED_BODY()

public:
	ALabAviso();

	// medidas en cm; Yaw en grados (para la línea y el cono)
	static ALabAviso* Crear(UWorld* Mundo, ELabFormaAviso Forma, const FVector& Lugar, float Yaw, float Radio, float Largo, float Ancho, float ArcoRad,
		float Duracion, FLinearColor Color, AActor* Duenio, TFunction<void(ALabAviso*)> AlTerminar);
	// algo que pasa un rato después, sin marca (setTimeout0 de la web)
	static void Despues(UWorld* Mundo, float Segundos, AActor* Duenio, TFunction<void(ALabAviso*)> Hacer);

	UPROPERTY(EditAnywhere, Category = "Aviso") TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UProceduralMeshComponent> Malla;

	ELabFormaAviso Forma = ELabFormaAviso::Circulo;
	float Radio = 100, Largo = 100, Ancho = 50, Arco = 1;
	float Duracion = 1, T = 0;
	FLinearColor Color = FLinearColor(1, .24f, .16f);
	TWeakObjectPtr<AActor> Duenio;
	TWeakObjectPtr<AActor> Seguir;     // la marca se mueve con él (golpe alrededor del que la tira)
	TWeakObjectPtr<AActor> Rastrear;   // apunta a él hasta FijarAntes segundos del final (mira láser)
	float FijarAntes = 0;
	TFunction<void(ALabAviso*)> AlTerminar;

	bool Toca(const FVector& P, float RadioCm) const;
	TArray<ALabPersonaje*> Alcanzados() const;   // jugadores adentro que no están rodando

	virtual void Tick(float Dt) override;

private:
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> MatFondo;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> MatLleno;
	void Dibujar(float Progreso);
	void Forma2D(float K, TArray<FVector>& V, TArray<int32>& Tri) const;
};
