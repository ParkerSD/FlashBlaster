/**
 * \file
 *
 * \brief S25FL116K component declaration
 *
 * Copyright (c) 2016-2019 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef _S25FL116K_H_INCLUDED
#define _S25FL116K_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <spi_nor_flash.h>

/** Device is protected, operation cannot be carried out. */
#define S25FL1_ERROR_PROTECTED 1
/** Device is busy executing a command. */
#define S25FL1_ERROR_BUSY 2
/** There was a problem while trying to program page data. */
#define S25FL1_ERROR_PROGRAM 3
/** There was an SPI communication error. */
#define S25FL1_ERROR_SPI 4

/** Register bits of s25fl116k */
/** Device ready/busy status bit. */
#define S25FL1_STATUS_RDYBSY (1 << 0)
/** Device is ready. */
#define S25FL1_STATUS_RDYBSY_READY (0 << 0)
/** Device is busy with internal operations. */
#define S25FL1_STATUS_RDYBSY_BUSY (1 << 0)
/** Write enable latch status bit. */
#define S25FL1_STATUS_WEL_Pos (1)
/** Write enable latch status bit. */
#define S25FL1_STATUS_WEL (1 << 1)
/** Device is not write enabled. */
#define S25FL1_STATUS_WEL_DISABLED (0 << 1)
/** Device is write enabled. */
#define S25FL1_STATUS_WEL_ENABLED (1 << 1)
/** Software protection status bit-field. */
#define S25FL1_STATUS_SWP (3 << 2)
/** All sectors are software protected. */
#define S25FL1_STATUS_SWP_PROTALL (3 << 2)
/** Some sectors are software protected. */
#define S25FL1_STATUS_SWP_PROTSOME (1 << 2)
/** No sector is software protected. */
#define S25FL1_STATUS_SWP_PROTNONE (0 << 2)
/** Write protect pin status bit. */
#define S25FL1_STATUS_WPP (1 << 4)
/** Write protect signal is not asserted. */
#define S25FL1_STATUS_WPP_NOTASSERTED (0 << 4)
/** Write protect signal is asserted. */
#define S25FL1_STATUS_WPP_ASSERTED (1 << 4)
/** Erase/program error bit. */
#define S25FL1_STATUS_EPE (1 << 5)
/** Erase or program operation was successful. */
#define S25FL1_STATUS_EPE_SUCCESS (0 << 5)
/** Erase or program error detected. */
#define S25FL1_STATUS_EPE_ERROR (1 << 5)
/** Sector protection registers locked bit. */
#define S25FL1_STATUS_SPRL (1 << 7)
/** Sector protection registers are unlocked. */
#define S25FL1_STATUS_SPRL_UNLOCKED (0 << 7)
/** Sector protection registers are locked. */
#define S25FL1_STATUS_SPRL_LOCKED (1 << 7)
/** Quad enable bit */
#define S25FL1_STATUS_QUAD_ENABLE (1 << 1)
/** Quad enable bit */
#define S25FL1_STATUS_WRAP_ENABLE (0 << 4)
/** Latency control bits */
#define S25FL1_STATUS_LATENCY_CTRL (0xF << 0)
#define S25FL1_STATUS_WRAP_BYTE (1 << 5)
#define S25FL1_BLOCK_PROTECT_Msk (7 << 2)
#define S25FL1_TOP_BTM_PROTECT_Msk (1 << 5)
#define S25FL1_SEC_PROTECT_Msk (1 << 6)
#define S25FL1_CHIP_PROTECT_Msk (0x1F << 2)

