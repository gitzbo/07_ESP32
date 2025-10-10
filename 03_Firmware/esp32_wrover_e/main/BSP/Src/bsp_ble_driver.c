/**
 * @file bsp_ble_driver.c
 * @author ZB (2536566200@qq.com)
 * @brief 蓝牙驱动
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "bsp_ble_driver.h"


#define TAG "BLE_DRIVER"

#define CID_ESP 0x02E5                                                  /**< 乐鑫的分配id号 */  
static uint8_t dev_uuid[16] = {0xdd, 0xdd};                             /**< 设备UUID */

/**< 配置服务参数 */
static esp_ble_mesh_cfg_srv_t config_server = {     
    /* 3 transmissions with 20ms interval */        
    .net_transmit       = ESP_BLE_MESH_TRANSMIT(2, 20),                 /**< 网络传输状态 */
    .relay              = ESP_BLE_MESH_RELAY_DISABLED,                  /**< 中继模式状态 */
    .relay_retransmit   = ESP_BLE_MESH_TRANSMIT(2, 20),                 /**< 中继重传状态 */
    .beacon             = ESP_BLE_MESH_BEACON_ENABLED,                  /**< 安全网络信标状态 */
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)          
    .gatt_proxy         = ESP_BLE_MESH_GATT_PROXY_ENABLED,              /**< GATT代理状态 */
#else       
    .gatt_proxy         = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,        /**< GATT代理状态 */
#endif      
#if defined(CONFIG_BLE_MESH_FRIEND)     
    .friend_state       = ESP_BLE_MESH_FRIEND_ENABLED,      
#else       
    .friend_state       = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,            /**< 朋友节点状态 */
#endif      
    .default_ttl        = 7,                                            /**< 默认电平 */
};      

/**< 开关服务参数 */        
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_0, 2 + 3, ROLE_NODE);           /**< 设置默认参数 */
static esp_ble_mesh_gen_onoff_srv_t onoff_server_0 = {                  /**< 开关服务0 */
    .rsp_ctrl = {                                                       /**< 服务器模型接收消息的响应控制 */
        .get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,                   /**< 自动回复 */
        .set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,                   /**< 自动回复 */
    },      
};      

/**< 开关服务参数 */        
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_1, 2 + 3, ROLE_NODE);           /**< 设置默认参数 */
static esp_ble_mesh_gen_onoff_srv_t onoff_server_1 = {                  /**< 开关服务1 */
    .rsp_ctrl = {                                                       /**< 服务器模型接收消息的响应控制 */
        .get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,                 /**< 应用回复 */
        .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,                 /**< 应用回复 */
    },      
};      

/**< 开关服务参数 */        
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_2, 2 + 3, ROLE_NODE);           /**< 设置默认参数 */
static esp_ble_mesh_gen_onoff_srv_t onoff_server_2 = {                  /**< 开关服务2 */
    .rsp_ctrl = {                                                       /**< 服务器模型接收消息的响应控制 */
        .get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,                   /**< 自动回复 */
        .set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,                 /**< 应用回复 */
    },
};

static esp_ble_mesh_model_t root_models[] = {                           /**< root模型 */
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),                         /**< 配置默认的服务模型 */
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_0, &onoff_server_0),    /**< 设置开关服务0模型 */
};

static esp_ble_mesh_model_t extend_model_0[] = {                        /**< extend0模型 */
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_1, &onoff_server_1),    /**< 设置开关服务1模型 */
};

static esp_ble_mesh_model_t extend_model_1[] = {                        /**< extend1模型 */
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_2, &onoff_server_2),    /**< 设置开关服务2模型 */
};

static esp_ble_mesh_elem_t elements[] = {                               /**< mesh抽象元素 */
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),      /**< 定义一个空模型元素到模型中 */
    ESP_BLE_MESH_ELEMENT(0, extend_model_0, ESP_BLE_MESH_MODEL_NONE),   /**< 定义一个空模型元素到模型中 */
    ESP_BLE_MESH_ELEMENT(0, extend_model_1, ESP_BLE_MESH_MODEL_NONE),   /**< 定义一个空模型元素到模型中 */
};

static esp_ble_mesh_comp_t composition = {                              /**< 节点组合数据上下文 */
    .cid            = CID_ESP,                                          /**< 16 位 SIG 分配的公司标识符 */
    .element_count  = ARRAY_SIZE(elements),                             /**< 元素大小 */
    .elements       = elements,                                         /**< 元素对象 */
};

