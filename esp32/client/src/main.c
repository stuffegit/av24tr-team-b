#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "setting.h"
#include "esp_bt.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nimble/nimble_port_freertos.h"

#define BUF_SIZE (BUFLEN * SOC_UART_FIFO_LEN)
#define DEVICE_NAME "TEAM_B_CLIENT"

#define BLE_SVC_UUID16 0xBBB0     // Service uuid
#define BLE_SVC_CHR_UUID16 0xBBB1 // Characteristic uuid

static int server_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t own_addr_type;
static uint16_t connection;
static ble_addr_t peer_addr;
static uint16_t chrval_handle;

static void peripheral_scan(void)
{
    struct ble_gap_disc_params disc_params = {0};
    disc_params.passive = 0;
    disc_params.filter_duplicates = 1;
    disc_params.itvl = BLE_GAP_ADV_ITVL_MS(20);
    disc_params.window = BLE_GAP_ADV_ITVL_MS(20);
    assert(0 == ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, server_gap_event, NULL));
}

static void peripheral_connect(const struct ble_gap_disc_desc *disc)
{
    int status = ble_gap_disc_cancel();

    if (status == 0)
    {
        struct ble_gap_conn_params conn_params = {
            .scan_itvl = 0x0010,   // 10 ms
            .scan_window = 0x0010, // 10 ms
            .itvl_min = 24,        // 30 ms
            .itvl_max = 40,        // 50 ms
            .latency = 0,
            .supervision_timeout = 400, // 4 sec
            .min_ce_len = 0,
            .max_ce_len = 0,
        };
        status = ble_gap_connect(own_addr_type, &disc->addr, 3000, &conn_params, server_gap_event, NULL);

        if (status != 0)
        {
        }
    }
    else
    {
    }
}

static int on_descriptor_discovery(uint16_t conn_handle, const struct ble_gatt_error *error, uint16_t chr_val_handle, const struct ble_gatt_dsc *dsc, void *)
{
    if (error->status == 0 && (dsc != NULL))
    {
        if (0 == ble_uuid_cmp(&dsc->uuid.u, BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16)))
        {
            uint8_t value[2] = {1, 0};
            assert(0 == ble_gattc_write_flat(conn_handle, dsc->handle, value, sizeof(value), NULL, NULL));
        }
    }
    else if (error->status == BLE_HS_EDONE)
    {
    }
    else
    {
        ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }

    return 0;
}

static int on_characteristic_discovery(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg)
{
    if ((error->status == 0) && (chr != NULL))
    {
        chrval_handle = chr->val_handle;

        assert(0 == ble_gattc_disc_all_dscs(conn_handle, chr->val_handle, chr->val_handle + 1, on_descriptor_discovery, NULL));
    }
    else if (error->status == BLE_HS_EDONE)
    {
    }
    else
    {
        ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }

    return 0;
}

static int on_service_discovery(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_svc *service, void *arg)
{
    if ((error->status == 0) && (service != NULL))
    {
        uint16_t svc_start_handle = service->start_handle;
        uint16_t svc_end_handle = service->end_handle;

        assert(0 == ble_gattc_disc_chrs_by_uuid(conn_handle, svc_start_handle, svc_end_handle,
                                                BLE_UUID16_DECLARE(BLE_SVC_CHR_UUID16), on_characteristic_discovery, NULL));
    }
    else if (error->status == BLE_HS_EDONE)
    {
    }
    else
    {
        ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }

    return 0;
}

static int server_gap_event(struct ble_gap_event *event, void *)
{
    int status = 0;
    struct ble_gap_conn_desc desc;
    struct ble_hs_adv_fields fields;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
        status = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
        if (status == 0)
        {
            for (int i = 0; i < fields.num_uuids16; i++)
            {
                if (ble_uuid_u16(&fields.uuids16[i].u) == BLE_SVC_UUID16)
                {
                    peripheral_connect(&event->disc);
                    break;
                }
            }
        }
        break;

    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0)
        {
            assert(0 == ble_gap_conn_find(event->connect.conn_handle, &desc));
            connection = event->connect.conn_handle;
            memcpy(peer_addr.val, desc.peer_id_addr.val, sizeof(desc.peer_id_addr.val));
            assert(0 == ble_gattc_disc_svc_by_uuid(event->connect.conn_handle, BLE_UUID16_DECLARE(BLE_SVC_UUID16), on_service_discovery, NULL));
        }
        else
        {
            peripheral_scan();
        }

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        memset(peer_addr.val, 0, sizeof(peer_addr.val));
        chrval_handle = 0;

        peripheral_scan();
        break;

    case BLE_GAP_EVENT_DISC_COMPLETE:
        break;

    case BLE_GAP_EVENT_NOTIFY_RX:
        uint8_t buffer[BUFLEN];
        memset(buffer, 0, sizeof(buffer));
        assert(0 == os_mbuf_copydata(event->notify_rx.om, 0, sizeof(buffer), buffer));
        uart_write_bytes(UART_NUM_0, buffer, sizeof(buffer));
        uart_flush(UART_NUM_0);
        break;

    case BLE_GAP_EVENT_MTU:
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        break;

    default:
        break;
    }

    return status;
}

static void server_on_reset(int reason)
{
}

static void server_on_sync(void)
{
    own_addr_type = ble_hs_util_ensure_addr(BLE_OWN_ADDR_PUBLIC);
    uint8_t addr_val[6] = {0};
    ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
    peripheral_scan();
}

void server_task(void *pvParameters)
{
    (void)pvParameters;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    const uart_config_t uart_config = {
        .baud_rate = BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};

    uart_driver_install(UART_NUM_0, BUF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    esp_err_t status = nvs_flash_init();
    if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        status = nvs_flash_init();
    }
    ESP_ERROR_CHECK(status);

    ESP_ERROR_CHECK(nimble_port_init());
    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_hs_cfg.reset_cb = server_on_reset;
    ble_hs_cfg.sync_cb = server_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    assert(0 == ble_svc_gap_device_name_set(DEVICE_NAME));

    assert(pdTRUE == xTaskCreate(server_task, "server_task", 4096, NULL, 8, NULL));

    nimble_port_run();
    nimble_port_freertos_deinit();
}
