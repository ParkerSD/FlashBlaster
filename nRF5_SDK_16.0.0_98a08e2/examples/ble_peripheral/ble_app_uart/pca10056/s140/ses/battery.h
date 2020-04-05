
#define FULL_CHARGE 10
#define HALF_CHARGE 5
#define NO_CHARGE 0 

typedef struct battery battery_struct; // needed if multiple structs declared in single file
typedef struct battery
{
    uint8_t state_of_charge; // 0 - 10 
    bool charging_state; // draw lightning bolt 
    bool battery_low; // draw red 
    bool battery_icon_present; 
    

}battery_struct;

//ADC functions
void saadc_sampling_event_init(void);
void saadc_sampling_event_enable(void);
void saadc_init(void);
void adc_init(void);



void battery_init(void);

void battery_draw_percent(uint8_t percent);

void battery_draw_charging(void);

void battery_draw_outline(void);