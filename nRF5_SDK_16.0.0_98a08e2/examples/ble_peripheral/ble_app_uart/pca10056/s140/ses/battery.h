


#define NO_CHARGE 0x0170   //2.6 volts
#define ONE_BAR_MAX_VOLT 0x0185
#define LOW_CHARGE 0x01A0  //3.0 volts
#define THREE_BAR_MAX_VOLT 0x01D0
#define HALF_CHARGE 0x0205 //3.5 volts
#define FIVE_BAR_MAX_VOLT 0x0210
#define SIX_BAR_MAX_VOLT 0x0218
#define SEVEN_BAR_MAX_VOLT 0x0226
#define EIGHT_BAR_MAX_VOLT 0x0234
#define FULL_CHARGE 0x0250 //4.2 volts


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