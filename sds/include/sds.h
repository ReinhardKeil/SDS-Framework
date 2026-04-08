/*
 * Copyright (c) 2025-2026 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SDS
#define SDS

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// ==== SDS Recorder and Player Interface ====

typedef void *sdsId_t;                 // Handle to SDS Recorder/Player stream

// Function return codes
#define SDS_OK                 (0)     // Operation completed successfully
#define SDS_ERROR              (-1)    // Operation failed
#define SDS_ERROR_PARAMETER    (-2)    // Operation failed: parameter error
#define SDS_ERROR_TIMEOUT      (-3)    // Operation failed: timeout error
#define SDS_ERROR_IO           (-4)    // Operation failed: SDS I/O interface error
#define SDS_ERROR_NO_SPACE     (-5)    // Operation failed: insufficient space in stream buffer
#define SDS_ERROR_NO_DATA      (-6)    // Operation failed: insufficient data in stream buffer
#define SDS_EOS                (-7)    // End of stream reached

// Event codes for sdsEvent callback function
#define SDS_EVENT_ERROR_IO     (1UL)   // SDS I/O interface error
#define SDS_EVENT_NO_SPACE     (2UL)   // sdsWrite() failed: insufficient space in stream buffer
#define SDS_EVENT_NO_DATA      (4UL)   // sdsRead() failed: insufficient data in stream buffer

// Open Mode
typedef enum {
  sdsModeRead  = 0,             // Open for read
  sdsModeWrite = 1              // Open for write
} sdsMode_t;                    // Open mode (read/write)

/**
  \typedef void (*sdsEvent_t) (sdsId_t id, uint32_t event)
  \brief       Callback function for recorder and player events
  \param[in]   id             \ref sdsId_t handle to SDS Recorder/Player stream
  \param[in]   event          event code (see \ref SDS_Event_Codes)
*/
typedef void (*sdsEvent_t) (sdsId_t id, uint32_t event);

/**
  \fn          int32_t sdsInit (sdsEvent_t event_cb)
  \brief       Initialize recorder and player.
  \param[in]   event_cb       pointer to \ref sdsEvent_t callback function
  \return      SDS_REC_PLAY_OK on success or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsInit (sdsEvent_t event_cb);

/**
  \fn          int32_t sdsUninit (void)
  \brief       Uninitialize recorder and player.
  \return      SDS_REC_PLAY_OK on success or
               a negative value on error (see \ref SDS_Recorder_Player_Return_Codes)
*/
int32_t sdsUninit (void);

/**
  \fn          sdsId_t sdsOpen (const char *name, void *buf, uint32_t buf_size, sdsMode_t mode)
  \brief       Open recorder or playback stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   buf            pointer to buffer for recorder stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   mode           stream mode: 0=read (playback), 1=write (recording)
  \return      \ref sdsId_t handle to SDS Recorder/Player stream, or NULL if operation failed
*/
sdsId_t sdsOpen (const char *name, void *buf, uint32_t buf_size, sdsMode_t mode);

/**
  \fn          int32_t sdsClose (sdsId_t id)
  \brief       Close recorder stream.
  \param[in]   id             \ref sdsId_t handle to SDS Recorder/Player stream
  \return      SDS_OK on success or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsClose (sdsId_t id);

/**
  \fn          int32_t sdsWrite (sdsId_t id, uint32_t timeslot, const void *buf, uint32_t buf_size)
  \brief       Close recorder stream or playback stream.
  \param[in]   id             \ref sdsId_t handle to SDS Recorder/Player stream
  \param[in]   timeslot       timeslot in ticks
  \param[in]   buf            pointer to the data block buffer to be written
  \param[in]   buf_size       size of the data block buffer in bytes
  \return      number of bytes successfully written or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsWrite (sdsId_t id, uint32_t timeslot, const void *buf, uint32_t buf_size);

/**
  \fn          int32_t sdsRead (sdsId_t id, uint32_t *timeslot, void *buf, uint32_t buf_size)
  \brief       Read entire data block along with its timeslot from the player stream.
  \param[in]   id             \ref sdsId_t handle to SDS Recorder/Player stream
  \param[out]  timeslot       pointer to buffer for a timeslot value
  \param[out]  buf            pointer to the data block buffer to be read
  \param[in]   buf_size       size of the data block buffer in bytes
  \return      number of bytes successfully read, or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsRead (sdsId_t id, uint32_t *timeslot, void *buf, uint32_t buf_size);

/**
  \fn          int32_t sdsGetSize (sdsId_t id)
  \brief       Get data block size for a read (Player) stream.
  \param[in]   id             \ref sdsId_t handle to SDS Recorder/Player stream
  \return      number of bytes in next available data block, or
               a negative value on error or SDS_EOS (see \ref SDS_Return_Codes)
*/
int32_t sdsGetSize (sdsId_t id);


// ==== SDS Control Interface ====

// Error information output information
typedef struct {                    
  int32_t status;                       // Error status code (see \ref SDS_Return_Codes)
  const char *file;                     // File name where error occurred
  uint32_t line;                        // Line number where error occurred
  uint8_t occurred;                     // Flag indicating that an error has occurred
} sdsError_t;

// Error information
extern sdsError_t sdsError;

