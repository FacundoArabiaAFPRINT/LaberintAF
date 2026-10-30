#include "LabDatos.h"
#include "LaberintAF.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FLabDatos& FLabDatos::Get()
{
	static FLabDatos Datos;
	return Datos;
}

FLabDatos::FLabDatos()
{
	Cargar(TEXT("Armas.csv"), Armas);
	Cargar(TEXT("Botin.csv"), Botin);
	Cargar(TEXT("Rarezas.csv"), Rarezas);
	Cargar(TEXT("Cazadores.csv"), Cazadores);
	Cargar(TEXT("Bestias.csv"), Bestias);
	CargarFilas(TEXT("HabilidadesCazadores.csv"), Habilidades);
	UE_LOG(LogLab, Log, TEXT("Datos: %d armas, %d cazadores, %d bestias, %d habilidades"), Armas.Num(), Cazadores.Num(), Bestias.Num(), Habilidades.Num());
}

bool FLabDatos::LeerLineas(const FString& Archivo, TArray<FString>& Lineas)
{
	const FString Ruta = FPaths::ProjectContentDir() / TEXT("Data") / Archivo;
	FString Texto;
	if (!FFileHelper::LoadFileToString(Texto, *Ruta))
	{
		UE_LOG(LogLab, Error, TEXT("No encuentro %s (¿se exportaron los datos con tools/exportar-datos.mjs?)"), *Ruta);
		return false;
	}
	Texto.ReplaceInline(TEXT("\r"), TEXT(""));
	Texto.ParseIntoArray(Lineas, TEXT("\n"), true);
	return true;
}

TArray<FString> FLabDatos::PartirLinea(const FString& Linea)
{
	TArray<FString> Out;
	FString Actual;
	bool Comillas = false;
	for (int32 i = 0; i < Linea.Len(); ++i)
	{
		const TCHAR C = Linea[i];
		if (Comillas)
		{
			if (C == TEXT('"'))
			{
				if (i + 1 < Linea.Len() && Linea[i + 1] == TEXT('"')) { Actual.AppendChar(TEXT('"')); ++i; }
				else Comillas = false;
			}
			else Actual.AppendChar(C);
		}
		else if (C == TEXT('"')) Comillas = true;
		else if (C == TEXT(',')) { Out.Add(Actual); Actual.Reset(); }
		else Actual.AppendChar(C);
	}
	Out.Add(Actual);
	return Out;
}

const FLabCazador* FLabDatos::CazadorDeNivel(int32 Nivel) const
{
	for (const auto& Par : Cazadores) if (Par.Value.Nivel == Nivel) return &Par.Value;
	return nullptr;
}

TArray<const FLabHabilidad*> FLabDatos::HabilidadesDe(const FString& Duenio) const
{
	TArray<const FLabHabilidad*> Out;
	for (const auto& Par : Habilidades) if (Par.Value.Duenio == Duenio) Out.Add(&Par.Value);
	return Out;
}
