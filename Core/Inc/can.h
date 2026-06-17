/*
 * can.h
 *
 * Minimal CAN frame definitions for MCP2515 driver.
 * Matches the Linux SocketCAN can_frame layout used by the driver.
 *
 * Created: 9 Jun 2026
 * Edited: June 2026 (Added GCC structural packing attributes)
 */

#ifndef CAN_H_
#define CAN_H_

#include <stdint.h>

/* ── CAN ID flags ──────────────────────────────────────────────────────────
 * The driver encodes frame type in the upper bits of can_id, matching the
 * Linux SocketCAN convention.
 */

/** Extended Frame Format (29-bit ID) flag */
#define CAN_EFF_FLAG  0x80000000UL

/** Remote Transmission Request flag */
#define CAN_RTR_FLAG  0x40000000UL

/** Error frame flag (not used by this driver but included for completeness) */
#define CAN_ERR_FLAG  0x20000000UL

/** Mask for 11-bit Standard Frame ID */
#define CAN_SFF_MASK  0x000007FFUL

/** Mask for 29-bit Extended Frame ID */
#define CAN_EFF_MASK  0x1FFFFFFFUL

/** Maximum data bytes in a Classical CAN frame */
#define CAN_MAX_DLEN  8

/* ── can_frame ───────────────────────────────────────────────────────────── */
/**
 * @brief Structured mirror of a Linux SocketCAN frame.
 * @note __attribute__((packed)) prevents the GCC compiler from injecting
 * unintended memory alignment padding bytes between members, which
 * would corrupt direct SPI DMA transfers.
 */
typedef struct __attribute__((packed)) {
    uint32_t can_id;              /**< CAN ID + EFF/RTR/ERR flags        */
    uint8_t  can_dlc;             /**< Data Length Code: 0–8             */
    uint8_t  pad;                 /**< Padding (unused)                  */
    uint8_t  res0;                /**< Reserved (unused)                 */
    uint8_t  res1;                /**< Reserved (unused)                 */
    uint8_t  data[CAN_MAX_DLEN]; /**< Payload bytes                     */
} can_frame;

#endif /* CAN_H_ */
