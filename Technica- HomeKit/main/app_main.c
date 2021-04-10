#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include <esp_log.h>
#include "driver/uart.h"
#include "driver/gpio.h"

#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

#include <iot_button.h>

#include <app_wifi.h>
#include <app_hap_setup_payload.h>

#include "nextion/nextion-esp32.h"
#include "nextion/page.h"
#include "nextion/component.h"

#include "lightbulb.h"
#include <bmp280.h>

#define SDA_GPIO 18
#define SCL_GPIO 19

#define LED_GPIO      2    
#define RELAY_GPIO    5   

static float pressure = 0.0;

static struct {
    bool Light_ActiveM;
    bool Fan_ActiveM;
    float TemperatureM;
    float HumidityM;
    float PressureM;
    int Led_BrightnessM;
} container;



hap_char_t *Light_On_Char;
hap_char_t *Fan_On_Char;
hap_char_t *TemperatureChar;
hap_char_t *HumidityChar;
hap_char_t *hap_char_current_relative_humidity_custom (float curr_rel_humidity)
{
    // hap_val_t HumidityValue;
    hap_char_t *hc = hap_char_float_create(HAP_CHAR_UUID_CURRENT_RELATIVE_HUMIDITY,
                                           HAP_CHAR_PERM_PR | HAP_CHAR_PERM_EV, curr_rel_humidity);

    hap_char_float_set_constraints(hc, 0.0, 100.0, 1.0);
    hap_char_add_unit(hc, HAP_CHAR_UNIT_PERCENTAGE);
    // HumidityValue.f = container.HumidityM;
    // hap_char_update_val(hc,&HumidityValue);
    return hc;
}

hap_char_t *hap_char_current_temperature_custom (float curr_temp)
{
    
    hap_char_t *hc = hap_char_float_create(HAP_CHAR_UUID_CURRENT_TEMPERATURE,HAP_CHAR_PERM_PR | HAP_CHAR_PERM_EV, curr_temp);

    hap_char_float_set_constraints(hc, -30.0, 100.0, 0.1);
    hap_char_add_unit(hc, HAP_CHAR_UNIT_CELSIUS);

    return hc;
}

const static char *NEX_TAG = "NEX_TAG";

static nextion_handle_t nextion_handle;
static TaskHandle_t task_interface_handle;


void process_touch_event(const nextion_handle_t handle,
                         uint8_t page_id,
                         uint8_t component_id,
                         nextion_touch_state_t state)
{
    if (page_id == 0 && state == NEXTION_TOUCH_RELEASED)
    {
        hap_val_t ActiveValue;
        switch(component_id){
            case 2:
                ESP_LOGI(NEX_TAG, "acc1 pressed");
                container.Light_ActiveM = !container.Light_ActiveM;
                lightbulb_set_on(container.Light_ActiveM);
                ActiveValue.i = container.Light_ActiveM;
                hap_char_update_val(Light_On_Char, &ActiveValue);
                break;
            case 3:
                ESP_LOGI(NEX_TAG, "acc2 pressed");
                container.Fan_ActiveM = !container.Fan_ActiveM;
                //lightbulb_set_on(container.Fan_ActiveM);
                ActiveValue.i = container.Fan_ActiveM;
                hap_char_update_val(Fan_On_Char, &ActiveValue);
                break;
            case 4:
                ESP_LOGI(NEX_TAG, "acc3 pressed");
                
                break;
        }
    }
}

