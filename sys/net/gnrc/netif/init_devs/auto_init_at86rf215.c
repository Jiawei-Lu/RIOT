/*
 * Copyright (C) 2019 ML!PA Consulting GmbH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/*
 * @ingroup sys_auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief   Auto initialization for at86rf215 network interfaces
 *
 * @author  Benjamin Valentin <benjamin.valentin@ml-pa.com>
 */

#ifdef MODULE_AT86RF215

#define USED_BANDS (IS_USED(MODULE_AT86RF215_SUBGHZ) + IS_USED(MODULE_AT86RF215_24GHZ))

#include "log.h"
#include "board.h"
#include "net/gnrc/netif/ieee802154.h"
#ifdef MODULE_GNRC_LWMAC
#include "net/gnrc/lwmac/lwmac.h"
#endif
#ifdef MODULE_GNRC_GOMACH
#include "net/gnrc/gomach/gomach.h"
#endif
#include "net/gnrc.h"

#include "at86rf215.h"
#include "at86rf215_params.h"

/* If we don't have enough NETIFs configured, disable the sub-GHz band */
#if (GNRC_NETIF_NUMOF == 1) && IS_USED(MODULE_AT86RF215_SUBGHZ) && IS_USED(MODULE_AT86RF215_24GHZ)
#undef MODULE_AT86RF215_SUBGHZ
#undef USED_BANDS
#define USED_BANDS 1
#endif

/**
 * @brief   Define stack parameters for the MAC layer thread
 * @{
 */
#define AT86RF215_MAC_STACKSIZE     (THREAD_STACKSIZE_DEFAULT)
#ifndef AT86RF215_MAC_PRIO
#define AT86RF215_MAC_PRIO          (GNRC_NETIF_PRIO)
#endif
#ifndef AT86RF215_MAC_PRIO_SUBGHZ
#define AT86RF215_MAC_PRIO_SUBGHZ   (AT86RF215_MAC_PRIO)
#endif

#define AT86RF215_NUM ARRAY_SIZE(at86rf215_params)

static at86rf215_t at86rf215_devs[AT86RF215_NUM * USED_BANDS];
static char _at86rf215_stacks[AT86RF215_NUM * USED_BANDS][AT86RF215_MAC_STACKSIZE];

static inline void _setup_netif(void* netdev, void* stack, int prio) {
    if (netdev == NULL) {
        return;
    }

#if defined(MODULE_GNRC_GOMACH)
        gnrc_netif_gomach_create(stack,
                                 AT86RF215_MAC_STACKSIZE,
                                 prio, "at86rf215-gomach",
                                 netdev);
#elif defined(MODULE_GNRC_LWMAC)
        gnrc_netif_lwmac_create(stack,
                                 AT86RF215_MAC_STACKSIZE,
                                 prio, "at86rf215-lwmac",
                                 netdev);
#else
        gnrc_netif_ieee802154_create(stack,
                                 AT86RF215_MAC_STACKSIZE,
                                 prio, "at86rf215",
                                 netdev);
#endif
}

void auto_init_at86rf215(void)
{
    unsigned i = 0;
    unsigned j = 0;
    while (j < AT86RF215_NUM) {

        at86rf215_t *dev_09 = NULL;
        at86rf215_t *dev_24 = NULL;
        void *stack_09 = NULL;
        void *stack_24 = NULL;

        if (IS_USED(MODULE_AT86RF215_SUBGHZ)) {
            dev_09   = &at86rf215_devs[i];
            stack_09 = &_at86rf215_stacks[i];
            ++i;
        }

        if (IS_USED(MODULE_AT86RF215_24GHZ)) {
            dev_24   = &at86rf215_devs[i];
            stack_24 = &_at86rf215_stacks[i];
            ++i;
        }

        at86rf215_setup(dev_09, dev_24, &at86rf215_params[j++]);

        /* setup sub-GHz interface */
        _setup_netif(dev_09, stack_09, AT86RF215_MAC_PRIO_SUBGHZ);

        /* setup 2.4-GHz interface */
        _setup_netif(dev_24, stack_24, AT86RF215_MAC_PRIO);
    }
}

#else
typedef int dont_be_pedantic;
#endif /* MODULE_AT86RF215 */

/** @} */