/* Disable OOB security for SILabs Android app */
static esp_ble_mesh_prov_t provision = {                                /**< 配置属性和能力 */
    .uuid               = dev_uuid,                                     /**< 设备UUID */
#if 0
    .output_size        = 4,                                            /**< 配置oob大小 */
    .output_actions     = ESP_BLE_MESH_DISPLAY_NUMBER,                  /**< 支持oob操作 */
    .input_size         = 4,                                            /**< 输入大小 */
    .input_actions      = ESP_BLE_MESH_PUSH,                            /**< 支持输入操作 */
#else
    .output_size        = 0,                                            /**< 配置oob大小 */
    .output_actions     = 0,                                            /**< 支持oob操作 */
#endif
};

static void prov_complete(  uint16_t net_idx, 
                            uint16_t addr, 
                            uint8_t flags, 
                            uint32_t iv_index)
{
    ESP_LOGI(TAG, "net_idx: 0x%04x, addr: 0x%04x", net_idx, addr);
    ESP_LOGI(TAG, "flags: 0x%02x, iv_index: 0x%08" PRIx32, flags, iv_index);
    // board_led_operation(LED_G, LED_OFF);
}

static void bsp_change_led_state(esp_ble_mesh_model_t *model,
                                 esp_ble_mesh_msg_ctx_t *ctx, 
                                 uint8_t onoff)
{
    uint16_t primary_addr = esp_ble_mesh_get_primary_element_address();
    uint8_t elem_count = esp_ble_mesh_get_element_count();
    // struct _led_state *led = NULL;
    uint8_t i;

    if (ESP_BLE_MESH_ADDR_IS_UNICAST(ctx->recv_dst)) {
        for (i = 0; i < elem_count; i++) {
            if (ctx->recv_dst == (primary_addr + i)) {
                // led = &led_state[i];
                // board_led_operation(led->pin, onoff);
            }
        }
    } else if (ESP_BLE_MESH_ADDR_IS_GROUP(ctx->recv_dst)) {
        if (esp_ble_mesh_is_model_subscribed_to_group(model, ctx->recv_dst)) {          /**< 检查模型是否已订阅指定的组地址 */
            // led = &led_state[model->element->element_addr - primary_addr];
            // board_led_operation(led->pin, onoff);
        }
    } else if (ctx->recv_dst == 0xFFFF) {
        // led = &led_state[model->element->element_addr - primary_addr];
        // board_led_operation(led->pin, onoff);
    }
}

static void bsp_handle_gen_onoff_msg(esp_ble_mesh_model_t *model,
                                     esp_ble_mesh_msg_ctx_t *ctx,
                                     esp_ble_mesh_server_recv_gen_onoff_set_t *set)
{
    esp_ble_mesh_gen_onoff_srv_t *srv = (esp_ble_mesh_gen_onoff_srv_t *)model->user_data;

    switch (ctx->recv_op) {
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:
        /**< 发送服务模式消息 */
        esp_ble_mesh_server_model_send_msg( model,                                      /**< 模型 */
                                            ctx,                                        /**< 消息 */
                                            ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,     /**< 通用开关操作码 */
                                            sizeof(srv->state.onoff),                   /**< 数据长度 */
                                            &srv->state.onoff);                         /**< 具体数据 */
        break;

    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
        if (set->op_en == false) {
            srv->state.onoff = set->onoff;
        } else {
            srv->state.onoff = set->onoff;
        }

        if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
            esp_ble_mesh_server_model_send_msg(model, 
                                                ctx,
                                                ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, 
                                                sizeof(srv->state.onoff), 
                                                &srv->state.onoff);
        }

        /**< 发送模型发送消息 */
        esp_ble_mesh_model_publish( model,                                      /**< 模型 */
                                    ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,     /**< 消息操作码 */
                                    sizeof(srv->state.onoff),                   /**< 消息长度 */
                                    &srv->state.onoff,                          /**< 要发送的负载参数 */
                                    ROLE_NODE);                                 /**< 发送角色 */
        bsp_change_led_state(model, ctx, srv->state.onoff);
        break;

    default:
        break;
    }
}

