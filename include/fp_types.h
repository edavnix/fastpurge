/**
 * @file fp_types.h
 * @brief Tipos comunes, enumeraciones y estructuras para FastPurge.
 *
 * Define el contrato entre los módulos de escaneo, filtrado, purga paralela
 * y reporte. No debe haber definiciones de tipos fuera de este archivo.
 */

#ifndef FP_TYPES_H
#define FP_TYPES_H

#include <stddef.h>
#include <time.h>

/* ── Opciones de línea de comandos ───────────────────────────────────────── */

/**
 * @struct FpConfig
 * @brief Configuración global parseada desde la CLI.
 *
 * Se llena en main.c y se pasa a los demás módulos.
 */
typedef struct {
  const char *target_path;      /**< Directorio raíz a procesar.            */
  int workers;                  /**< Número de procesos hijos (default 1).  */
  int min_age_days;             /**< Borrar solo archivos con más de N días (0 = sin filtro). */
  int secure_wipe;              /**< 1 = sobrescribir con ceros antes de borrar, 0 = normal. */
  int dry_run;                  /**< 1 = solo simular, no borrar realmente. */
  const char *exclude_pattern;  /**< Subdirectorio a excluir (ej: ".git"), o NULL. */
} FpConfig;

/* ── Información de un archivo ───────────────────────────────────────────── */

/**
 * @struct FpFileEntry
 * @brief Metadatos de un archivo encontrado durante el escaneo.
 *
 * Se usa para construir la lista de archivos a procesar.
 */
typedef struct {
  char path[4096];    /**< Ruta absoluta del archivo.       */
  off_t size;         /**< Tamaño en bytes.                 */
  time_t mtime;       /**< Última modificación (timestamp). */
} FpFileEntry;

/* ── Resultado del escaneo ───────────────────────────────────────────────── */

/**
 * @struct FpScanResult
 * @brief Resumen de todos los archivos encontrados y filtrados.
 *
 * Lo produce scanner.c + filter.c, y se pasa a purge.c.
 */
typedef struct {
  FpFileEntry **files;  /**< Array de punteros a archivos candidatos. */
  int file_count;       /**< Número de archivos en el array.          */
  size_t total_bytes;   /**< Suma de tamaños de todos los archivos.   */
} FpScanResult;

/* ── Progreso y estadísticas de purga ────────────────────────────────────── */

/**
 * @struct FpProgress
 * @brief Datos enviados desde cada worker al proceso padre.
 *
 * Se transmite por pipe. Cada worker envía uno al terminar su lote.
 */
typedef struct {
  int worker_id;        /**< Identificador del worker (0..workers-1). */
  int deleted;          /**< Número de archivos borrados por este worker. */
  int failed;           /**< Número de fallos al borrar.               */
  size_t freed_bytes;   /**< Bytes liberados por este worker.         */
} FpProgress;

/**
 * @struct FpPurgeResult
 * @brief Resultado final acumulado de todos los workers.
 *
 * Lo construye el proceso padre tras recibir todos los FpProgress.
 */
typedef struct {
  int total_deleted;    /**< Suma de archivos borrados.        */
  int total_failed;     /**< Suma de fallos.                   */
  size_t total_freed;   /**< Suma de bytes liberados.          */
  double elapsed_sec;   /**< Tiempo total de la operación.     */
} FpPurgeResult;

/* ── Modos de operación internos (no expuestos al CLI) ───────────────────── */

/**
 * @enum FpWipeMode
 * @brief Modo de sobrescritura (usado internamente por wipe.c).
 */
typedef enum {
  FP_WIPE_NONE = 0,   /**< No sobrescribir, solo unlink. */
  FP_WIPE_ZEROS,      /**< Sobrescribir con ceros una vez. */
  /* En el futuro podría añadirse FP_WIPE_RANDOM, etc. */
} FpWipeMode;

#endif /* FP_TYPES_H */
