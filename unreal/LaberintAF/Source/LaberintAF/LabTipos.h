// Tipos comunes de Laberint AF: escala, filas de las tablas de datos (Content/Data/*.csv) y estados.
// Las distancias de las tablas están en unidades de la versión web (1 unidad = Lab::U centímetros), así el balance pasa tal cual.
#pragma once

#include "CoreMinimal.h"
#include "LabTipos.generated.h"

namespace Lab
{
	constexpr float U = 12.5f;            // cm por unidad de la versión web (un casillero de 16 = 2 m)
	constexpr float Tile = 16.f * U;      // 200 cm
	constexpr float AltoMuro = 320.f;
	constexpr float Vel = 0.55f;          // las velocidades web, en cm/s, se achican para que se vea creíble con personas reales
	inline float Cm(float Unidades) { return Unidades * U; }
	inline float CmS(float UnidadesPorSeg) { return UnidadesPorSeg * U * Vel; }
}

UENUM(BlueprintType)
enum class ELabGolpe : uint8 { Filo, Contundente, Distancia, Explosion, Otro };

UENUM(BlueprintType)
enum class ELabMiembro : uint8 { BrazoIzq, BrazoDer, PiernaIzq, PiernaDer };

// Armas.csv
USTRUCT(BlueprintType)
struct FLabArma
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Nombre;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Ranura;   // melee, ranged, lamp, helmet, ammo, throw
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Danio = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Alcance = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Barrido = 0;  // radianes
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Enfriamiento = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Nivel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Aturde = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ProbAturdir = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Empuje = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Velocidad = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Cargador = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Luz = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Armadura = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Genero;
};

// Botin.csv: peso de cada objeto al sortear lo que sale de un cofre
USTRUCT(BlueprintType)
struct FLabBotin
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Peso = 1;
};

// Rarezas.csv
USTRUCT(BlueprintType)
struct FLabRareza
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Multiplicador = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Color;
};

// Cazadores.csv (uno por nivel, 1 a 10)
USTRUCT(BlueprintType)
struct FLabCazador
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Nivel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Nombre;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Corto;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Aviso;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Descripcion;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Cantidad = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Vida = 100;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Danio = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Velocidad = 30;  // ya multiplicada por 1,3 como en la web
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Radio = 4;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Escala = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Enfriamiento = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Mascota;
};

// Bestias.csv (también los jefes)
USTRUCT(BlueprintType)
struct FLabBestia
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Nombre;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Vida = 30;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Danio = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Velocidad = 40;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Alerta = 100;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Oro = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Radio = 4;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool Jefe = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool Fantasma = false;
};

// HabilidadesCazadores.csv: cada habilidad de un cazador o jefe. Forma: Proyectil, Linea, Circulo, Cono, Embestida, Salto,
// Teletransporte, Invocar, Grito, Trampa, Torbellino. Flags (separados por |): Rastrea, Laser, Propio, EnObjetivo, Charco,
// CharcoFuego, Tela, Roca, ChoquePared, Persigue, Detras, Niebla, Cerca, Lejos, RobaVida, Mirada, Gancho, Radial, Furia, SoloAlfa
USTRUCT(BlueprintType)
struct FLabHabilidad
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Duenio;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Forma;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DistMin = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DistMax = 100;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Radio = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Largo = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Ancho = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Arco = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Aviso = 0;       // segundos que se ve la marca antes de pegar
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Danio = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Enfriamiento = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Aturde = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Enreda = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Frena = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Veneno = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Fuego = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Sangra = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Ciega = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Empuje = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Velocidad = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Cantidad = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float Duracion = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Invoca;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool RequiereVer = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Flags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Mensaje;

	bool Tiene(const TCHAR* Flag) const
	{
		TArray<FString> P; Flags.ParseIntoArray(P, TEXT("|"));
		return P.Contains(Flag);
	}
};

// Efectos de un golpe (los mismos de hitP en la web)
USTRUCT(BlueprintType)
struct FLabEfectos
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Aturde = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Enreda = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Frena = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Veneno = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Fuego = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Sangra = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Ciega = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Empuje = 0;   // unidades web

	static FLabEfectos De(const FLabHabilidad& H)
	{
		FLabEfectos E; E.Aturde = H.Aturde; E.Enreda = H.Enreda; E.Frena = H.Frena; E.Veneno = H.Veneno;
		E.Fuego = H.Fuego; E.Sangra = H.Sangra; E.Ciega = H.Ciega; E.Empuje = H.Empuje; return E;
	}
};
