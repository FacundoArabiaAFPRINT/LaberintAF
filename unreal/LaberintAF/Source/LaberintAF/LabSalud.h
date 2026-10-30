// Vida, estados y miembros (lo mismo que damage/hitP/bleedTick/trySever de la versión web), para el jugador y los enemigos.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LabTipos.h"
#include "LabSalud.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLabAlMorir, AActor*, Victima, AActor*, Asesino);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLabAlDanio, AActor*, Victima, float, Cantidad, AActor*, Fuente);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLabAlCortar, AActor*, Victima, ELabMiembro, Miembro);

UCLASS(ClassGroup = (Lab), meta = (BlueprintSpawnableComponent))
class LABERINTAF_API ULabSalud : public UActorComponent
{
	GENERATED_BODY()

public:
	ULabSalud();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud") float VidaMax = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Salud") float Vida = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud") float Armadura = 0;          // 0 a 0,5
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud") bool bEsJugador = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Salud") bool bPuedePerderMiembros = true;

	// estados en segundos que quedan (los mismos de st en la web)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Veneno = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Fuego = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Sangrado = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Aturdido = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Enredado = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Lento = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Ciego = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Invulnerable = 0;   // la voltereta
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Estados") float Intangible = 0;     // la niebla de Bob

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Miembros") TArray<bool> Miembros;  // brazo izq, brazo der, pierna izq, pierna der

	UPROPERTY(BlueprintAssignable) FLabAlMorir AlMorir;
	UPROPERTY(BlueprintAssignable) FLabAlDanio AlDanio;
	UPROPERTY(BlueprintAssignable) FLabAlCortar AlCortar;

	UFUNCTION(BlueprintCallable) bool Vivo() const { return Vida > 0; }
	UFUNCTION(BlueprintCallable) void Iniciar(float NuevaVidaMax);
	// devuelve true si el golpe entró (no estaba rodando ni muerto)
	UFUNCTION(BlueprintCallable) bool RecibirGolpe(float Cantidad, AActor* Fuente, ELabGolpe Tipo, const FLabEfectos& Efectos);
	UFUNCTION(BlueprintCallable) void Curar(float Cantidad, bool bCortaSangrado);
	UFUNCTION(BlueprintCallable) bool TieneMiembro(ELabMiembro M) const { return Miembros.IsValidIndex(int32(M)) && Miembros[int32(M)]; }

	// multiplicadores de la web: una pierna menos ×0,6 de velocidad (el jugador), un brazo menos ×0,62 de daño
	UFUNCTION(BlueprintCallable) float MultVelocidad() const;
	UFUNCTION(BlueprintCallable) float MultDanio() const;
	UFUNCTION(BlueprintCallable) bool PuedeMoverse() const { return Aturdido <= 0 && Enredado <= 0; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TWeakObjectPtr<AActor> UltimoGolpe;
	void IntentarCortar(float Cantidad, AActor* Fuente, ELabGolpe Tipo);
	void Morir(AActor* Asesino);
};
