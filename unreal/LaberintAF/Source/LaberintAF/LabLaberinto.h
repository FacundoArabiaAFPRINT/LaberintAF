// El laberinto: misma generación que la versión web (genMaze): grilla de 19 x 19 celdas de 4 casilleros, pasillos de un
// laberinto perfecto con algunos atajos, sala central con la salida, salas de jefe, bóvedas, mercaderes, altares, tesoros y
// salones con columnas, y cuevas que rompen la cuadrícula. Construye muros y piso con instancias (una malla repetida).
// Para los gráficos realistas se cambian MallaMuro/MallaPiso y sus materiales por los de Megascans (ver unreal/MIGRACION.md).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabLaberinto.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ELabCelda : uint8 { Muro, Pasillo, Sala, Salida };

USTRUCT(BlueprintType)
struct FLabSala
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FString Tipo;   // exit, boss, vault, merchant, altar, treasure, hall
	UPROPERTY(BlueprintReadOnly) int32 CX = 0;
	UPROPERTY(BlueprintReadOnly) int32 CY = 0;
	UPROPERTY(BlueprintReadOnly) int32 W = 1;
	UPROPERTY(BlueprintReadOnly) int32 H = 1;
	UPROPERTY(BlueprintReadOnly) FVector Centro = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FVector Min = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FVector Max = FVector::ZeroVector;
	bool Contiene(const FVector& P) const { return P.X >= Min.X && P.X <= Max.X && P.Y >= Min.Y && P.Y <= Max.Y; }
};

UCLASS()
class LABERINTAF_API ALabLaberinto : public AActor
{
	GENERATED_BODY()

public:
	ALabLaberinto();

	// medidas de la grilla (como enumeración: MSVC no acepta constantes static en una clase exportada con LABERINTAF_API)
	enum : int32 { CW = 19, CH = 19, CS = 4, TW = CW * CS + 1, TH = CH * CS + 1, MID = CW / 2 };

	UPROPERTY(EditAnywhere, Category = "Laberinto") int32 Semilla = 0;   // 0 = una distinta cada partida
	UPROPERTY(EditAnywhere, Category = "Laberinto|Aspecto") TObjectPtr<UStaticMesh> MallaMuro;
	UPROPERTY(EditAnywhere, Category = "Laberinto|Aspecto") TObjectPtr<UStaticMesh> MallaPiso;
	UPROPERTY(EditAnywhere, Category = "Laberinto|Aspecto") TObjectPtr<UMaterialInterface> MaterialMuro;
	UPROPERTY(EditAnywhere, Category = "Laberinto|Aspecto") TObjectPtr<UMaterialInterface> MaterialPiso;

	UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Muros;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Piso;

	UPROPERTY(BlueprintReadOnly) TArray<FLabSala> Salas;
	UPROPERTY(BlueprintReadOnly) FVector Salida = FVector::ZeroVector;

	void Generar();
	void Construir();

	ELabCelda En(int32 X, int32 Y) const { return (X < 0 || Y < 0 || X >= TW || Y >= TH) ? ELabCelda::Muro : Grilla[Y * TW + X]; }
	bool EsSolido(const FVector& P) const;
	FVector Centro(int32 X, int32 Y) const;
	FIntPoint Casillero(const FVector& P) const;
	bool Visible(const FVector& A, const FVector& B) const;   // línea de vista sobre la grilla
	const FLabSala* SalaEn(const FVector& P) const;
	FVector PuntoLibre(TFunctionRef<bool(const FVector&)> Filtro, bool* Encontrado = nullptr);
	FVector PuntoLibreCerca(const FVector& Desde, float RadioCm);
	TArray<FVector> Camino(const FVector& Desde, const FVector& Hasta) const;   // BFS sobre los casilleros, como la web
	TArray<FIntPoint> CallejonesSinSalida() const;
	FRandomStream& Azar() { return Rng; }

private:
	TArray<ELabCelda> Grilla;
	FRandomStream Rng;
	void Pasillo(int32 CX, int32 CY);
	void Entre(int32 CX, int32 CY, int32 NX, int32 NY);
};
