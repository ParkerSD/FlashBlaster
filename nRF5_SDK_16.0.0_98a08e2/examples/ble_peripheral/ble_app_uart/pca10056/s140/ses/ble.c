
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_twi.h"
#include "nrfx_twi.h"
#include "nrfx_twim.h"
#include "nrf_drv_spi.h"
#include "oled.h"
#include "app_button.h"
#include "nrf_gpio.h"
#include "button.h"
#include "ssd1351.h"
#include "usb.h"
#include "app_scheduler.h"
#include "nrf_power.h"
#include "system.h"
#include "flash.h"
#include "battery.h"
#include "ble.h"
#include "twi.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME                     "FLASHBLASTER"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                2000 // 0 = no timeout                            /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(7.5, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(7.5, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */



BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

//*********************GLOBALS*******************//

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};

uint8_t nus_data_global[BLE_PACKET_SIZE]; //max MTU matches chucksize of react.js app 
static uint8_t parser_trace = 0; 
static uint16_t packet_index = 0;
static uint8_t string_length; 
//static uint32_t data_length; //changed to local 
static int file_data_length; 
static int current_byte_pos;
static uint32_t file_data_addr_global;
static uint32_t chip_addr_global; 
static bool prog_flag = false; 
static bool add_all_mode = false; 
static bool ad_started = false;
static uint8_t chip_id[WORD_SIZE]; //chip_id decoded from chip name string 



/*
static struct chips_supported
{
    char NRF52840[8] = {"NRF52840"};
    char NRF52832[8] = {"NRF52832"};
    char ATSAMC21[8] = {"ATSAMC21"};
    char ATSAMC51[8] = {"ATSAMC51"};
}
*/

void ble_set_ad_stopped(void)
{
    ad_started = false;
}

/**@brief Function for starting advertising.
 */
void advertising_start(void)
{
    if(!ad_started)
    {
        oled_advertising_indicate(APP_ADV_DURATION);
        uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
        ad_started = true; 
    }
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}
/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


char* fetch_name(uint16_t length)
{
    char* name = malloc(sizeof(char[MAX_STRING_SIZE])); //max size of name string

    for(int i = 0; i < length; i++)
    {
        *(name + i) = nus_data_global[parser_trace + i];  
    }

    memset((name + length), NULL, (MAX_STRING_SIZE - length));  //set empty chars to null 
    string_length = length; 
    parser_trace += length;

    return name; 
}


uint32_t char_string_to_int(uint8_t digits)
{   
    char num_string[digits];
    uint8_t hex_bcd[digits]; 
    uint32_t accumulator = 0; 

    for(int i = 0; i < digits; i++)
    {
        num_string[i] = nus_data_global[parser_trace + i];
        hex_bcd[i] = num_string[i] - 0x30; 
        accumulator += (hex_bcd[i] * pow(10, digits-i-1)); 
    }
    return accumulator; 
}



char* ble_parse_name(void) //pass in all offsets
{
    //char string_len[2];
    char *name;

    uint32_t data_length = ble_parse_data_length(); 
    name = fetch_name(data_length); //offset is 6 

    return name;
} 



uint32_t ble_parse_data_length(void)
{
    char num_chars = nus_data_global[parser_trace]; //ascii for number of characters in length 
    parser_trace += 1;
    uint8_t i = num_chars - 0x30; //get int from ascii 
    uint32_t data_length = char_string_to_int(i);
    parser_trace += i; 

    return data_length;
}

uint32_t ble_parse_start_address(void)
{
    char hex_string[9];
    for(int i=0; i<8; i++)
    {
        hex_string[i] = nus_data_global[parser_trace+i];
    }
    hex_string[8] = '\0';
    uint32_t address = strtoul(hex_string, NULL, 16);

    return address; 
}

bool string_compare(char* name, char* target_name, uint8_t name_length)
{
    for(int i = 0; i < name_length; i++)
    {
        if(*(name + i) != *(target_name + i))
        {
            return false; 
        }
    }

    return true; 
}

void ble_decode_chip_id(char* chip_name, uint8_t chip_name_length) //NOTE: add statements here to increase supported chips and update atmel
{
   //1. compare strings
   //2. assign value to chip_id[3] global 

   if(string_compare(chip_name, "NRF52840", chip_name_length))
   {
      chip_id[3] = 0; //NOTE: increment value in LSB until 256 chips reached
   }
   else if(string_compare(chip_name, "NRF52832", chip_name_length))
   {
      chip_id[3] = 1; 
   }
   else if(string_compare(chip_name, "ATSAMD51", chip_name_length))
   {
      chip_id[3] = 2; 
   }
   else if(string_compare(chip_name, "ATSAMD21", chip_name_length))
   {
      chip_id[3] = 3; 
   }
   else if(string_compare(chip_name, "STM32F0", chip_name_length))
   {
      chip_id[3] = 4; 
   }
   else if(string_compare(chip_name, "STM32F1", chip_name_length))
   {
      chip_id[3] = 5; 
   }

   //else 
   // error - chip not supported
}


