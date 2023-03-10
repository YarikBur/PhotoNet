#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/gpio.h"

#define WIFI_SSID "PhotoNet"
#define WIFI_PASS "2Z410p71@@nv"
#define EXAMPLE_MAX_STA_CONN (16)

#define GPIO_LED 2

static const char *TAG = "wifi softAP";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_id == WIFI_EVENT_AP_STACONNECTED)
  {
    wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, " station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);

    gpio_set_level(GPIO_LED, 1);
  }
  else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
  {
    wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, " station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
  
    gpio_set_level(GPIO_LED, 0);
  }
}

void wifi_init_softap()
{
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

  wifi_config_t wifi_config = {
      .ap = {
          .ssid = WIFI_SSID,
          .ssid_len = strlen(WIFI_SSID),
          .password = WIFI_PASS,
          .max_connection = EXAMPLE_MAX_STA_CONN,
          .authmode = WIFI_AUTH_WPA_WPA2_PSK
          },
  };
  if (strlen(WIFI_PASS) == 0)
  {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, " wifi_init_softap finished. SSID: %s password: %s", WIFI_SSID, WIFI_PASS);
}

void init_gpio()
{
  gpio_pad_select_gpio(GPIO_LED);
  gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);
}

void app_main()
{
  init_gpio();

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, " ESP_WIFI_MODE_AP");
  wifi_init_softap();
}