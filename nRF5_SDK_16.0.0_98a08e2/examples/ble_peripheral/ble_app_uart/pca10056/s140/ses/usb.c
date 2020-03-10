


#include "nrf_drv_usbd.h"
#include "nrf_drv_power.h"
#include "nrf_drv_clock.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_timer.h"
#include "app_error.h"
#include "nrfx_usbd.h"

void init_power_clock(void)
{
    ret_code_t ret;
    /* Initializing power and clock */
    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);
    ret = nrf_drv_power_init(NULL);
    APP_ERROR_CHECK(ret);
    nrf_drv_clock_hfclk_request(NULL);
    nrf_drv_clock_lfclk_request(NULL);
    while (!(nrf_drv_clock_hfclk_is_running() &&
            nrf_drv_clock_lfclk_is_running()))
    {
        /* Just waiting */
    }

    /* Avoid warnings if assertion is disabled */
    UNUSED_VARIABLE(ret);
}


void power_usb_event_handler(nrf_drv_power_usb_evt_t event)
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


void usb_pwr_init(void)
{
  static const nrf_drv_power_usbevt_config_t config =
  {
    .handler = power_usb_event_handler
  };
  uint32_t ret = nrf_drv_power_usbevt_init(&config);
  APP_ERROR_CHECK(ret);
}


void usb_init(void)
{
//   /* USB work starts right here */
//   ret = nrf_drv_usbd_init(usbd_event_handler);
//   APP_ERROR_CHECK(ret);
}