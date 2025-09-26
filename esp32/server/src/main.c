#include "setting.h"
// #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "setting.h"
#include "driver/uart.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// UUIDs
#define UART_SVC_UUID 0xBBB0
#define UART_CHR_UUID 0xBBB1

#define BUF_SIZE (2 * SOC_UART_FIFO_LEN)

static uint16_t uart_chr_val_handle;
static uint16_t conn_handle = 0;

uint8_t buffer[BUFLEN] = {0};
struct os_mbuf *txom;

static void notify_task(void *param);
static void start_advertising(void);

// === GATT Access Callback (client read/write) ===
static int gatt_svr_chr_access_uart(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        break;
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        break;

    default:
        break;
    }
    return 0;
}

// === GATT Database ===
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(UART_SVC_UUID),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = BLE_UUID16_DECLARE(UART_CHR_UUID),
                .access_cb = gatt_svr_chr_access_uart,
                .val_handle = &uart_chr_val_handle,
                .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
            },
            {0} // end
        },
    },
    {0} // end
};

// === GAP Event Handler ===
static int gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0)
        {
            conn_handle = event->connect.conn_handle;
            txom = ble_hs_mbuf_from_flat(buffer, sizeof(buffer));
            ble_gatts_notify_custom(conn_handle, uart_chr_val_handle, txom);
        }
        else
        {
            start_advertising();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        conn_handle = 0;

        start_advertising();
        break;

    default:
        break;
    }
    return 0;
}

// === Advertising ===
static void start_advertising(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.name = (uint8_t *)"ESP-Server";
    fields.name_len = strlen("ESP-Server");
    fields.name_is_complete = 1;

    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.tx_pwr_lvl_is_present = 1;

    fields.uuids16 = (ble_uuid16_t[]){BLE_UUID16_INIT(UART_SVC_UUID)};
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params advp;
    memset(&advp, 0, sizeof(advp));
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                      &advp, gap_event, NULL);
}

// === Notifier Task ===
static void notify_task(void *param)
{
    while (1)
    {
        int len = uart_read_bytes(UART_NUM_0, buffer, sizeof(buffer), portMAX_DELAY); // portMAX_DELAY Might not work

        if (len == BUFLEN)
        {
            txom = ble_hs_mbuf_from_flat(buffer, sizeof(buffer));
            ble_gatts_notify_custom(conn_handle, uart_chr_val_handle, txom);
        }
    }
}

// === Host Sync Callback ===
static void on_sync(void)
{
    //  Set device name
    ble_svc_gap_device_name_set("ESP-Server");

    // Start advertising
    start_advertising();
}

// === Main Host Task ===
void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

// === app_main ===
void app_main(void)
{
    const uart_config_t uart_config = {
        .baud_rate = BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(UART_NUM_0, BUF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Init NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Init NimBLE
    nimble_port_init();
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ble_hs_cfg.sync_cb = on_sync;

    // Start host task
    nimble_port_freertos_init(host_task);

    // Start notifier task
    xTaskCreate(notify_task, "notify_task", 2048, NULL, 1, NULL);
}
