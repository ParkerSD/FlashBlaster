/**
 * \file
 *
 * \brief S25FL116K component implement
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

#include <hal_gpio.h>
#include <hal_qspi_sync.h>
#include <spi_nor_flash.h>
#include <s25fl116k.h>

/** S25FL116K spi nor flash's interface */
static const struct spi_nor_flash_interface s25fl116k_interface = {
    s25fl116k_read,
    s25fl116k_write,
    s25fl116k_erase,
    s25fl116k_enable_xip,
    s25fl116k_disable_xip,
};

void s25fl116k_soft_reset(struct qspi_sync_descriptor *descr, uint8_t width)
{
	struct _qspi_command cmd = {
	    .inst_frame.bits.width    = width,
	    .inst_frame.bits.inst_en  = 1,
	    .inst_frame.bits.tfr_type = QSPI_WRITE_ACCESS,
	    .instruction              = S25FL1_SOFT_RESET_ENABLE,
	};
	qspi_sync_serial_run_command(descr, &cmd);

	cmd.instruction = S25FL1_SOFT_RESET;
	qspi_sync_serial_run_command(descr, &cmd);
}

static uint8_t s25fl116k_read_reg(struct qspi_sync_descriptor *descr, uint8_t width, uint8_t inst)
{
	uint32_t status;
	/** To prevent unaligned access. Set "buf_len" to 4 instead of 1 */
	struct _qspi_command cmd = {
	    .inst_frame.bits.width    = width,
	    .inst_frame.bits.inst_en  = 1,
	    .inst_frame.bits.data_en  = 1,
	    .inst_frame.bits.tfr_type = QSPI_READ_ACCESS,
	    .instruction              = inst,
	    .buf_len                  = 4,
	    .rx_buf                   = &status,
	};

	qspi_sync_serial_run_command(descr, &cmd);
	return (uint8_t)status;
}

static void s25fl116k_wait_memory_access_ready(struct qspi_sync_descriptor *descr)
{
	while (s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_1) & S25FL1_STATUS_RDYBSY)
		;
}

static void s25fl116k_write_enable(struct qspi_sync_descriptor *descr, uint8_t width, bool en)
{
	uint8_t              inst   = (en == true ? S25FL1_WRITE_ENABLE : S25FL1_WRITE_DISABLE);
	uint8_t              status = 0;
	struct _qspi_command cmd    = {
        .inst_frame.bits.width   = width,
        .inst_frame.bits.inst_en = 1,
        .instruction             = inst,
    };

	status = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_1);
	while ((status & S25FL1_STATUS_WEL) >> S25FL1_STATUS_WEL_Pos != en) {
		qspi_sync_serial_run_command(descr, &cmd);
		status = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_1);
	}
}

static void s25fl116k_write_status(struct qspi_sync_descriptor *descr, uint8_t width, uint8_t inst, uint8_t *data)
{
	/** To prevent unaligned access. Set "buf_len" to 4 instead of 3 */
	struct _qspi_command cmd = {
	    .inst_frame.bits.width    = width,
	    .inst_frame.bits.inst_en  = 1,
	    .inst_frame.bits.data_en  = 1,
	    .inst_frame.bits.tfr_type = QSPI_WRITE_ACCESS,
	    .instruction              = inst,
	    .buf_len                  = 4,
	    .tx_buf                   = data,
	};
	s25fl116k_write_enable(descr, width, true);
	qspi_sync_serial_run_command(descr, &cmd);
	s25fl116k_write_enable(descr, width, false);
}

static void s25fl116k_page_program(struct qspi_sync_descriptor *descr, const struct _qspi_command *cmd)
{
	s25fl116k_write_enable(descr, QSPI_INST1_ADDR1_DATA1, true);
	s25fl116k_wait_memory_access_ready(descr);
	qspi_sync_serial_run_command(descr, cmd);
	s25fl116k_wait_memory_access_ready(descr);
	s25fl116k_write_enable(descr, QSPI_INST1_ADDR1_DATA1, false);
}

/**
 * \brief Construct s25fl116k spi nor flash
 */
struct spi_nor_flash *s25fl116k_construct(struct spi_nor_flash *const me, void *const io, const uint8_t quad_mode)
{
	struct s25fl116k *s25fl1 = (struct s25fl116k *)me;
	spi_nor_flash_construct(me, io, &s25fl116k_interface);

