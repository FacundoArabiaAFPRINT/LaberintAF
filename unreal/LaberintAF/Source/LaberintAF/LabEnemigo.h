// Bestias, cazadores y jefes (updateMob, updateHunter, hunterSkills y bossTick de la versión web) en una sola clase.
// Atención con sigilo (aw: «?» que se llena y «!» al descubrirte), búsqueda por la zona aproximada (los cazadores), patrulla,
// golpe con preparación y estocada, y las habilidades de HabilidadesCazadores.csv con su aviso en el piso.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LabTipos.h"
#include "LabEnemigo.generated.h"

class ULabSalud;
class UStaticMeshComponent;
class ALabPersonaje;
class ALabAviso;

UCLASS()
class LABERINTAF_API ALabEnemigo : public ACharacter
{
	GENERATED_BODY()

public:
	ALabEnemigo();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<ULabSalud> Salud;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Cuerpo;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Cabeza;

	UPROPERTY(BlueprintReadOnly) FString Tipo;        // perro, zombi, fantasma, demonio, cria, gallina, jefe, carnicero, bruja, roberto, duende…
	UPROPERTY(BlueprintReadOnly) FString Nombre;
	UPROPERTY(BlueprintReadOnly) bool bCazador = false;
	UPROPERTY(BlueprintReadOnly) bool bJefe = false;
	UPROPERTY(BlueprintReadOnly) bool bElite = false;
	UPROPERTY(BlueprintReadOnly) bool bAlfa = false;
	UPROPERTY(BlueprintReadOnly) bool bLlave = false;   // el cazador que lleva la llave de su presa
	UPROPERTY(BlueprintReadOnly) bool bDormido = false; // cazadores durante el farmeo
	UPROPERTY(BlueprintReadOnly) bool bFuria = false;   // jefes por debajo de la mitad de la vida
	UPROPERTY(BlueprintReadOnly) float Atencion = 0;    // 0 a 1
	UPROPERTY(BlueprintReadOnly) int32 Nivel = 0;

	float Danio = 5, Velocidad = 300, Alerta = 1000, Enfriamiento = 1, Oro = 5, Radio = 50;
	FVector Casa = FVector::ZeroVector;
	TWeakObjectPtr<ALabPersonaje> Presa;

	void ConfigurarBestia(const FString& Id, const FLabBestia& B, bool bEsElite);
	void ConfigurarCazador(const FString& Id, const FLabCazador& C, int32 SuNivel, bool bConLlave, ALabPersonaje* SuPresa);
	void Dormir();
	void Despertar(const FVector& Lugar);
	void AlRecibirGolpeDe(AActor* Quien);

	virtual void Tick(float Dt) override;

protected:
	virtual void BeginPlay() override;

private:
	TArray<const FLabHabilidad*> Habilidades;
	TArray<float> HabCd;
	TArray<FVector> Ruta; float RutaT = 0; FVector Meta = FVector::ZeroVector;
	FVector Patrulla = FVector::ZeroVector; float PatrullaT = 0;
	FVector Supuesto = FVector::ZeroVector; float SupuestoT = 0;
	FVector VistoEn = FVector::ZeroVector; float PerdidoT = 0; bool bDescubrio = false;
	float GolpeCd = 0, Ventana = 0, DanioVentana = 0, Bloqueo = 0, BuffT = 0, Estocada = 0;
	// embestida, salto y torbellino en curso
	FVector DashVel = FVector::ZeroVector; float DashT = 0; const FLabHabilidad* DashHab = nullptr; TSet<TWeakObjectPtr<ALabPersonaje>> DashGolpeados;
	FVector SaltoA, SaltoB; float SaltoT = -1, SaltoDur = 1; const FLabHabilidad* SaltoHab = nullptr;
	float GiroT = 0, GiroGolpe = 0; const FLabHabilidad* GiroHab = nullptr;
	float Intangible = 0;
	UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> MatCuerpo;

	void Aspecto(FLinearColor Color, float Escala);
	bool Ve(const ALabPersonaje* P) const;
	void Sigilo(ALabPersonaje* P, float Dt);
	void IrHacia(const FVector& Punto, float Velocidad, float Dt);
	bool Habilidad(int32 i, ALabPersonaje* T, float D, bool bVe);
	void Pegar(ALabPersonaje* T, float Dt, float D);
	void TickEspeciales(float Dt);
	void Aplicar(ALabPersonaje* P, const FLabHabilidad& H, float Danio);
	UFUNCTION() void AlMorir(AActor* Victima, AActor* Asesino);
};
