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
void add_file(void);
void add_chip(void);
void add_project(void);
void ble_parse_data_length(void);
char* ble_parse_name(void);
void char_string_to_int(uint8_t digits);
char* fetch_name(uint16_t length);
void nrf_qwr_error_handler(uint32_t nrf_error);
void gap_params_init(void);
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name);
void advertising_start(void);
void idle_state_handle(void);

uint32_t seek_to_project(char* project_name, uint8_t length);

uint32_t seek_to_chip(uint32_t project_addr, char* chip_name, uint8_t length);

void append_file_addr(uint32_t chip_addr);