void add_project(void)
{   
    char *project_name;
    uint8_t project_name_length;
    project_name = ble_parse_name(); 
    project_name_length = string_length; //not used 
    
    flash_add_project(project_name);
}


void add_chip(void)
{
    char *chip_name;
    uint8_t chip_name_length; 
    char *project_name;
    uint8_t project_name_length;
    chip_name = ble_parse_name();
    chip_name_length = string_length;
    ble_decode_chip_id(chip_name, chip_name_length); //determine chip_id from name
    project_name = ble_parse_name();
    project_name_length = string_length;

    //Seek to parent based on name
    uint32_t project_addr = seek_to_project(project_name, project_name_length); //projects start at 4096, 52 bytes per project 
    if(project_addr == NULL)
    {   
        //ERROR: no project found
        nrf_gpio_pin_set(LED_RED);
    }
    
    flash_add_chip(project_addr, chip_name, chip_id, false);
}



uint32_t add_file(void)
{
    char* file_name;
    uint8_t file_name_length; 
    char* chip_name;
    uint8_t chip_name_length;
    char* project_name;
    uint8_t project_name_length;
    uint32_t start_address;
     
    file_name = ble_parse_name();
    file_name_length = string_length; 
    chip_name = ble_parse_name();
    chip_name_length = string_length;
    project_name = ble_parse_name();
    project_name_length = string_length;

    file_data_length = ble_parse_data_length(); 
    start_address = ble_parse_start_address();
    
    prog_flag = true; //flags nus handler to start appending incoming data 
    
    // flash seek functions
    uint32_t project_addr = seek_to_project(project_name, project_name_length); //projects start at 4096, 52 bytes per project 
    if(project_addr == NULL)
    {   
        //ERROR: no project found
        nrf_gpio_pin_set(LED_RED);
    }
    uint32_t chip_addr = seek_to_chip(project_addr, chip_name, chip_name_length); //chips start at addr 8192, 56 bytes per chip 
    if(chip_addr == NULL)
    {   
        //ERROR: no chip found
        nrf_gpio_pin_set(LED_RED);
    }

    file_header_write(chip_addr, file_name, start_address, file_data_length, false); //write file header and append file addr to chip 
    
    return chip_addr; 
} 

void add_all(void)
{
    char* file_name;
    uint8_t file_name_length; // not used 
    char* chip_name;
    uint8_t chip_name_length;
    char* project_name;
    uint8_t project_name_length; // not used 
    uint32_t start_address;
     
    file_name = ble_parse_name();
    file_name_length = string_length; 
    chip_name = ble_parse_name();
    chip_name_length = string_length;
    ble_decode_chip_id(chip_name, chip_name_length); //determine chip_id from chip_name
    project_name = ble_parse_name();
    project_name_length = string_length;

    file_data_length = ble_parse_data_length(); 
    start_address = ble_parse_start_address();

    prog_flag = true; //flags nus handler to start appending incoming data
    
    uint32_t project_addr = flash_add_project(project_name);
    uint32_t chip_addr = flash_add_chip(project_addr, chip_name, chip_id, true);
    file_header_write(chip_addr, file_name, start_address, file_data_length, true);
    
}


void packet_write_first(void)
{
    //read file bytes programmed
    //update file in flash with data_address?
    //append data as its received 
    //increment global file counter  
     
    uint32_t bytes_prog = fetch_bytes_prog(); 
    file_data_addr_global = bytes_prog + DATA_SECTOR_START; 
    flash_write(nus_data_global, file_data_addr_global, BLE_PACKET_SIZE); //NOTE WORD WRITES ONLY, write remaining file data from first ble packet after header
}

void packet_write(uint32_t data_start_addr)
{
    flash_write(nus_data_global, data_start_addr, BLE_PACKET_SIZE);
}


