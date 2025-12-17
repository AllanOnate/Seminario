# CESFAM (Cadmium v2) — Modelo DEVS

Este repo contiene una implementación **Cadmium v2** del modelo DEVS basado en tu diagrama (CESFAM → Gestor de Casos/Buzón APS → Médicos → Adherencia/Decisión → retorno/egreso).

## 1) Requisitos

- Compilador C++17 (`g++` o `clang++`)
- Cadmium v2 (headers) disponible localmente (carpeta `include/` que contenga `cadmium/core/...`)

## 2) Build
### Para ejecutar codigo primcipal
```bash
make all
```
Salida:
- bin/CESFAM_V2
  
### Para ejecutar test
```bash
make test
```

Salida:
- bin/test_generator_v2
- bin/test_gestor_v2
- bin/test_medico_v2

## 3) Run

```bash
./bin/CESFAM_V2 input_data/params.ini
```

Se genera:
- `simulation_results/cesfam_log.csv` (por defecto)

## 4) Parámetros

Archivo `input_data/params.ini`.

- `simulation.until`: duración total en segundos.
- `arrivals.rate`: tasa de llegadas (pacientes/segundo) si no se usa CSV.
- `arrivals.csv`: si se define, usa `input_data/arrivals.csv` (llegadas determinísticas).
- `router.doctors`: número de médicos.
- `service.mean`: tiempo medio de atención (segundos).
- `consent.p_accept`: probabilidad de que el paciente sea aceptado por APS (si no, sale por RC).
- `adherence.p_continue_base`: probabilidad base de retorno (DA).

## 5) Estructura

- `data_structures/`: mensajes (`Patient`)
- `atomics/`: atomics DEVS (Generador, Gestor, Router, Médico, Adherencia)
- `coupled/`: acoplados (Equipo médico)
- `top_model/`: CESFAM + main
- `input_data/`: params + ejemplo arrivals.csv
- `simulation_results/`: logs

## 6) Notas

- Los tiempos se manejan como `double` (segundos).
- El modelo utiliza round-robin para asignación de pacientes a médicos (RouterMedicos).
- Para evitar bucles infinitos, `adherence.max_followups` limita el número de retornos.