void do_interface_logic()
{
    do
    {
        nextion_event_process(nextion_handle);
        char temp_string[50];
        int a = container.TemperatureM*10;
        sprintf(temp_string,"%d",a);
        // if (nextion_component_set_text(nextion_handle, "te", temp_string,strlen(temp_string)) == NEX_OK)
        // {
        //     ESP_LOGI(NEX_TAG, "Setting te text as %s", temp_string);
        // }
        if (nextion_component_set_number(nextion_handle, "te", a) == NEX_OK)
        {
            ESP_LOGI(NEX_TAG, "Setting te value as %d", a);
        }
        a = container.HumidityM*10;
        if (nextion_component_set_number(nextion_handle, "hu", a) == NEX_OK)
        {
            ESP_LOGI(NEX_TAG, "Setting te value as %d", a);
        }
        a = (container.PressureM/100000.0);
        a = (a==0)?10:(a*10);
        if (nextion_component_set_number(nextion_handle, "pr", a) == NEX_OK)
        {
            ESP_LOGI(NEX_TAG, "Setting te value as %d", a);
        }
        if (nextion_component_set_boolean(nextion_handle, "acc1", container.Light_ActiveM) == NEX_OK)
        {
            ESP_LOGI(NEX_TAG, "acc1 val is %d", container.Light_ActiveM);
        }
        else{
            ESP_LOGI(NEX_TAG, "could not set acc val = %d", container.Light_ActiveM);
        }
        if (nextion_component_set_boolean(nextion_handle, "acc2", container.Fan_ActiveM) == NEX_OK)
        {
            ESP_LOGI(NEX_TAG, "acc2 val is %d", container.Fan_ActiveM);
        }
        else{
            ESP_LOGI(NEX_TAG, "could not set acc val = %d", container.Fan_ActiveM);
        }
        if (nextion_component_set_number(nextion_handle, "h0", container.Led_BrightnessM) == NEX_OK)
        {
            ESP_LOGI(NEX_TAG, "slider h0 val is %d", container.Led_BrightnessM);
        }
        else{
            ESP_LOGI(NEX_TAG, "could not set slider h0 val = %d", container.Led_BrightnessM);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    } while (true);
}


static const char *TAG = "HAP Bridge";

#define BRIDGE_TASK_PRIORITY  1
#define BRIDGE_TASK_STACKSIZE 4 * 1024
#define BRIDGE_TASK_NAME      "hap_bridge"

#define NUM_BRIDGED_ACCESSORIES 3

/* Reset network credentials if button is pressed for more than 3 seconds and then released */
#define RESET_NETWORK_BUTTON_TIMEOUT        3

/* Reset to factory if button is pressed and held for more than 10 seconds */
#define RESET_TO_FACTORY_BUTTON_TIMEOUT     10

void relay_write(bool on, int gpio) {
    gpio_set_level(gpio, on ? 1 : 0);
}

void led_write(bool on, int gpio) {
    gpio_set_level(gpio, on ? 0 : 1);
}

/* The button "Boot" will be used as the Reset button for the example */
#define RESET_GPIO  GPIO_NUM_0
/**
 * @brief The network reset button callback handler.
 * Useful for testing the Wi-Fi re-configuration feature of WAC2
 */
static void reset_network_handler(void* arg)
{
    hap_reset_network();
}
/**
 * @brief The factory reset button callback handler.
 */
static void reset_to_factory_handler(void* arg)
{
    hap_reset_to_factory();
}
static void button_press_event(void* arg)
{
    hap_val_t ActiveValue;
    container.Light_ActiveM = !container.Light_ActiveM;
    ESP_LOGI(TAG, "Button Pressed");
    lightbulb_set_on(container.Light_ActiveM);
    ActiveValue.i = container.Light_ActiveM;
    hap_char_update_val(Light_On_Char, &ActiveValue);
    // NEXTION UPDATE
    
}
/**
 * The Reset button  GPIO initialisation function.
 * Same button will be used for resetting Wi-Fi network as well as for reset to factory based on
 * the time for which the button is pressed.
 */
static void reset_key_init(uint32_t key_gpio_pin)
{
    button_handle_t handle = iot_button_create(key_gpio_pin, BUTTON_ACTIVE_LOW);
    iot_button_set_evt_cb(handle, BUTTON_CB_RELEASE, button_press_event, NULL);
    iot_button_add_on_release_cb(handle, RESET_NETWORK_BUTTON_TIMEOUT, reset_network_handler, NULL);
    iot_button_add_on_press_cb(handle, RESET_TO_FACTORY_BUTTON_TIMEOUT, reset_to_factory_handler, NULL);
}
static void device_identify_task(void *_args) {
    for (int i=0; i<3; i++) {
      for (int j=0; j<2; j++) {
          led_write(true, LED_GPIO);
          vTaskDelay(100 / portTICK_PERIOD_MS);
          led_write(false, LED_GPIO);
          vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led_write(false, LED_GPIO);
    vTaskDelete(NULL);
}

static void InitializePlatform() {

    container.Light_ActiveM = 0;
    container.TemperatureM = 26.8; 
    container.HumidityM = 50.0;
    
    gpio_config_t io_conf = {0};

    io_conf.pin_bit_mask = (1ULL<<RELAY_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    
    io_conf.pin_bit_mask = (1ULL<<LED_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    
    relay_write(container.Light_ActiveM, RELAY_GPIO);
    led_write(container.Light_ActiveM, LED_GPIO);
}
/* Mandatory identify routine for the accessory (bridge)
 * In a real accessory, something like LED blink should be implemented
 * got visual identification
 */
static int bridge_identify(hap_acc_t *ha)
{
    ESP_LOGI(TAG, "Bridge identified");
    return HAP_SUCCESS;
}

/* Mandatory identify routine for the bridged accessory
 * In a real bridge, the actual accessory must be sent some request to
 * identify itself visually
 */
static int accessory_identify(hap_acc_t *ha)
{
    hap_serv_t *hs = hap_acc_get_serv_by_uuid(ha, HAP_SERV_UUID_ACCESSORY_INFORMATION);
    hap_char_t *hc = hap_serv_get_char_by_uuid(hs, HAP_CHAR_UUID_NAME);
    const hap_val_t *val = hap_char_get_val(hc);
    char *name = val->s;
    xTaskCreate(device_identify_task, "Device identify", 512, NULL, 2, NULL);
    ESP_LOGI(TAG, "Bridged Accessory %s identified", name);
    return HAP_SUCCESS;
}

void temperature_sensor_task(void *_args) {

    hap_val_t TemperatureValue;
    hap_val_t HumidityValue;
    bmp280_params_t params;
    bmp280_t dev;
    bmp280_init_default_params(&params);
    memset(&dev, 0, sizeof(bmp280_t));

    ESP_ERROR_CHECK(bmp280_init_desc(&dev, BMP280_I2C_ADDRESS_0, 0, SDA_GPIO, SCL_GPIO));
    ESP_ERROR_CHECK(bmp280_init(&dev, &params));

    bool bme280p = dev.id == BME280_CHIP_ID;
    printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

    while (1) {
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        if (bmp280_read_float(&dev, &container.TemperatureM, &pressure, &container.HumidityM) != ESP_OK)
        {
            printf("Temperature/pressure reading failed\n");
            continue;
        }
    //  if (bmp280_read_float(&dev, &container.TemperatureM, &pressure, &container.HumidityM) != ESP_OK)
    //     ESP_LOGI(TAG, "Read Temp: %0.01fC Humidity: %0.01f%% ", container.TemperatureM, container.HumidityM);
    //  else
    //     ESP_LOGE(TAG, "Could not read data from BMP sensor");

        printf("Pressure: %.2f Pa, Temperature: %.2f C", pressure, container.TemperatureM);
        if (bme280p)
            printf(", Humidity: %.2f\n", container.HumidityM);
        else
            printf("\n");
        TemperatureValue.f = (float)container.TemperatureM;
        HumidityValue.f = container.HumidityM;
        hap_char_update_val(TemperatureChar, &TemperatureValue);
        hap_char_update_val(HumidityChar, &HumidityValue);
    }
}

static int temperature_read(hap_char_t *hc, hap_status_t *status, void *serv_priv, void *read_priv)
{
    int ret = HAP_SUCCESS;
    if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_CURRENT_TEMPERATURE)) {
        hap_val_t TemperatureValue;
        // if (bmp280_read_float(&dev, &container.TemperatureM, &pressure, &container.HumidityM) != ESP_OK)
        //     ESP_LOGI(TAG, "Read Temp: %0.01fC Humidity: %0.01f%% ", container.TemperatureM, container.HumidityM);
        // else
        //     ESP_LOGE(TAG, "Could not read data from BMP sensor");

        TemperatureValue.f = (float)container.TemperatureM;
        ESP_LOGI(TAG, "The current temperature is %f", TemperatureValue.f);
        hap_char_update_val(hc, &TemperatureValue);
        *status = HAP_STATUS_SUCCESS;
    } else {
        *status = HAP_STATUS_RES_ABSENT;
        ret = HAP_FAIL;
    }
    return ret;
}

static int humidity_read(hap_char_t *hc, hap_status_t *status, void *serv_priv, void *read_priv)
{
    int ret = HAP_SUCCESS;
    if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_CURRENT_RELATIVE_HUMIDITY)) {
        hap_val_t HumidityValue;
        // if (bmp280_read_float(&dev, &container.TemperatureM, &pressure, &container.HumidityM) != ESP_OK)
        //     ESP_LOGI(TAG, "Read Temp: %0.01fC Humidity: %0.01f%% ", container.TemperatureM, container.HumidityM);
        // else
        //     ESP_LOGE(TAG, "Could not read data from BMP sensor");
        HumidityValue.f = container.HumidityM;
        ESP_LOGI(TAG, "The current humidity is %f", HumidityValue.f);
        hap_char_update_val(hc, &HumidityValue);
        *status = HAP_STATUS_SUCCESS;
    } else {
        *status = HAP_STATUS_RES_ABSENT;
        ret = HAP_FAIL;
    }
    return ret;
}
/* A dummy callback for handling a write on the "On" characteristic of Fan.
 * In an actual accessory, this should control the hardware
 */
static int fan_on(bool value)
{
    ESP_LOGI(TAG, "Received Write. Fan %s", value ? "On" : "Off");
    /* TODO: Control Actual Hardware */
    return 0;
}

/* A dummy callback for handling a write on the "On" characteristic of Fan.
 * In an actual accessory, this should control the hardware
 */
static int fan_write(hap_write_data_t write_data[], int count,
        void *serv_priv, void *write_priv)
{
    ESP_LOGI(TAG, "Write called for Accessory %s", (char *)serv_priv);
    int i, ret = HAP_SUCCESS;
    hap_write_data_t *write;
    for (i = 0; i < count; i++) {
        write = &write_data[i];
        if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) {
            fan_on(write->val.b);
            hap_char_update_val(write->hc, &(write->val));
            *(write->status) = HAP_STATUS_SUCCESS;
            container.Fan_ActiveM = write->val.b;
        } else {
            *(write->status) = HAP_STATUS_RES_ABSENT;
        }
    }
    return ret;
}
/* Callback for handling writes on the Light Bulb Service
 */