	struct qspi_sync_descriptor *descr = (struct qspi_sync_descriptor *)(me->io);
	uint8_t                      status[3];

	status[0] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_1);
	status[1] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_2);
	status[2] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_3);

	s25fl1->quad_mode = quad_mode;
	if (quad_mode) {
		while (!(status[1] & S25FL1_STATUS_QUAD_ENABLE)) {
			status[1] |= S25FL1_STATUS_QUAD_ENABLE;
			s25fl116k_write_status(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_WRITE_STATUS, status);
			status[1] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_2);
		}
	} else {
		while (status[1] & S25FL1_STATUS_QUAD_ENABLE) {
			status[1] &= (~S25FL1_STATUS_QUAD_ENABLE);
			s25fl116k_write_status(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_WRITE_STATUS, status);
			status[1] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_2);
		}
	}
	return me;
}

int32_t s25fl116k_read(const struct spi_nor_flash *const me, uint8_t *buf, uint32_t address, uint32_t length)
{
	struct s25fl116k *           s25fl1 = (struct s25fl116k *)me;
	struct qspi_sync_descriptor *descr  = (struct qspi_sync_descriptor *)(me->io);

	struct _qspi_command cmd = {
	    .inst_frame.bits.inst_en      = 1,
	    .inst_frame.bits.data_en      = 1,
	    .inst_frame.bits.addr_en      = 1,
	    .inst_frame.bits.dummy_cycles = 8,
	    .inst_frame.bits.tfr_type     = QSPI_READMEM_ACCESS,
	    .instruction                  = S25FL1_READ_ARRAY,
	    .address                      = address,
	    .buf_len                      = length,
	    .rx_buf                       = buf,
	};

	if (s25fl1->quad_mode) {
		cmd.instruction           = S25FL1_READ_ARRAY_QUAD;
		cmd.inst_frame.bits.width = QSPI_INST1_ADDR1_DATA4;
	}
	qspi_sync_serial_run_command(descr, &cmd);
	return ERR_NONE;
}

int32_t s25fl116k_write(const struct spi_nor_flash *const me, uint8_t *buf, uint32_t address, uint32_t length)
{
	struct qspi_sync_descriptor *descr = (struct qspi_sync_descriptor *)(me->io);

	uint32_t i = 0;

	/** Size / page_size */
	uint32_t number_of_writes = (length >> 8);

	struct _qspi_command cmd = {
	    .inst_frame.bits.inst_en  = 1,
	    .inst_frame.bits.data_en  = 1,
	    .inst_frame.bits.addr_en  = 1,
	    .inst_frame.bits.tfr_type = QSPI_WRITEMEM_ACCESS,
	    .instruction              = S25FL1_BYTE_PAGE_PROGRAM,
	    .address                  = address,
	    .buf_len                  = length,
	    .tx_buf                   = buf,
	};

	/** If less than page size */
	if (number_of_writes == 0) {
		s25fl116k_page_program(descr, &cmd);
	} else {
		/** Multiple page */
		for (i = 0; i < number_of_writes; i++) {
			cmd.buf_len = S25FL1_PAGE_SIZE;
			cmd.address = address;
			cmd.tx_buf  = buf, s25fl116k_page_program(descr, &cmd);
			buf += S25FL1_PAGE_SIZE;
			address += S25FL1_PAGE_SIZE;
		}
		if (length % S25FL1_PAGE_SIZE) {
			cmd.buf_len = length % S25FL1_PAGE_SIZE;
			cmd.address = address;
			cmd.tx_buf  = buf, s25fl116k_page_program(descr, &cmd);
		}
	}
	return ERR_NONE;
}