void ble_cmd_parser(void)
{
    if(nus_data_global[0] == 'C' && nus_data_global[1] == 'C') //start byte == "CC" 
    {
        char cmd[2]; 

        cmd[0] = nus_data_global[2];
        cmd[1] = nus_data_global[3];
        parser_trace = 4; 

        if(cmd[0] == '1' && cmd[1] =='0') //add project 
        {
            add_project(); 
        }
        else if(cmd[0] == '2' && cmd[1] =='0') //add chip 
        {
            // add chip to parent project in flash
            add_chip(); 
        }
        else if(cmd[0] == '3' && cmd[1] =='0') //add file 
        {
            // add file to parent chip and project in flash
            chip_addr_global = add_file(); 
        }
        else if(cmd[0] == '0' && cmd[0] =='0') //add all
        {
            add_all(); 
            add_all_mode = true; 
        }
        else if(cmd[0] == '4' && cmd[1] =='0') //delete project 
        {
            //delete_project();
        }
        else if(cmd[0] == '5' && cmd[1] =='0') //delete chip 
        {
            //delete_chip(); 
        }
        else if(cmd[0] == '6' && cmd[1] =='0') //delete file
        {
            //delete_file(); //NOTE: how to reuse empty data storage after deletion?
        }
        else if(cmd[0] == 'F' && cmd[1] =='F') //erase device
        {
            flash_init(); //reset flash
            hibernate(); // reset device
        }

        //else if production mode cmd

        //TODO free names after writing to flash

        parser_trace = 0; //reset for next command
    }
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */

void nus_data_handler(ble_nus_evt_t * p_evt)
{
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {        
        //oled_draw_transfer_progress();// not working, priority mismatch?
        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            nus_data_global[i] = p_evt->params.rx_data.p_data[i];
        }
        if(!prog_flag)
        {
            ble_cmd_parser(); 
            current_byte_pos = file_data_length;
        }
        else // prog flag true, start file write
        {
           if(current_byte_pos > 0)
            {   
                if(packet_index == 0)
                {
                    packet_write_first(); //calculate start address and write first packet
                    packet_index++;
                    current_byte_pos -= BLE_PACKET_SIZE; // subtract bytes programmed 
                }
                else
                {
                    uint32_t current_addr = (packet_index * BLE_PACKET_SIZE) + file_data_addr_global; //global assigned in packet_write_first(); 
                    packet_write(current_addr); 

                    //append entire nus_data_global array to file data in flash
                    //exception for last file (usually not full 244 bytes) 

                    packet_index++;
                    current_byte_pos -= BLE_PACKET_SIZE; 
                }
            }
            if(current_byte_pos <= 0)
            {
                // AFTER PROGRAMMING COMPLETE
                //oled_draw_transfer_complete(); 
                prog_flag = false;
                if(!add_all_mode)
                {
                    flash_file_num_inc(chip_addr_global);  //increment file_num in chip parent in flash
                }
                else
                {
                    add_all_mode = false; 
                }
                flash_file_dir_update(file_data_length); //increment file_count_global and file_bytes_programmed in flash
                
                device_shutdown();

                //NOTE FOR TEST 
//                uint8_t data_buff[FLASH_SECTOR_SIZE];
//                flash_read(data_buff, 0, FLASH_SECTOR_SIZE);
//
//                uint8_t data_buff1[FLASH_SECTOR_SIZE];
//                flash_read(data_buff1, CHIPS_START_ADDR, FLASH_SECTOR_SIZE);
//                prog_flag = true;
//
//                uint8_t data_buff2[FLASH_SECTOR_SIZE];
//                flash_read(data_buff2, FILES_START_ADDR, FLASH_SECTOR_SIZE);
//                prog_flag = true;
            }
        }
    }
}


/**@brief Function for initializing services that will be used by the application.
 */
void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            idle_state_handle();
            break;
        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
void ble_draw_icon(uint16_t color)
{
    //SSD1351_draw_line(x0, y0, x1, y1, uint16_t color)

//    small ble logo
//    SSD1351_draw_line( 5, 10, 9, 14, color); // draw x
//    SSD1351_draw_line( 5, 14, 9, 10, color);
//    SSD1351_draw_line( 7, 8, 7, 16, color); // draw vertical line
//    SSD1351_draw_line( 7, 8, 9, 10, color); // close two triangles
//    SSD1351_draw_line( 7, 16, 9, 14, color);

//  large ble logo 
    SSD1351_draw_line( 5, 10, 11, 16, color); 
    SSD1351_draw_line( 5, 16, 11, 10, color);
    SSD1351_draw_line( 8, 7, 8, 19, color); 
    SSD1351_draw_line( 8, 7, 11, 10, color); 
    SSD1351_draw_line( 8, 19, 11, 16, color);

    SSD1351_update();
}


void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:

            oled_stop_ad_timer(); 
            ble_draw_icon(COLOR_BLUE);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
 
            break;

        case BLE_GAP_EVT_DISCONNECTED:

            ble_draw_icon(COLOR_BLACK);
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            oled_draw_err(error_client_timeout); //PC(central) is client 
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            oled_draw_err(error_server_timeout); //flashblaster(peripheral) is server
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

//    init.config.ble_adv_primary_phy      = BLE_GAP_PHY_2MBPS;
//    init.config.ble_adv_secondary_phy    = BLE_GAP_PHY_2MBPS;
//    init.config.ble_adv_extended_enabled = true;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}