static int lightbulb_write(hap_write_data_t write_data[], int count,void *serv_priv, void *write_priv)
{
    int i, ret = HAP_SUCCESS;
    hap_write_data_t *write;
    for (i = 0; i < count; i++) {
        write = &write_data[i];
        /* Setting a default error value */
        *(write->status) = HAP_STATUS_VAL_INVALID;
        if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) {
            ESP_LOGI(TAG, "Received Write for Light %s", write->val.b ? "On" : "Off");
            if (lightbulb_set_on(write->val.b) == 0) {
                container.Light_ActiveM = write->val.b;
                *(write->status) = HAP_STATUS_SUCCESS;
            }
        } else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_BRIGHTNESS)) {
            ESP_LOGI(TAG, "Received Write for Light Brightness %d", write->val.i);
            if (lightbulb_set_brightness(write->val.i) == 0) {
                *(write->status) = HAP_STATUS_SUCCESS;
                container.Led_BrightnessM = write->val.i;
            }
        } 
        else {
            *(write->status) = HAP_STATUS_RES_ABSENT;
        }
        /* If the characteristic write was successful, update it in hap core
         */
        if (*(write->status) == HAP_STATUS_SUCCESS) {
            hap_char_update_val(write->hc, &(write->val));
        } else {
            /* Else, set the return value appropriately to report error */
            ret = HAP_FAIL;
        }
    }
    return ret;
}