#ifndef SDS_ERRORCHECK

// Check for error and record error location
#define SDS_ERRORCHECK(sds_status)                   \
  if (!((sds_status) != 0) && (!sdsError.occurred)) {\
    sdsError.status = sds_status;                    \
    sdsError.file = __FILE__;                        \
    sdsError.line = __LINE__;                        \
    sdsError.occurred = 1U;                          \
  }

#endif SDS_ERRORCHECK

#ifndef SDS_PRINTF

// Print Messages
#define SDS_PRINTF(...)                               \
  printf(__VA_ARGS__)
#endif

#endif SDS_PRINTF

// Configuration options and diagnostic storage
extern volatile uint32_t sdsFlags;    // SDS control flags (see \ref SDS_Flags)
#define SDS_FLAG_START     (1U << 31) // sdsFlags.31 = 1 for start of the SDS recording/playback I/O
#define SDS_FLAG_TERMINATE (1U << 30) // sdsFlags.30 = 1 for terminating CI run (on FVP simulation or pyOCD)
#define SDS_FLAG_PLAYBACK  (1U << 29) // sdsFlags.29 = 1 for playback mode
                                      // sdsFlags.24...28 reserved for future enhancements
                                      // sdsFlags.0..27 for user options (i.e. bypassing filter, ect.)


/**
  \fn          void sdsFlagsModify (uint32_t set_mask, uint32_t clear_mask)
  \brief       Modify SDS control flags (atomic operation).
  \param[in]   set_mask        bits to set in sdsFlags
  \param[in]   clear_mask      bits to clear in sdsFlags
*/
void sdsFlagsModify (uint32_t set_mask, uint32_t clear_mask);


extern volatile uint32_t sdsState;        // SDS states (see \ref SDSIO_States)
#define SDS_STATE_INACTIVE          0     // Streaming is not active and not connected to SDSIO module
#define SDS_STATE_CONNECTED         1     // Streaming is connected to SDSIO module
#define SDS_STATE_START             2     // Request to start streaming, open streams and get ready for read/write operations
#define SDS_STATE_ACTIVE            3     // Streaming is active
#define SDS_STATE_STOP_REQ          4     // Request to stop streaming and close streams
#define SDS_STATE_STOP_DONE         5     // Streaming is stopped
#define SDS_STATE_END               6     // Request to end streaming (no more data)

/**
  \fn          int32_t sdsQuery (uint32_t idle_factor)
  \param[in]   idle_factor     idle factor of control loop (optional value, -1 if not available)
  \return      Exchange information with client (sdsFlags, error message, idle factor)
*/
int32_t sdsQuery (uint32_t idle_factor);


//**** This may go to an own header file, but we need then a sdsQuery function that calls effectively sdsioQuery  */
// ==== SDS I/O Interface ====

typedef void *sdsioId_t;              // Handle to SDS I/O stream

/**
  \fn          int32_t sdsioInit (void)
  \brief       Initialize SDS I/O interface.
  \return      SDS_OK on success or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsioInit (void);

/**
  \fn          int32_t sdsioUninit (void)
  \brief       Un-initialize SDS I/O interface.
  \return      SDS_OK on success or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsioUninit (void);

/**
  \fn          sdsioId_t sdsioOpen (const char *name, sdsMode_t mode)
  \brief       Open I/O stream (note: this function called by sds.c module).
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   mode           \ref sdsioMode_t open mode
  \return      \ref sdsioId_t Handle to SDS I/O stream, or NULL if operation failed
*/
sdsioId_t sdsioOpen (const char *name, sdsMode_t mode);

/**
  \fn          int32_t sdsioClose (sdsioId_t id)
  \brief       Close I/O stream (note: this function called by sds.c module).
  \param[in]   id             \ref sdsioId_t handle to SDS I/O stream
  \return      SDSIO_OK on success or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsioClose (sdsioId_t id);

/**
  \fn          int32_t sdsioWrite (sdsioId_t id, const void *buf, uint32_t buf_size)
  \brief       Write data to I/O stream (note: this function called by sds.c module).
  \param[in]   id             \ref sdsioId_t handle to SDS I/O stream
  \param[in]   buf            pointer to buffer with data to write
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes successfully written or
               a negative value on error (see \ref SDS_Return_Codes)
*/
int32_t sdsioWrite (sdsioId_t id, const void *buf, uint32_t buf_size);

/**
  \fn          int32_t sdsioRead (sdsioId_t id, void *buf, uint32_t buf_size)
  \brief       Read data from I/O stream (note: this function called by sds.c module).
  \param[in]   id             \ref sdsioId_t handle to SDS I/O stream
  \param[out]  buf            pointer to buffer for data to read
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes successfully read, or
               a negative value on error or SDS_EOS (see \ref SDS_Return_Codes)
*/
int32_t sdsioRead (sdsioId_t id, void *buf, uint32_t buf_size);

/**
  \fn          int32_t sdsioQuery (uint32_t idle_factor)
  \param[in]   idle_factor     idle factor of control loop (optional value)
  \return      Exchange information with client (sdsFlags, error message, idle factor)
*/
int32_t sdsioQuery (uint32_t idle_factor);

#ifdef  __cplusplus
}
#endif

#endif  /* SDS_H */
