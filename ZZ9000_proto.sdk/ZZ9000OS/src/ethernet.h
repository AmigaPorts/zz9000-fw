/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2019, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
*/

#ifndef ETHERNET_H_
#define ETHERNET_H_

int ethernet_init();
u16 ethernet_send_frame(u16 frame_size);
void ethernet_receive_frame();
u32 get_frames_received();
uint8_t* ethernet_get_mac_address_ptr();
void ethernet_update_mac_address();
uint8_t* ethernet_current_receive_ptr();
int ethernet_get_backlog();

// FIXME allocate this memory properly

#define TX_BD_LIST_START_ADDRESS	0x03100000
#define RX_BD_LIST_START_ADDRESS	0x03108000
#define TX_FRAME_ADDRESS            0x03110000
#define RX_FRAME_ADDRESS            0x03120000
#define RX_BACKLOG_ADDRESS          0x03200000 // 32 * 2048 space (64 kB)
#define USB_BLOCK_STORAGE_ADDRESS   0x04000000 // FIXME move all of these to a memory table header file
#define RX_FRAME_PAD 4
#define FRAME_SIZE 2048

#define FRAME_MAX_BACKLOG 32

#define RXBD_CNT       1	/* Number of RxBDs to use */
#define TXBD_CNT       2	/* Number of TxBDs to use */

#endif