int32_t s25fl116k_erase(const struct spi_nor_flash *const me, uint32_t address, uint32_t length)
{
	struct qspi_sync_descriptor *descr = (struct qspi_sync_descriptor *)(me->io);

	uint32_t temp_addr = address;
	uint32_t temp_len  = length;
	uint8_t  status;

	struct _qspi_command cmd = {
	    .inst_frame.bits.inst_en  = 1,
	    .inst_frame.bits.addr_en  = 1,
	    .inst_frame.bits.tfr_type = QSPI_WRITE_ACCESS,
	};

	if ((length % S25FL1_SECTOR_SIZE) || (address % S25FL1_SECTOR_SIZE)) {
		return ERR_INVALID_ARG;
	}
	status = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_1);
	if ((status & S25FL1_STATUS_RDYBSY) != S25FL1_STATUS_RDYBSY_READY)
		return S25FL1_ERROR_BUSY;

	s25fl116k_write_enable(descr, QSPI_INST1_ADDR1_DATA1, true);

	if (address == 0 && length >= S25FL1_FLASH_SIZE) {
		cmd.inst_frame.bits.addr_en = 0;
		cmd.instruction             = S25FL1_CHIP_ERASE_2;
		qspi_sync_serial_run_command(descr, &cmd);
		temp_len -= S25FL1_FLASH_SIZE;
		s25fl116k_wait_memory_access_ready(descr);
	}

	while (temp_len > 0) {
		if ((temp_addr % S25FL1_BLOCK_SIZE == 0) && (temp_len >= S25FL1_BLOCK_SIZE)) {
			cmd.address     = temp_addr;
			cmd.instruction = S25FL1_BLOCK_ERASE_64K;
			qspi_sync_serial_run_command(descr, &cmd);
			temp_addr += S25FL1_BLOCK_SIZE;
			temp_len -= S25FL1_BLOCK_SIZE;
			s25fl116k_wait_memory_access_ready(descr);
		} else if ((temp_addr % S25FL1_SECTOR_SIZE == 0) && (temp_len >= S25FL1_SECTOR_SIZE)) {
			cmd.address     = temp_addr;
			cmd.instruction = S25FL1_BLOCK_ERASE_4K;
			qspi_sync_serial_run_command(descr, &cmd);
			temp_addr += S25FL1_SECTOR_SIZE;
			temp_len -= S25FL1_SECTOR_SIZE;
			s25fl116k_wait_memory_access_ready(descr);
		} else {
			return ERR_INVALID_ARG;
		}
	}
	s25fl116k_write_enable(descr, QSPI_INST1_ADDR1_DATA1, false);
	return ERR_NONE;
}

int32_t s25fl116k_enable_xip(const struct spi_nor_flash *const me)
{
	struct qspi_sync_descriptor *descr = (struct qspi_sync_descriptor *)(me->io);

	uint8_t dummy_read, status[3];

	struct _qspi_command cmd = {
	    .inst_frame.bits.inst_en        = 1,
	    .inst_frame.bits.data_en        = 1,
	    .inst_frame.bits.addr_en        = 1,
	    .inst_frame.bits.opt_en         = 1,
	    .inst_frame.bits.tfr_type       = QSPI_READMEM_ACCESS,
	    .inst_frame.bits.width          = QSPI_INST1_ADDR4_DATA4,
	    .inst_frame.bits.dummy_cycles   = 5,
	    .inst_frame.bits.continues_read = 1,
	    .inst_frame.bits.opt_len        = QSPI_OPT_4BIT,
	    .instruction                    = S25FL1_READ_ARRAY_QUAD_IO,
	    .option                         = 0x2,
	    .address                        = 0,
	    .buf_len                        = 1,
	    .rx_buf                         = &dummy_read,
	};

	status[0] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_1);
	status[1] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_2);
	status[2] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_3);

	while (!(status[1] & S25FL1_STATUS_QUAD_ENABLE)) {
		status[1] |= S25FL1_STATUS_QUAD_ENABLE;
		s25fl116k_write_status(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_WRITE_STATUS, status);
		status[1] = s25fl116k_read_reg(descr, QSPI_INST1_ADDR1_DATA1, S25FL1_READ_STATUS_2);
	}

	qspi_sync_serial_run_command(descr, &cmd);
	return ERR_NONE;
}

int32_t s25fl116k_disable_xip(const struct spi_nor_flash *const me)
{
	struct qspi_sync_descriptor *descr = (struct qspi_sync_descriptor *)(me->io);

	struct _qspi_command cmd = {
	    .inst_frame.bits.width   = QSPI_INST1_ADDR1_DATA1,
	    .inst_frame.bits.inst_en = 1,
	    .instruction             = S25FL1_CONT_MODE_RESET,
	};

	qspi_sync_init(descr, QSPI);
	qspi_sync_enable(descr);

	qspi_sync_serial_run_command(descr, &cmd);
	return ERR_NONE;
}