static void bsp_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                         esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:               /**< 初始化 BLE Mesh 配置功能及内部数据信息完成事件 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;

    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:            /**< 启用节点配置功能完成事件 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;

    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:              /**< 建立 BLE Mesh 链接事件 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
            param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;

    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:             /**< 关闭 BLE Mesh 链接事件 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
            param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;

    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:               /**< 配置完成事件 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(  param->node_prov_complete.net_idx, 
                        param->node_prov_complete.addr,
                        param->node_prov_complete.flags, 
                        param->node_prov_complete.iv_index);
        break;

    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:                  /**< 配置重置事件 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        break;

    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:    /**< 设置未配置设备名称完成事件 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    default:
        break;
    }
}

static void bsp_ble_mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                                           esp_ble_mesh_generic_server_cb_param_t *param)
{
    esp_ble_mesh_gen_onoff_srv_t *srv;
    ESP_LOGI(TAG, "event 0x%02x, opcode 0x%04" PRIx32 ", src 0x%04x, dst 0x%04x",
        event, param->ctx.recv_op, param->ctx.addr, param->ctx.recv_dst);

    switch (event) {
    case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:                                      /**< 接收状态消息回调 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||                    /**< 当消息是set或unack状态时触发 */
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            ESP_LOGI(TAG, "onoff 0x%02x", param->value.state_change.onoff_set.onoff);
            bsp_change_led_state(param->model, &param->ctx, param->value.state_change.onoff_set.onoff);
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:                                      /**< 接收回复消息回调 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET) {                    /**< 获取消息操作码 */
            srv = (esp_ble_mesh_gen_onoff_srv_t *)param->model->user_data;
            ESP_LOGI(TAG, "onoff 0x%02x", srv->state.onoff);
            bsp_handle_gen_onoff_msg(param->model, &param->ctx, NULL);
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:                                      /**< 接收设置消息回调 */
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||    
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            ESP_LOGI(TAG, "onoff 0x%02x, tid 0x%02x", 
                            param->value.set.onoff.onoff, 
                            param->value.set.onoff.tid);

            if (param->value.set.onoff.op_en) {
                ESP_LOGI(TAG, "trans_time 0x%02x, delay 0x%02x",
                                param->value.set.onoff.trans_time, 
                                param->value.set.onoff.delay);
            }
            bsp_handle_gen_onoff_msg(param->model, &param->ctx, &param->value.set.onoff);   /**< 处理onoff消息 */
        }
        break;
    default:
        ESP_LOGE(TAG, "Unknown Generic Server event 0x%02x", event);
        break;
    }
}

static void bsp_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                              esp_ble_mesh_cfg_server_cb_param_t *param)
{
    /**< 判断是否为配置服务状态 */
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:                     /**< 配置密钥添加 */
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x",
                param->value.state_change.appkey_add.net_idx,
                param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:                  /**< 配置应用绑定 */
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_app_bind.element_addr,
                param->value.state_change.mod_app_bind.app_idx,
                param->value.state_change.mod_app_bind.company_id,
                param->value.state_change.mod_app_bind.model_id);
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:                   /**< 配置订阅添加 */
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
            ESP_LOGI(TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_sub_add.element_addr,
                param->value.state_change.mod_sub_add.sub_addr,
                param->value.state_change.mod_sub_add.company_id,
                param->value.state_change.mod_sub_add.model_id);
            break;

        default:
            break;
        }
    }
}

static esp_err_t ble_mesh_init(void)
{
    esp_err_t err = ESP_OK;

    /**< 将回调注册到驱动中 */
    esp_ble_mesh_register_prov_callback(bsp_ble_mesh_provisioning_cb);                  /**< 注册 BLE Mesh 配置回调。 */
    esp_ble_mesh_register_config_server_callback(bsp_ble_mesh_config_server_cb);        /**< 注册 BLE Mesh 配置服务器模型回调 */
    esp_ble_mesh_register_generic_server_callback(bsp_ble_mesh_generic_server_cb);      /**< 注册 BLE Mesh 通用服务器模型回调 */

    /**< 蓝牙mesh初始化 */
    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

    /**< 启动特定配置 */
    err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh node (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized");

    // board_led_operation(LED_G, LED_ON);
    
    return err;
}

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    if (dev_uuid == NULL) {
        ESP_LOGE(TAG, "%s, Invalid device uuid", __func__);
        return;
    }

    /* Copy device address to the device uuid with offset equals to 2 here.
     * The first two bytes is used for matching device uuid by Provisioner.
     * And using device address here is to avoid using the same device uuid
     * by different unprovisioned devices.
     */
    memcpy(dev_uuid + 2, esp_bt_dev_get_address(), BD_ADDR_LEN);                    /**< 获取设备UUID */
}

bsp_ble_driver_status_e bsp_ble_driver_init(void)
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));         /**< 蓝牙控制器内存释放 */
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();        /**< 获取蓝牙控制器默认配置 */

    ret = esp_bt_controller_init(&bt_cfg);                                          /**< 蓝牙控制器初始化 */
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed", __func__);
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);                                /**< 启用蓝牙控制器 */
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed", __func__);
        return ret;
    }

    ret = esp_bluedroid_init();                                                     /**< 蓝牙初始化 */
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed", __func__);
        return ret;
    }

    ret = esp_bluedroid_enable();                                                   /**< 蓝牙使能 */
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed", __func__);
        return ret;
    }

    ble_mesh_get_dev_uuid(dev_uuid);                                                /**< 获取设备UUID */
    ret = ble_mesh_init();                                                          /**< 蓝牙mesh初始化 */
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", ret);
        return BSP_BLE_DRIVER_STATUS_ERROR;
    }

    return BSP_BLE_DRIVER_STATUS_OK;
}





