
#define FULL_CHARGE 0x0250 //4.2 volts
#define HALF_CHARGE 0x0205 //3.5 volts
#define LOW_CHARGE 0x01A0  //3.0 volts
#define NO_CHARGE 0x0170   //2.6 volts


typedef struct battery battery_struct; // needed if multiple structs declared in single file
typedef struct battery
{
    bool charging_state; // draw lightning bolt 
    bool battery_icon_present; 
}battery_struct;

//ADC functions
void saadc_sampling_event_init(void);
void saadc_sampling_event_enable(void);
void saadc_init(void);

void adc_init(void);
void adc_average(void);


void battery_init(void);

void battery_draw_icon(void);

void battery_draw_charging(uint16_t);

void battery_draw_outline(uint16_t);

void battery_set_charging_state(bool); 