/*The main thread for handling the Bridge Accessory */
static void bridge_thread_entry(void *p)
{

    InitializePlatform();

    hap_acc_t *tempaccessory = NULL;
    hap_serv_t *tempservice = NULL;
    hap_serv_t *humidityservice = NULL;
    hap_acc_t *Fan_Accessory = NULL;
    hap_serv_t *Fan_Service = NULL;
    hap_acc_t *Bulb_Accessory = NULL;
    hap_serv_t *Bulb_Service = NULL;
    hap_acc_t *accessory;


    /* Initialize the HAP core */
    hap_init(HAP_TRANSPORT_WIFI);

    /* Initialise the mandatory parameters for Accessory which will be added as
     * the mandatory services internally
     */
    hap_acc_cfg_t cfg = {
        .name = "Esp-Bridge",
        .manufacturer = "VIT",
        .model = "quatre carres",
        .serial_num = "SN0123456789",
        .fw_rev = "0.1",
        .hw_rev = NULL,
        .pv = "1.1.0",
        .identify_routine = bridge_identify,
        .cid = HAP_CID_BRIDGE,
    };
    /* Create accessory object */
    accessory = hap_acc_create(&cfg);

    /* Add a dummy Product Data */
    uint8_t product_data[] = {'Q','U','A','T','R','E','-','C','A','R','R','E','S'};
    hap_acc_add_product_data(accessory, product_data, sizeof(product_data));
    /* Add the Accessory to the HomeKit Database */
    hap_add_accessory(accessory);


    tempaccessory = hap_acc_create(&cfg);
    hap_acc_add_product_data(tempaccessory, product_data, sizeof(product_data));

    ESP_LOGI(TAG, "Creating temperture service (current temp: %0.01fC)", container.TemperatureM);
    /* Create the temp Service. Include the "name" since this is a user visible service  */
    tempservice = hap_serv_create(HAP_SERV_UUID_TEMPERATURE_SENSOR);
    hap_serv_add_char(tempservice, hap_char_name_create("ESP Temperature Sensor"));
    hap_serv_add_char(tempservice, hap_char_current_temperature_custom(container.TemperatureM));

    TemperatureChar = hap_serv_get_char_by_uuid(tempservice, HAP_CHAR_UUID_CURRENT_TEMPERATURE);

    /* Set the read callback for the service (optional) */
    hap_serv_set_read_cb(tempservice, temperature_read);
    /* Add the Garage Service to the Accessory Object */
    hap_acc_add_serv(tempaccessory, tempservice);


    ESP_LOGI(TAG, "Creating humidity service (current humidity: %0.01f%%)", container.HumidityM);
    /* Create the temp Service. Include the "name" since this is a user visible service  */
    humidityservice = hap_serv_create(HAP_SERV_UUID_HUMIDITY_SENSOR);
    hap_serv_add_char(tempservice, hap_char_name_create("ESP Humidity Sensor"));
    hap_serv_add_char(humidityservice, hap_char_current_relative_humidity_custom(container.HumidityM)); 

    HumidityChar = hap_serv_get_char_by_uuid(humidityservice, HAP_CHAR_UUID_CURRENT_RELATIVE_HUMIDITY);
    /* Set the read callback for the service (optional) */
    hap_serv_set_read_cb(humidityservice, humidity_read);
    /* Add the Humidity Service to the Accessory Object */
    hap_acc_add_serv(tempaccessory, humidityservice);

    hap_add_bridged_accessory(tempaccessory, hap_get_unique_aid("TEMPERATURE"));

    Fan_Accessory = hap_acc_create(&cfg);
    Fan_Service = hap_serv_fan_create(false);
    hap_serv_add_char(Fan_Service, hap_char_name_create("Fan"));
    hap_serv_set_priv(Fan_Service, strdup("Fan"));

    /* Set the write callback for the service */     
    hap_serv_set_write_cb(Fan_Service, fan_write);
 
    /* Add the Fan Service to the Accessory Object */
    hap_acc_add_serv(Fan_Accessory, Fan_Service);

    /* Add the Accessory to the HomeKit Database */
    hap_add_bridged_accessory(Fan_Accessory, hap_get_unique_aid("Fan"));

    hap_acc_cfg_t bulb_cfg = {
        .name = "ESP-LightBulb",
        .manufacturer = "VIT",
        .model = "quatre carres",
        .serial_num = "SN0123456789",
        .fw_rev = "0.1",
        .hw_rev = NULL,
        .pv = "1.1.0",
        .identify_routine = accessory_identify,
        .cid = HAP_CID_LIGHTING,
    };

    Bulb_Accessory = hap_acc_create(&bulb_cfg);
    Bulb_Service = hap_serv_lightbulb_create(true);

    int ret = hap_serv_add_char(Bulb_Service, hap_char_name_create("My Light"));
    //hap_serv_add_char(Bulb_Service, hap_char_on_create(0));
    ret |= hap_serv_add_char(Bulb_Service, hap_char_brightness_create(50));
    Light_On_Char = hap_serv_get_char_by_uuid(Bulb_Service, HAP_CHAR_UUID_ON);
    /* Set the write callback for the service */
    hap_serv_set_write_cb(Bulb_Service, lightbulb_write);
    
    /* Add the Light Bulb Service to the Accessory Object */
    hap_acc_add_serv(Bulb_Accessory, Bulb_Service);

    hap_add_bridged_accessory(Bulb_Accessory, hap_get_unique_aid("My Light"));

    lightbulb_init();
    
    reset_key_init(RESET_GPIO);


#ifdef CONFIG_EXAMPLE_USE_HARDCODED_SETUP_CODE
    /* Unique Setup code of the format xxx-xx-xxx. Default: 111-22-333 */
    hap_set_setup_code(CONFIG_EXAMPLE_SETUP_CODE);
    /* Unique four character Setup Id. Default: ES32 */
    hap_set_setup_id(CONFIG_EXAMPLE_SETUP_ID);
#ifdef CONFIG_APP_WIFI_USE_WAC_PROVISIONING
    app_hap_setup_payload(CONFIG_EXAMPLE_SETUP_CODE, CONFIG_EXAMPLE_SETUP_ID, true, cfg.cid);
#else
    app_hap_setup_payload(CONFIG_EXAMPLE_SETUP_CODE, CONFIG_EXAMPLE_SETUP_ID, false, cfg.cid);
#endif
#endif

    /* Enable Hardware MFi authentication (applicable only for MFi variant of SDK) */
    hap_enable_mfi_auth(HAP_MFI_AUTH_HW);

    /* Initialize Wi-Fi */
    app_wifi_init();

    /* After all the initializations are done, start the HAP core */
    hap_start();
    /* Start Wi-Fi */
    app_wifi_start(portMAX_DELAY);
    /* The task ends here. The read/write callbacks will be invoked by the HAP Framework */
  
    //xTaskCreate(temperature_sensor_task, "Temperature Sensor", 1024, NULL, 4, NULL);
    

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(temperature_sensor_task, "Temperature Sensor", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
    
    xTaskCreate(bridge_thread_entry, BRIDGE_TASK_NAME, BRIDGE_TASK_STACKSIZE, NULL, BRIDGE_TASK_PRIORITY, NULL);

    nextion_handle = nextion_driver_install(UART_NUM_2, 9600, GPIO_NUM_17, GPIO_NUM_16);

    if (nextion_handle == NULL)
    {
        ESP_LOGE(NEX_TAG, "could not install nextion driver");
        return;
    }

    if (nextion_init(nextion_handle) != NEX_OK)
    {
        ESP_LOGE(NEX_TAG, "could not init nextion device");
        return;
    }

    nextion_event_callback_t callback;
    callback.on_touch = &process_touch_event;

    nextion_event_callback_set(nextion_handle, callback);

    if (nextion_page_set(nextion_handle, "0") != NEX_OK)
    {
        ESP_LOGE(NEX_TAG, "could not change to page 0");
        return;
    }

    if (xTaskCreate(do_interface_logic,
                    "do_interface_logic",
                    4096,
                    NULL,
                    20,
                    &task_interface_handle) != pdPASS)
    {
        ESP_LOGE(NEX_TAG, "could not create task");
        return;
    }

    vTaskSuspend(NULL);

    if (!nextion_driver_delete(nextion_handle))
    {
        ESP_LOGE(NEX_TAG, "could not delete nextion driver");
    }
}

