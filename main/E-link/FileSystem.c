#include "FileSystem.h"

// 文件系统开始
void LittlefsBegin(void){
	esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "spiffs",
        .format_if_mount_failed = false,
        .dont_mount = false,
    };

    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            printf("Failed to mount or format filesystem\n");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            printf("Failed to find LittleFS partition\n");
        }
        else
        {
            printf("Failed to initialize LittleFS (%s)\n", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        printf("Failed to get LittleFS partition information (%s)\n", esp_err_to_name(ret));
    }
    else
    {
        printf("Partition size: total: %d, used: %d\n", total, used);
    }
}