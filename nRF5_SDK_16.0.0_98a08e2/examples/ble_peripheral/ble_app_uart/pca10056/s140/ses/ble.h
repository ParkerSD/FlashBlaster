void advertising_init(void);
void gatt_init(void);
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt);
void ble_stack_init(void);
void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);
void ble_draw_icon(uint16_t color);
void on_adv_evt(ble_adv_evt_t ble_adv_evt);
void conn_params_init(void);
void conn_params_error_handler(uint32_t nrf_error);
void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
void services_init(void);
void nus_data_handler(ble_nus_evt_t * p_evt);
void ble_cmd_parser(void);
uint32_t add_file(void);
void add_chip(void);
void add_project(void);
uint32_t ble_parse_data_length(void);
uint32_t ble_parse_start_address(void);
char* ble_parse_name(void);
uint32_t char_string_to_int(uint8_t digits);
char* fetch_name(uint16_t length);
void nrf_qwr_error_handler(uint32_t nrf_error);
void gap_params_init(void);
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name);
void advertising_start(void);
void idle_state_handle(void);

void packet_write_first(void);
void packet_write(uint32_t data_start_addr);

bool string_compare(char* name, char* target_name, uint8_t name_length);
void ble_decode_chip_id(char* chip_name, uint8_t chip_name_length);