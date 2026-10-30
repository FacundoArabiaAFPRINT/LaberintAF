// Lee las tablas de Content/Data (CSV exportados de la versión web con tools/exportar-datos.mjs) y las deja a mano.
// Se leen como archivos de texto, sin importarlas como assets: así se pueden regenerar desde la web cuando se rebalancea.
// La primera fila tiene los nombres de los campos (iguales a los de los USTRUCT de LabTipos.h); la primera columna es el nombre.
#pragma once

#include "CoreMinimal.h"
#include "LabTipos.h"
#include "UObject/UnrealType.h"

class LABERINTAF_API FLabDatos
{
public:
	static FLabDatos& Get();

	TMap<FName, FLabArma> Armas;
	TMap<FName, FLabBotin> Botin;
	TMap<FName, FLabRareza> Rarezas;
	TMap<FName, FLabCazador> Cazadores;
	TMap<FName, FLabBestia> Bestias;
	TArray<TPair<FName, FLabHabilidad>> Habilidades;

	const FLabCazador* CazadorDeNivel(int32 Nivel) const;
	TArray<const FLabHabilidad*> HabilidadesDe(const FString& Duenio) const;

	// Separa una línea de CSV respetando las comillas
	static TArray<FString> PartirLinea(const FString& Linea);

	// Carga un CSV en un mapa de estructuras usando la reflexión de Unreal (cada columna va al campo de igual nombre)
	template <typename T>
	static void Cargar(const FString& Archivo, TMap<FName, T>& Out)
	{
		TArray<TPair<FName, T>> Filas; CargarFilas<T>(Archivo, Filas);
		for (auto& F : Filas) Out.Add(F.Key, F.Value);
	}

	template <typename T>
	static void CargarFilas(const FString& Archivo, TArray<TPair<FName, T>>& Out)
	{
		TArray<FString> Lineas;
		if (!LeerLineas(Archivo, Lineas) || Lineas.Num() < 2) return;
		const TArray<FString> Campos = PartirLinea(Lineas[0]);
		UScriptStruct* S = T::StaticStruct();
		for (int32 i = 1; i < Lineas.Num(); ++i)
		{
			if (Lineas[i].TrimStartAndEnd().IsEmpty()) continue;
			const TArray<FString> V = PartirLinea(Lineas[i]);
			if (V.Num() == 0) continue;
			T Fila;
			for (int32 c = 1; c < Campos.Num() && c < V.Num(); ++c)
			{
				if (FProperty* P = S->FindPropertyByName(FName(*Campos[c])))
				{
					FString Txt = V[c];
					if (P->IsA<FBoolProperty>()) Txt = Txt.Equals(TEXT("true"), ESearchCase::IgnoreCase) ? TEXT("True") : TEXT("False");
					if (Txt.IsEmpty() && !P->IsA<FStrProperty>()) continue;
					P->ImportText_Direct(*Txt, P->ContainerPtrToValuePtr<void>(&Fila), nullptr, PPF_None);
				}
			}
			Out.Add(TPair<FName, T>(FName(*V[0]), Fila));
		}
	}

private:
	FLabDatos();
	static bool LeerLineas(const FString& Archivo, TArray<FString>& Lineas);
};
