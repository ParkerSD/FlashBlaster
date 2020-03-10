


#include "nrf_drv_usbd.h"
#include "nrf_drv_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_timer.h"
#include "app_error.h"
#include "nrfx_usbd.h"

static void power_usb_event_handler(nrf_drv_power_usb_evt_t event)
{
    switch (event)
    {
    case NRF_DRV_POWER_USB_EVT_DETECTED:
       // NRF_LOG_INFO("USB power detected");
        if (!nrf_drv_usbd_is_enabled())
        {
            nrf_drv_usbd_enable();
        }
        break;
    case NRF_DRV_POWER_USB_EVT_REMOVED:
       // NRF_LOG_INFO("USB power removed");
        //m_usbd_configured = false;
        //m_send_mouse_position = false;
        if (nrf_drv_usbd_is_started())
        {
            nrf_drv_usbd_stop();
        }
        if (nrf_drv_usbd_is_enabled())
        {
            nrf_drv_usbd_disable();
        }
        /* Turn OFF LEDs */
        //bsp_board_led_off(LED_USB_STATUS);
        //bsp_board_led_off(LED_USB_POWER);
        break;
    case NRF_DRV_POWER_USB_EVT_READY:
       // NRF_LOG_INFO("USB ready");
        //bsp_board_led_on(LED_USB_POWER);
        if (!nrf_drv_usbd_is_started())
        {
            nrf_drv_usbd_start(true);
        }
        break;
    default:
        ASSERT(false);
    }
}


void usb_init(void)
{
  static const nrf_drv_power_usbevt_config_t config =
  {
    .handler = power_usb_event_handler
  };
  uint32_t ret = nrf_drv_power_usbevt_init(&config);
  APP_ERROR_CHECK(ret);
}