/** Register addresses of s25fl116k */
/** Sequential program mode command code 1. */
#define S25FL1_SEQUENTIAL_PROGRAM_1 0xAD
/** Sequential program mode command code 2. */
#define S25FL1_SEQUENTIAL_PROGRAM_2 0xAF
/** Protect sector command code. */
#define S25FL1_PROTECT_SECTOR 0x36
/** Unprotected sector command code. */
#define S25FL1_UNPROTECT_SECTOR 0x39
/** Read sector protection registers command code. */
#define S25FL1_READ_SECTOR_PROT 0x3C
/** Resume from deep power-down command code. */
#define S25FL1_SOFT_RESET_ENABLE 0x66
/** Resume from deep power-down command code. */
#define S25FL1_SOFT_RESET 0x99
/** Read status register command code. */
#define S25FL1_READ_STATUS_1 0x05
/** Read status register command code. */
#define S25FL1_READ_STATUS_2 0x35
/** Read status register command code. */
#define S25FL1_READ_STATUS_3 0x33
/** Write enable command code. */
#define S25FL1_WRITE_ENABLE 0x06
/** Write Enable for Volatile Status Register. */
#define S25FL1_WRITE_ENABLE_FOR_VOLATILE_STATUS 0x50
/** Write disable command code. */
#define S25FL1_WRITE_DISABLE 0x04
/** Write status register command code. */
#define S25FL1_WRITE_STATUS 0x01
/** Resume from deep power-down command code. */
#define S25FL1_WRAP_ENABLE 0x77
/** Byte/page program command code. */
#define S25FL1_BYTE_PAGE_PROGRAM 0x02
/** Block erase command code (4K block). */
#define S25FL1_BLOCK_ERASE_4K 0x20
/** Block erase command code (32K block). */
#define S25FL1_BLOCK_ERASE_32K 0x52
/** Block erase command code (64K block). */
#define S25FL1_BLOCK_ERASE_64K 0xD8
/** Chip erase command code 1. */
#define S25FL1_CHIP_ERASE_1 0x60
/** Chip erase command code 2. */
#define S25FL1_CHIP_ERASE_2 0xC7
/** Read array (low frequency) command code. */
#define S25FL1_READ_ARRAY_LF 0x03
/** Read array command code. */
#define S25FL1_READ_ARRAY 0x0B
/** Fast Read array  command code. */
#define S25FL1_READ_ARRAY_DUAL 0x3B
/** Fast Read array  command code. */
#define S25FL1_READ_ARRAY_QUAD 0x6B
/** Fast Read array  command code. */
#define S25FL1_READ_ARRAY_DUAL_IO 0xBB
/** Fast Read array  command code. */
#define S25FL1_READ_ARRAY_QUAD_IO 0xEB
/** Deep power-down command code. */
#define S25FL1_DEEP_PDOWN 0xB9
/** Resume from deep power-down command code. */
#define S25FL1_RES_DEEP_PDOWN 0xAB
/** Manufacturer/ Device ID command code. */
#define S25FL1_MANUFACTURER_DEVICE_ID 0x90
/** Read manufacturer and device ID command code. */
#define S25FL1_READ_JEDEC_ID 0x9F
/** Continuous Read Mode Reset command code. */
#define S25FL1_CONT_MODE_RESET 0xFF

/** Size of s25fl116k */
#define S25FL1_PAGE_SIZE 256
#define S25FL1_SECTOR_SIZE 4096
#define S25FL1_BLOCK_SIZE 65536
#define S25FL1_FLASH_SIZE 0x200000

/**
 * \brief S25FL116K spi nor flash
 */
struct s25fl116k {
	/** The parent abstract spi nor flash */
	struct spi_nor_flash parent;
	/** Quad mode used by SPI read/write data */
	uint8_t quad_mode;
};

/**
 * \brief Construct s25fl116k spi nor flash
 *
 * \param[in] me        Pointer to the abstract spi nor flash structure
 * \param[in] io        Pointer to the instance of controller
 * \param[in] quad_mode 0 single-bit SPI data, 1 quad SPI data
 *
 * \return pointer to initialized flash
 */
struct spi_nor_flash *s25fl116k_construct(struct spi_nor_flash *const me, void *const io, const uint8_t quad_mode);

/**
 * \brief Read bytes from the spi nor Flash
 *
 * \param[in]  me       Pointer to the abstract spi nor flash structure
 * \param[out] buf      Pointer to the buffer
 * \param[in]  address  Source bytes address to read from flash
 * \param[in]  length   Number of bytes to read
 * \return status.
 */
int32_t s25fl116k_read(const struct spi_nor_flash *const me, uint8_t *buf, uint32_t address, uint32_t length);

/**
 * \brief Write bytes to the spi nor Flash
 *
 * \param[in] me        Pointer to the abstract spi nor flash structure
 * \param[in] buf       Pointer to the buffer
 * \param[in] address   Destination bytes address to write into flash
 * \param[in] length    Number of bytes to write
 * \return status.
 */
int32_t s25fl116k_write(const struct spi_nor_flash *const me, uint8_t *buf, uint32_t address, uint32_t length);

/**
 * \brief Erase sectors/blocks in the spi nor Flash
 * \param[in] me        Pointer to the abstract spi nor flash structure
 * \param[in] address   Destination bytes address aligned with sector/block start
 * \param[in] length    Number of bytes to be erase
 * \return status.
 *
 * \note length must be multiple of sector size
 */
int32_t s25fl116k_erase(const struct spi_nor_flash *const me, uint32_t address, uint32_t length);

/**
 * \brief Enable the XIP mode (continous read)
 *
 * \param[in] me        Pointer to the abstract spi nor flash structure
 * \return status.
 */
int32_t s25fl116k_enable_xip(const struct spi_nor_flash *const me);

/**
 * \brief Disable the XIP mode (continous read)
 *
 * \param[in] me        Pointer to the abstract spi nor flash structure
 * \return status.
 */
int32_t s25fl116k_disable_xip(const struct spi_nor_flash *const me);

#ifdef __cplusplus
}
#endif

#endif /* _S25FL116K_H_INCLUDED */
